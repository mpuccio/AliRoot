#ifndef ALIMUONTRACKERDDLDECODER_H
#define ALIMUONTRACKERDDLDECODER_H
/**************************************************************************
 * This file is property of and copyright by the ALICE HLT Project        *
 * All rights reserved.                                                   *
 *                                                                        *
 * Primary Authors:                                                       *
 *   Artur Szostak <artursz@iafrica.com>                                  *
 *                                                                        *
 * Permission to use, copy, modify and distribute this software and its   *
 * documentation strictly for non-commercial purposes is hereby granted   *
 * without fee, provided that the above copyright notice appears in all   *
 * copies and that both the copyright notice and this permission notice   *
 * appear in the supporting documentation. The authors make no claims     *
 * about the suitability of this software for any purpose. It is          *
 * provided "as is" without express or implied warranty.                  *
 **************************************************************************/

/* $Id$ */

///
/// \file   AliMUONTrackerDDLDecoder.h
/// \author Artur Szostak <artursz@iafrica.com>
/// \date   28-11-2007
/// \brief  Implementation of a high performance DDL decoder for the muon tracking stations.
///
/// This file implementes the AliMUONTrackerDDLDecoder class, which contains
/// the core logic for decoding the payload in DDL streams comming from the muon
/// spectrometer's tracking chambers in a very efficient manner.
///
/// This implementation is derived from work done by Christian Finck for the
/// AliMUONPayloadTracker.
///
/// Note to maintainers: Please remember that this file is used by the online
/// dHLT system. As an online system, the dHLT requires the fastest code possible
/// in the decoders to satisfy its timing constraints. The performance impact
/// must be checked before any proposed modification is made to this file.
///

#include <cassert>
#include <ostream>
#include <Rtypes.h>


// We use C binding for the structures because C is more uniform with its application
// binary interface (ABI) between compilers.
extern "C"
{

// The following structures are the headers found in the DDL payload from the
// muon tracking chambers. The specification is defined in ALICE-INT-2005-012
// (https://edms.cern.ch/file/591904/1/ALICE-INT-2005-012.pdf)

/// The block header structure of the Tracker DDL payload.
struct AliMUONBlockHeaderStruct
{
	UInt_t     fDataKey;        ///< Data key word for CRT header 
	UInt_t     fTotalLength;    ///< total length of block structure (w/o padding word)
	UInt_t     fLength;         ///< length of raw data
	UInt_t     fDSPId;          ///< DSP id
	UInt_t     fL0Trigger;      ///< L0 trigger word
	UInt_t     fMiniEventId;    ///< Bunch Crossing for mini-event id (see TDR chapter 8)
	UInt_t     fEventId1;       ///< Event Id in bunch crossing
	UInt_t     fEventId2;       ///< Event Id in orbit number
};

/// The DSP header structure of the Tracker DDL payload.
struct AliMUONDSPHeaderStruct
{
	UInt_t     fDataKey;          ///< Data key word for FRT header 
	UInt_t     fTotalLength;      ///< total length of block structure
	UInt_t     fLength;           ///< length of raw data
	UInt_t     fDSPId;            ///< DSP id
	UInt_t     fBlkL1ATrigger;    ///< L1 accept in Block Structure (CRT)
	UInt_t     fMiniEventId;      ///< Mini Event Id in bunch crossing 
	UInt_t     fL1ATrigger;       ///< Number of L1 accept in DSP Structure (FRT)
	UInt_t     fL1RTrigger;       ///< Number of L1 reject in DSP Structure (FRT)
	UInt_t     fPaddingWord;      ///< padding dummy word for 64 bits transfer
	UInt_t     fErrorWord;        ///< Error word
};

/// The bus patch header structure of the Tracker DDL payload.
struct AliMUONBusPatchHeaderStruct
{
	UInt_t     fDataKey;       ///< Data key word for bus patch header 
	UInt_t     fTotalLength;   ///< total length of bus patch structure
	UInt_t     fLength;        ///< length of raw data
	UInt_t     fBusPatchId;    ///< bus patch id
};

} // extern "C"


/// \ingroup raw
/// \class AliMUONTrackerDDLDecoderEventHandler
/// \brief Callback event handler class for the AliMUONTrackerDDLDecoder.
/// This class is the base class defining what methods the event handler for the
/// high performance decoder should have. This handler actually does nothing.
/// The user of this decoder will have to derive from this class a custom event
/// handler that actually does something within the callback methods OnNewBusPatch,
/// OnData, OnError etc...
///
class AliMUONTrackerDDLDecoderEventHandler
{
public:

	// The only reason for a virtual destructor is to make -Weffc++ shutup.
	// This should not really be here since we do not actually want or need
	// run-time polymorphism.
	virtual ~AliMUONTrackerDDLDecoderEventHandler() {}

	/// All the possible error codes for the parsing.
	enum ErrorCode
	{
		kNoError = 0,                  /// Decoding was successful.
		// Offset our error codes to stay clear of any common codes in AliMUONRawStreamTracker:
		kBufferTooBig = 10,            /// The DDL raw data is larger than indicated by the headers; extra bytes are probably just garbage.
		kTooManyBlocks = 11,           /// Too many block structures found.
		kTooManyDSPs = 12,             /// Too many DSP structures found in the block.
		kTooManyBusPatches = 13,       /// Too many bus patch structures found in the DSP structure.
		kNoBlockHeader = 14,           /// Missing a block header.
		kBadBlockKey = 15,             /// The block header key word does not contain the correct value.
		kBadBlockLength = 16,          /// The block length field points past the end of the raw data size.
		kBadBlockTotalLength = 17,     /// The total block length field points past the end of the raw data size.
		kBlockLengthMismatch = 18,     /// The block length and total length fields do not correspond. One or both of these values is incorrect.
		kNoDSPHeader = 19,             /// Missing a DSP header.
		kBadDSPKey = 20,               /// The DSP header key word does not contain the correct value.
		kBadDSPLength = 21,            /// The DSP structure length field points past the end of the block structure.
		kBadDSPTotalLength = 22,       /// The total DSP structure length field points past the end of the block structure.
		kDSPLengthMismatch = 23,       /// The DSP structure length and total length fields do not correspond. One or both of these values is incorrect.
		kNoBusPatchHeader = 24,        /// Missing a bus patch header.
		kBadBusPatchKey = 25,          /// The bus patch header key word does not contain the correct value.
		kBadBusPatchLength = 26,       /// The bus patch length field points past the end of the DSP structure.
		kBadBusPatchTotalLength = 27,  /// The total bus patch length field points past the end of the DSP structure.
		kBusPatchLengthMismatch = 28,  /// The bus patch length and total length fields do not correspond. One or both of these values is incorrect.
		// match up error codes with AliMUONRawStreamTracker:
		kGlitchFound = 1,              /// Found a glitch. This means a 1 byte word has been randomly inserted into the raw data by mistake.
		kBadPaddingWord = 2,           /// The padding word does not contain the correct value.
		kParityError = 3               /// Found a parity error in the data word.
	};

	// The following methods should be overridden for specific processing to
	// take place in your event handler.

	/// The OnNewBuffer method will be called whenever a new buffer containing
	/// a DDL payload is about to be processed.
	/// The default behaviour of this method is to do nothing.
	/// \param buffer  The pointer to the start of the memory buffer storing
	///                the DDL payload.
	/// \param bufferSize The size in bytes of the memory buffer.
	void OnNewBuffer(const void* /*buffer*/, UInt_t /*bufferSize*/) {}
	
	/// OnNewBlock is called whenever a new block header is found in the payload.
	/// The default behaviour of this method is to do nothing.
	/// \param header  This is a pointer to the block header as found in the
	///                DDL payload.
	/// \param data  This is a pointer to the start of the block's contents.
	/// Note: both pointers point into the memory buffer being parsed so the
	/// contents must not be modified. On the other hand this is very efficient
	/// because no memory copying is required.
	void OnNewBlock(const AliMUONBlockHeaderStruct* /*header*/, const void* /*data*/) {}
	
	/// OnNewDSP is called whenever a new DSP header is found in the payload.
	/// Every DSP header recevied by a call to OnNewDSP is associated to the
	/// block header received in the most recent call to OnNewBlock.
	/// The default behaviour of this method is to do nothing.
	/// \param header  This is a pointer to the DSP header as found in the
	///                DDL payload.
	/// \param data  This is a pointer to the start of the DSP's contents.
	/// Note: both pointers point into the memory buffer being parsed so the
	/// contents must not be modified. On the other hand this is very efficient
	/// because no memory copying is required.
	void OnNewDSP(const AliMUONDSPHeaderStruct* /*header*/, const void* /*data*/) {}
	
	/// OnNewBusPatch is called whenever a new bus patch header is found in
	/// the payload. Every bus patch recevied by a call to OnNewBusPatch is
	/// associated to the DSP header received in the most recent call to OnNewDSP.
	/// The default behaviour of this method is to do nothing.
	/// \param header  This is a pointer to the bus patch header as found
	///                 in the DDL payload.
	/// \param data  This is a pointer to the start of the bus patch's contents,
	///              specifically the raw data words.
	/// Note: both pointers point into the memory buffer being parsed so the
	/// contents must not be modified. On the other hand this is very efficient
	/// because no memory copying is required.
	void OnNewBusPatch(const AliMUONBusPatchHeaderStruct* /*header*/, const void* /*data*/) {}
	
	/// OnData is called for every raw data word found within a bus patch.
	/// Every data ward recevied by a call to OnData is associated to the bus patch
	/// header received in the most recent call to OnNewBusPatch.
	/// The default behaviour of this method is to do nothing.
	/// \param data  This is the raw data word as found within the bus patch payload.
	void OnData(UInt_t /*data*/) {}
	
	/// Whenever a parsing error of the DDL payload is encountered because of
	/// corruption of the raw data (eg. bit flips) the OnError method is called
	/// imediately at the point this error is discovered.
	/// The default behaviour of this method is to do nothing.
	/// \param error  This is an error code indicating the kind of problem
	///               encountered with the DDL payload.
	/// \param location  This is a pointer into the DDL payload memory buffer
	///         indicating the exact location where the parsing error happened
	///         or i.e. the location of the corruption.
	/// Note that a relative offset in bytes from the start of the memory buffer
	/// can be calculated by: storing the buffer pointer recevied in OnNewBuffer
	/// earlier in fBufferStart for example, and then the offset is given by:
	///   offset = (unsigned long)location - (unsigned long)fBufferStart;
	void OnError(ErrorCode /*error*/, const void* /*location*/) {}
	
	/// This is a utility method which will unpack the MANU ID, channel ID and
	/// ADC signal value from a raw data word. It should normally be used in
	/// OnData() to unpack these fields.
	/// [in]  \param data  This is the raw data word found in the DDL payload.
	/// [out] \param maniId    This is filled with the unpacked MANU ID.
	/// [out] \param channelId This is filled with the unpacked MANU channel ID.
	/// [out] \param adc       This is filled with the unpacked ADC signal.
	static void UnpackADC(
			UInt_t data,
			UShort_t& manuId, UChar_t& channelId, UShort_t& adc
		)
	{
		manuId = (UShort_t)(data >> 18) & 0x7FF;
		channelId = (Char_t)(data >> 12) & 0x3F;
		adc = (UShort_t)(data & 0xFFF);
	}
	
	/// This is a utility method which converts an error code to a string
	/// respresentation for printing purposes.
	/// \param code  The error code as received in OnError for example.
	/// \return  An ANSI string containing the name of the error code symbol.
	static const char* ErrorCodeToString(ErrorCode code);
	
	/// This is a utility method which converts an error code to user friendly
	/// descriptive message useful for printing to the screen.
	/// \param code  The error code as received in OnError for example.
	/// \return  An ANSI string containing a descriptive message of the error.
	static const char* ErrorCodeToMessage(ErrorCode code);
};


/// \ingroup raw
/// \class AliMUONTrackerDDLDecoder
/// \brief A high performance decoder class for MUON tracking DDL data.
/// This class implements a high performance decoder for decoding DDL payload
/// data coming from the muon spectrometers tracking chambers.
/// It has been implemented using the event driven paradigm, which allows us
/// to minimise the number of method calls made in the inner loops of the algorithm
/// and minimise the memory footprint.
/// The decoder class only contains the basic decoding and error checking logic.
/// It calls methods such as OnNewBlock, OnNewBusPatch, OnData etc in
/// the event handler during the decoding to return the decoded data.
/// The event handler class is nothing more than a callback interface to deliver
/// the next chunks of decoded data.
/// To actually do something with the data one needs to implement a custom
/// event handler (callback) class by inheriting from AliMUONTrackerDDLDecoderEventHandler
/// and overriding the callback methods like so:
/// \code
///  class MyCustomHandler : public AliMUONTrackerDDLDecoderEventHandler
///  {
///  public:
///     void OnData(UInt_t data)
///     {
///       // I can do something with 'data' here.
///     }
///  };
/// \endcode
/// Once the custom handler is written then the decoder is in instantiated as
/// shown below, to use your new custom handler. And to start decoding one needs
/// to call the Decode() method of the decoder.
/// \code
///  AliMUONTrackerDDLDecoder<MyCustomHandler> myDecoder;
///  muDecoder.Decoder(buffer, bufferSize);
/// \endcode
/// Note that this class was written as a template on purpose. To maximise the
/// compilers chance to make optimisations and inlining code must use a template.
/// Depending on exactly what you do inside your handler the decoder will be
/// significantly slower if run time polymorphism was used i.e. making the class
/// AliMUONTrackerDDLDecoderEventHandler abstract and using virtual methods.
///
template <class EventHandler>
class AliMUONTrackerDDLDecoder
{
public:

	/// Default contructor.
	AliMUONTrackerDDLDecoder() :
		fExitOnError(true), fTryRecover(false),
		fSendDataOnParityError(false), fHadError(false),
		fMaxBlocks(2), fMaxDSPs(5), fMaxBusPatches(5), fHandler()
	{}
	
	/// Constant method to return the event handler instance.
	const EventHandler& GetHandler() const { return fHandler; }
	
	/// Returns the event handler instance.
	EventHandler& GetHandler() { return fHandler; }
	
	/// Returns the "exit on error" flag.
	/// i.e. should the decoder stop on the very first error found.
	bool ExitOnError() const { return fExitOnError; }
	
	/// Sets the "exit on error" flag.
	/// i.e. should the decoder stop on the very first error found.
	void ExitOnError(bool value) { fExitOnError = value; }
	
	/// Returns the "try to recover from errors" flag.
	/// i.e. should the decoder try to recover from errors found in the
	/// payload headers.
	bool TryRecover() const { return fTryRecover; }
	
	/// Sets the "try to recover from errors" flag.
	/// i.e. should the decoder try to recover from errors found in the
	/// payload headers.
	void TryRecover(bool value) { fTryRecover = value; }
	
	/// Returns the flag indicating if the raw data words in the bus patches
	/// that failed their parity tests (i.e. parity error / bit flip in the
	/// raw data word) will be sent to the event handler anyway through OnData.
	bool SendDataOnParityError() const { return fSendDataOnParityError; }
	
	/// Sets the flag indicating if the raw data words in the bus patches
	/// that failed their parity tests (i.e. parity error / bit flip in the
	/// raw data word) will be sent to the event handler anyway through OnData.
	void SendDataOnParityError(bool value) { fSendDataOnParityError = value; }
	
	/// Returns the maximum block count expected in the DDL payload.
	UInt_t MaxBlocks() const { return fMaxBlocks; }
	
	/// Sets the maximum block count expected in the DDL payload.
	void MaxBlocks(UInt_t n) { fMaxBlocks = n; }
	
	/// Returns the maximum DSP header count expected in any given block
	/// structure within the DDL payload.
	UInt_t MaxDSPs() const { return fMaxDSPs; }
	
	/// Sets the maximum DSP header count expected in any given block structure
	/// within the DDL payload.
	void MaxDSPs(UInt_t n) { fMaxDSPs = n; }
	
	/// Returns the maximum number of bus patches expected in any given DSP
	/// structure within the DDL payload.
	UInt_t MaxBusPatches() const { return fMaxBusPatches; }
	
	/// Sets the maximum number of bus patches expected in any given DSP
	/// structure within the DDL payload.
	void MaxBusPatches(UInt_t n) { fMaxBusPatches = n; }
	
	/// This method decodes the DDL payload contained in the buffer.
	bool Decode(const void* buffer, UInt_t bufferSize);
	
private:

	bool fExitOnError; /// Indicates if we should exit on the very first error.
	bool fTryRecover; /// Indicates if we should try recover from a corrupt structure header.
	bool fSendDataOnParityError; /// If set to true then we issue a OnData() event even if the data word had a parity error.
	bool fHadError; /// Indicates if we had an error decoding the data.
	UInt_t fMaxBlocks; /// Maximum number of block structures allowed in a DDL stream.
	UInt_t fMaxDSPs; /// Maximum number of DSP structures allowed in a DDL stream.
	UInt_t fMaxBusPatches; /// Maximum number of bus patch structures allowed in a DDL stream.
	EventHandler fHandler; /// The event handler which deals with parsing events.

	bool DecodeBlockData(
			const AliMUONBlockHeaderStruct* blockHeader,
			const UChar_t* start, const UChar_t* end
		);

	bool DecodeDSPData(const UChar_t* start, const UChar_t* end);
	
	bool DecodeBusPatchData(const UChar_t* start, const UChar_t* end);

	/// Possible results that can be returned by the TryRecoverStruct method.
	enum RecoverResult
	{
		kRecoverFailed,        /// The recovery failed. Cannot continue parsing.
		kStructRecovered,      /// Indicates that we recovered from a corrupt structure header and can continue processing the given structure.
		kContinueToNextStruct  /// Must continue parsing the next structure and ignore the current one.
	};

	RecoverResult TryRecoverStruct(
			UInt_t expectedKey,
			UInt_t headerSize,
			UInt_t totalLength,
			UInt_t length,
			const UChar_t* structStart,
			const UChar_t* bufferEnd,
			const UChar_t*& dataEnd,
			const UChar_t*& structEnd,
			const UChar_t*& current
		);
	
	const UChar_t* FindKey(
			UInt_t key, const UChar_t* start, const UChar_t* end
		);
	
	bool ParityIsOk(UInt_t data);
	
	static const UInt_t fgkBlockDataKey;     /// The key word expected to identify block structure headers.
	static const UInt_t fgkDSPDataKey;       /// The key word expected to identify DSP structure headers.
	static const UInt_t fgkBusPatchDataKey;  /// The key word expected to identify bus patch headers.
	static const UInt_t fgkPaddingWord;      /// The expected format of the padding word in the DDL payload.
};

//_____________________________________________________________________________

// The following are the structure header keys which are used to identify the kind
// of structure header we are dealing with: block, DSP or bus patch header.
template <class EventHandler>
const UInt_t AliMUONTrackerDDLDecoder<EventHandler>::fgkBlockDataKey = 0xFC0000FC;
template <class EventHandler>
const UInt_t AliMUONTrackerDDLDecoder<EventHandler>::fgkDSPDataKey = 0xF000000F;
template <class EventHandler>
const UInt_t AliMUONTrackerDDLDecoder<EventHandler>::fgkBusPatchDataKey = 0xB000000B;
template <class EventHandler>
const UInt_t AliMUONTrackerDDLDecoder<EventHandler>::fgkPaddingWord = 0xBEEFFACE;


inline const char* AliMUONTrackerDDLDecoderEventHandler::ErrorCodeToString(ErrorCode code)
{
	/// This is a utility method which converts an error code to a string
	/// representation for printing purposes.
	/// \param code  The error code as received in OnError for example.
	/// \return  An ANSI string containing the name of the error code symbol.
	
	switch (code)
	{
	case kNoError: return "kNoError";
	case kBufferTooBig: return "kBufferTooBig";
	case kTooManyBlocks: return "kTooManyBlocks";
	case kTooManyDSPs: return "kTooManyDSPs";
	case kTooManyBusPatches: return "kTooManyBusPatches";
	case kNoBlockHeader: return "kNoBlockHeader";
	case kBadBlockKey: return "kBadBlockKey";
	case kBadBlockLength: return "kBadBlockLength";
	case kBadBlockTotalLength: return "kBadBlockTotalLength";
	case kBlockLengthMismatch: return "kBlockLengthMismatch";
	case kNoDSPHeader: return "kNoDSPHeader";
	case kBadDSPKey: return "kBadDSPKey";
	case kBadDSPLength: return "kBadDSPLength";
	case kBadDSPTotalLength: return "kBadDSPTotalLength";
	case kDSPLengthMismatch: return "kDSPLengthMismatch";
	case kNoBusPatchHeader: return "kNoBusPatchHeader";
	case kBadBusPatchKey: return "kBadBusPatchKey";
	case kBadBusPatchLength: return "kBadBusPatchLength";
	case kBadBusPatchTotalLength: return "kBadBusPatchTotalLength";
	case kBusPatchLengthMismatch: return "kBusPatchLengthMismatch";
	case kGlitchFound: return "kGlitchFound";
	case kBadPaddingWord: return "kBadPaddingWord";
	case kParityError: return "kParityError";
	default: return "INVALID";
	}
}


inline const char* AliMUONTrackerDDLDecoderEventHandler::ErrorCodeToMessage(ErrorCode code)
{
	/// This is a utility method which converts an error code to user friendly
	/// descriptive message useful for printing to the screen.
	/// \param code  The error code as received in OnError for example.
	/// \return  An ANSI string containing a descriptive message of the error.
	
	switch (code)
	{
	case kNoError:
		return "Decoding was successful.";
	case kBufferTooBig:
		return "The DDL raw data is larger than indicated by the headers;"
		       " extra bytes are probably just garbage.";
	case kTooManyBlocks:
		return "Too many block structures found.";
	case kTooManyDSPs:
		return "Too many DSP structures found in the block.";
	case kTooManyBusPatches:
		return "Too many bus patch structures found in the DSP structure.";
	case kNoBlockHeader:
		return "Missing a block header.";
	case kBadBlockKey:
		return "The block header key word does not contain the correct value.";
	case kBadBlockLength:
		return "The block length field points past the end of the raw data size.";
	case kBadBlockTotalLength:
		return "The total block length field points past the end of the"
		       " raw data size.";
	case kBlockLengthMismatch:
		return "The block length and total length fields do not correspond."
		       " One or both of these values is incorrect.";
	case kNoDSPHeader:
		return "Missing a DSP header.";
	case kBadDSPKey:
		return "The DSP header key word does not contain the correct value.";
	case kBadDSPLength:
		return "The DSP structure length field points past the end of the"
		       " block structure.";
	case kBadDSPTotalLength:
		return "The total DSP structure length field points past the end of"
		       " the block structure.";
	case kDSPLengthMismatch:
		return "The DSP structure length and total length fields do not"
		       " correspond. One or both of these values is incorrect.";
	case kNoBusPatchHeader:
		return "Missing a bus patch header.";
	case kBadBusPatchKey:
		return "The bus patch header key word does not contain the correct value.";
	case kBadBusPatchLength:
		return "The bus patch length field points past the end of the"
		       " DSP structure.";
	case kBadBusPatchTotalLength:
		return "The total bus patch length field points past the end of"
		       " the DSP structure.";
	case kBusPatchLengthMismatch:
		return "The bus patch length and total length fields do not correspond."
		       " One or both of these values is incorrect.";
	case kGlitchFound:
		return "Found a glitch. This means a 1 byte word has been randomly"
		       " inserted into the raw data by mistake.";
	case kBadPaddingWord:
		return "The padding word does not contain the correct value.";
	case kParityError:
		return "Found a parity error in the data word.";
	default:
		return "Unknown error code!";
	}
}


inline std::ostream& operator << (std::ostream& os, AliMUONTrackerDDLDecoderEventHandler::ErrorCode code)
{
	/// This is the stream operator for std::ostream classes to be able to
	/// easily write the error messages associated with the error codes generated
	/// by the decoder to 'cout' or 'cerr' for example.
	
	os << AliMUONTrackerDDLDecoderEventHandler::ErrorCodeToMessage(code);
	return os;
}


template <class EventHandler>
bool AliMUONTrackerDDLDecoder<EventHandler>::Decode(const void* buffer, UInt_t bufferSize)
{
	/// This method should be called to actually decode the DDL payload
	/// contained in a memory buffer. The payload should be for a muon tracking
	/// chamber DDL stream.
	/// As the decoder progresses it will make method calls to the event handler
	/// instance (which can be accessed with the GetHandler() method) to indicate
	/// the start of the new block, DSP and bus patch headers. For every raw
	/// data word the OnData method of the event handler is called.
	///
	/// If an error occurs during the parse because the data is corrupt then
	/// the OnError method is called indicating what the problem was.
	/// Decoding will stop at this point unless the fExitOnError flag is set
	/// to false. Also raw data words which contain a parity error are only
	/// sent to the event handler with OnData if the fSendDataOnParityError
	/// flag is set to true. There is also an optional flag fTryRecover which
	/// can enable logic which will attempt to recover the header structures found
	/// in the DDL payload if they are found to be inconsistent (assumed corrupt).
	///
	/// \param buffer  This is the pointer to the start of the memory buffer
	///     containing the DDL payload. Remember that this must be the start of
	///     the payload and not the DDL stream. That is, this pointer should be
	///     equal to: DDL start pointer + 8 * sizeof(UInt_t).
	/// \param bufferSize  This is the pointer to the first byte just past the
	///     end of the block structure.
	/// \return Returns false if there was any problem with decoding the data,
	///     and true otherwise. Note: the data may have been partially decoded
	///     even if false was returned, which would be indicated by at least one
	///     call to the event handlers OnData method.
	
	assert( buffer != NULL );
	
	fHadError = false;
	
	// We are basically implementing something like a recursive decent parser.
	// So start by marking the current buffer position and end of buffer.
	const UChar_t* current = reinterpret_cast<const UChar_t*>(buffer);
	const UChar_t* end = current + bufferSize;
	
	// Signal a new buffer event.
	fHandler.OnNewBuffer(buffer, bufferSize);

	UInt_t blockCount = 0; // Indicates the number of blocks decoded.
	while (current < end)
	{
		// Mark the start of the block structure.
		const UChar_t* blockStart = current;
		
		// Get the block header, move the current pointer just past the end
		// of the header and check that we have not overflowed the buffer.
		const AliMUONBlockHeaderStruct* blockHeader
			= reinterpret_cast<const AliMUONBlockHeaderStruct*>(blockStart);
		current += sizeof(AliMUONBlockHeaderStruct);
		if (current > end)
		{
			// So we only got part of a block header at the very end
			// of the buffer. Nothing to do but report the error and exit.
			if (blockCount == fMaxBlocks)
				// Special case where we got all the blocks we
				// expected, so the remaining data must be rubbish.
				fHandler.OnError(EventHandler::kBufferTooBig, blockHeader);
			else
				fHandler.OnError(EventHandler::kNoBlockHeader, blockHeader);
			return false;
		}
		
		// The header fits the buffer so we can mark the data start and
		// read from the header to find the end of data and block pointers.
		const UChar_t* dataStart = current;
		current += blockHeader->fLength * sizeof(UInt_t);
		const UChar_t* dataEnd = current;
		const UChar_t* blockEnd = blockStart
			+ blockHeader->fTotalLength * sizeof(UInt_t);
		
		// Now we need to check for the following things:
		// 1) Is the end of block or end of data pointer outside the buffer
		//    boundaries.
		// 2) Are the values for these pointers the same.
		// 3) Is the expected data key in the header present.
		// If any of the above fail then we know there is a problem with
		// the block header. It must be corrupted somehow.
		if (blockHeader->fDataKey != fgkBlockDataKey
		    or dataEnd > end or blockEnd > end or dataEnd != blockEnd)
		{
			// So let us see what exactly is wrong and report this.
			if (blockCount == fMaxBlocks)
			{
				// Special case where we got all the blocks we
				// expected, so the remaining data must be rubbish.
				// Don't even bother trying to recover the data.
				fHandler.OnError(EventHandler::kBufferTooBig, blockHeader);
				return false;
			}
			if (blockHeader->fDataKey != fgkBlockDataKey)
				fHandler.OnError(EventHandler::kBadBlockKey, &blockHeader->fDataKey);
			if (blockEnd > end)
				fHandler.OnError(EventHandler::kBadBlockLength, &blockHeader->fLength);
			if (dataEnd > end)
				fHandler.OnError(EventHandler::kBadBlockTotalLength, &blockHeader->fTotalLength);
			if (dataEnd != blockEnd)
				fHandler.OnError(EventHandler::kBlockLengthMismatch, blockHeader);
			
			// Stop the decoding if so requested by the user, otherwise
			// remember about the error so that we return false from this
			// routine and continue decoding.
			if (fExitOnError)
				return false;
			else
				fHadError = true;
			
			// Try to recover from the corrupt header.
			RecoverResult result = TryRecoverStruct(
					fgkBlockDataKey, sizeof(AliMUONBlockHeaderStruct),
					blockHeader->fTotalLength, blockHeader->fLength,
					blockStart, end, dataEnd, blockEnd, current
				);
			if (result == kContinueToNextStruct)
				continue; // Try the next block at 'current'.
			if (result == kRecoverFailed) return false;
		}
		
		// At this point we certainly have a valid block header, so we
		// need to check if we have more blocks than we expected. If not
		// then we can indicate we have another block and decode its data.
		if (++blockCount > fMaxBlocks)
		{
			fHandler.OnError(EventHandler::kTooManyBlocks, current);
			
			// In this case we stop the decoding because clearly
			// something is seriously wrong with the data if we are
			// getting more blocks than expected.
			return false;
		}
		
		fHandler.OnNewBlock(blockHeader, dataStart);
		if (DecodeBlockData(blockHeader, dataStart, dataEnd)) continue;
		
		// At this point we had a problem decoding the block structure's
		// data. Thus we should stop further decoding if so requested by
		// the user. Note the fHadError flag is already marked inside
		// DecodeBlockData.
		if (fExitOnError) return false;
	}
	
	return not fHadError;
}


template <class EventHandler>
bool AliMUONTrackerDDLDecoder<EventHandler>::DecodeBlockData(
		const AliMUONBlockHeaderStruct* blockHeader,
		const UChar_t* start, const UChar_t* end
	)
{
	/// This method decodes a block structure's data payload. It unpacks the
	/// DSP structures contained inside and then for each DSP it calls the
	/// OnNewDSP method for the event handler to signal the start of each new
	/// DSP structure.
	/// \param start  This is the pointer to the start of the block
	///               structure's data.
	/// \param end  This is the pointer to the first byte just past the
	///             end of the block structure.
	/// \return If the block structure's data was decoded without errors
	///      or we could recover from the errors, then true is returned.
	///      False is returned otherwise.
	
	const UChar_t* current = start;
	
	UInt_t dspCount = 0; // Indicates the number of DSPs decoded.
	while (current < end)
	{
		// Mark the start of the DSP structure.
		const UChar_t* dspStart = current;
		
		// Get the DSP header, move the current pointer just past the end
		// of the header and check that we have not overflowed the buffer.
		const AliMUONDSPHeaderStruct* dspHeader
			= reinterpret_cast<const AliMUONDSPHeaderStruct*>(dspStart);
		current += sizeof(AliMUONDSPHeaderStruct);
		if (current > end)
		{
			// So we only got part of a DSP header at the very end of
			// the block structure buffer. Nothing to do but report the
			// error and exit. Set fHadError in case of further decoding.
			fHandler.OnError(EventHandler::kNoDSPHeader, dspHeader);
			fHadError = true;
			return false;
		}
		
		// The header fits the buffer so we can mark the data start and
		// read from the header to find the end of data and DSP pointers.
		const UChar_t* dataStart = current;
		current += dspHeader->fLength * sizeof(UInt_t);
		const UChar_t* dataEnd = current;
		const UChar_t* dspEnd = dspStart + dspHeader->fTotalLength * sizeof(UInt_t);
		
		// Now we need to check for the following things:
		// 1) Is the end of DSP or end of data pointer outside the buffer
		//    boundaries.
		// 2) Are the values for these pointers the same.
		// 3) Is the expected data key in the header present.
		// If any of the above fail then we know there is a problem with
		// the DSP header. It must be corrupted somehow.
		if (dspHeader->fDataKey != fgkDSPDataKey
		    or dataEnd > end or dspEnd > end or dataEnd != dspEnd)
		{
			// So let us see what exactly is wrong and report this.
			if (dspHeader->fDataKey != fgkDSPDataKey)
				fHandler.OnError(EventHandler::kBadDSPKey, &dspHeader->fDataKey);
			if (dspEnd > end)
				fHandler.OnError(EventHandler::kBadDSPLength, &dspHeader->fLength);
			if (dataEnd > end)
				fHandler.OnError(EventHandler::kBadDSPTotalLength, &dspHeader->fTotalLength);
			if (dataEnd != dspEnd)
				fHandler.OnError(EventHandler::kDSPLengthMismatch, dspHeader);
			
			// Indicate we had and error and stop the decoding if so
			// requested by the user.
			fHadError = true;
			if (fExitOnError) return false;
			
			// Try to recover from the corrupt header.
			RecoverResult result = TryRecoverStruct(
					fgkDSPDataKey, sizeof(AliMUONDSPHeaderStruct),
					dspHeader->fTotalLength, dspHeader->fLength,
					dspStart, end, dataEnd, dspEnd, current
				);
			if (result == kContinueToNextStruct)
				continue; // Try the next DSP at 'current'.
			if (result == kRecoverFailed) return false;
		}
		
		// At this point we certainly have a valid DSP header, so we
		// need to check if we have more DSPs than we expected. If not
		// then we can indicate we have another DSP and decode its data.
		if (++dspCount > fMaxDSPs)
		{
			fHandler.OnError(EventHandler::kTooManyDSPs, current);
			
			// In this case we stop further decoding of the block
			// structure data because clearly something is seriously
			// wrong if we are getting more DSPs than expected.
			// Indicate that we had an error so the Decode() method
			// returns false.
			fHadError = true;
			return false;
		}
		
		fHandler.OnNewDSP(dspHeader, dataStart);
		
		// Check the error word in the header.
		if (dspHeader->fErrorWord == (0x000000B1 | blockHeader->fDSPId)
		    or dspHeader->fErrorWord == (0x00000091 | blockHeader->fDSPId)
		   )
		{
			// An event with a glitch in the readout has been detected.
			// It means that somewhere a 1 byte word has been randomly
			// inserted and all the readout sequence is shifted until
			// the next event.
			fHandler.OnError(EventHandler::kGlitchFound, &dspHeader->fErrorWord);
			fHadError = true;
			if (fExitOnError) return false;
			
			// Try recover by finding the very next DSP and continue
			// decoding from there. Note: to achieve all we have to do
			// is continue to the next iteration, because the logic
			// will land up calling the FindKey method within the
			// TryRecoverStruct method above.
			if (fTryRecover) continue;
		}
		
		// Check if we are padding. If we are, then the bus patch data is
		// actually 4 bytes smaller and the last word is a padding word.
		if (dspHeader->fPaddingWord == 1)
		{
			dataEnd -= sizeof(UInt_t);
			
			// Check the pad word is correct.
			const UInt_t* padWord = reinterpret_cast<const UInt_t*>(dataEnd);
			if (*padWord != fgkPaddingWord)
			{
				fHandler.OnError(EventHandler::kBadPaddingWord, padWord);
				fHadError = true;
				if (fExitOnError) return false;
			}
		}
		
		if (DecodeDSPData(dataStart, dataEnd)) continue;
		
		// At this point we had a problem decoding the DSP structure's
		// data, thus we should stop further decoding if so requested by
		// the user. Note the fHadError flag is already marked inside
		// DecodeDSPData.
		if (fExitOnError) return false;
	}
	
	return true;
}


template <class EventHandler>
bool AliMUONTrackerDDLDecoder<EventHandler>::DecodeDSPData(
		const UChar_t* start, const UChar_t* end
	)
{
	/// This method decodes a DSP structure's data payload. It finds all the
	/// bus patches found inside and for each it calls the OnNewBusPatch method
	/// for the event handler to signal the start of each new bus patch.
	/// \param start  This is the pointer to the start of the DSP structure's data.
	/// \param end  This is the pointer to the first byte just past the
	///             end of the DSP structure.
	/// \return If the DSP structure's data was decoded without errors
	///      or we could recover from the errors, then true is returned.
	///      False is returned otherwise.
	
	const UChar_t* current = start;
	
	UInt_t busPatchCount = 0; // Indicates the number of bus patches decoded.
	while (current < end)
	{
		// Mark the start of the bus patch structure.
		const UChar_t* busPatchStart = current;
		
		// Get the bus patch header, move the current pointer just past
		// the end of the header and check that we have not overflowed
		// the buffer.
		const AliMUONBusPatchHeaderStruct* busPatchHeader
			= reinterpret_cast<const AliMUONBusPatchHeaderStruct*>(busPatchStart);
		current += sizeof(AliMUONBusPatchHeaderStruct);
		if (current > end)
		{
			// So we only got part of a bus patch header at the very
			// end of the DSP structure buffer. Nothing to do but
			// report the error and exit. Set fHadError in case of
			// further decoding.
			fHandler.OnError(EventHandler::kNoBusPatchHeader, busPatchHeader);
			fHadError = true;
			return false;
		}
		
		// The header fits the buffer so we can mark the data start and
		// read from the header to find the end of data and bus patch
		// structure pointers.
		const UChar_t* dataStart = current;
		current += busPatchHeader->fLength * sizeof(UInt_t);
		const UChar_t* dataEnd = current;
		const UChar_t* busPatchEnd = busPatchStart
			+ busPatchHeader->fTotalLength * sizeof(UInt_t);
		
		// Now we need to check for the following things:
		// 1) Is the end of bus patch structure or end of data pointer
		//    outside the buffer boundaries.
		// 2) Are the values for these pointers the same.
		// 3) Is the expected data key in the header present.
		// If any of the above fail then we know there is a problem with
		// the bus patch header. It must be corrupted somehow.
		if (busPatchHeader->fDataKey != fgkBusPatchDataKey
		    or dataEnd > end or busPatchEnd > end or dataEnd != busPatchEnd)
		{
			// So let us see what exactly is wrong and report this.
			if (busPatchHeader->fDataKey != fgkBusPatchDataKey)
				fHandler.OnError(EventHandler::kBadBusPatchKey, &busPatchHeader->fDataKey);
			if (busPatchEnd > end)
				fHandler.OnError(EventHandler::kBadBusPatchLength, &busPatchHeader->fLength);
			if (dataEnd > end)
				fHandler.OnError(EventHandler::kBadBusPatchTotalLength, &busPatchHeader->fTotalLength);
			if (dataEnd != busPatchEnd)
				fHandler.OnError(EventHandler::kBusPatchLengthMismatch, busPatchHeader);
			
			// Indicate we had and error and stop the decoding if so
			// requested by the user.
			fHadError = true;
			if (fExitOnError) return false;
			
			// Try to recover from the corrupt header.
			RecoverResult result = TryRecoverStruct(
					fgkBusPatchDataKey, sizeof(AliMUONBusPatchHeaderStruct),
					busPatchHeader->fTotalLength, busPatchHeader->fLength,
					busPatchStart, end, dataEnd, busPatchEnd, current
				);
			if (result == kContinueToNextStruct)
				continue; // Try the next bus patch at 'current'.
			if (result == kRecoverFailed) return false;
		}
		
		// At this point we certainly have a valid bus patch header, so
		// we need to check if we have more bus patches than we expected.
		// If not then we can indicate we have another bus patch and
		// decode its data.
		if (++busPatchCount > fMaxBusPatches)
		{
			fHandler.OnError(EventHandler::kTooManyBusPatches, current);
			
			// In this case we stop further decoding of the DSP
			// structure's data because clearly something is seriously
			// wrong if we are getting more bus patches than expected.
			// Indicate that we had an error so the Decode() method
			// returns false.
			fHadError = true;
			return false;
		}
		
		fHandler.OnNewBusPatch(busPatchHeader, dataStart);
		if (DecodeBusPatchData(dataStart, dataEnd)) continue;
		
		// At this point we had a problem decoding the bus patch data,
		// thus we should stop further decoding if so requested by the
		// user. Note the fHadError flag is already marked inside
		// DecodeBusPatchData.
		if (fExitOnError) return false;
	}
	
	return true;
}


template <class EventHandler>
bool AliMUONTrackerDDLDecoder<EventHandler>::DecodeBusPatchData(
		const UChar_t* start, const UChar_t* end
	)
{
	/// This method decodes a single bus patch's data payload.
	/// It will check the parity of the raw data words and send them
	/// to the event handler instance with calls to OnData.
	/// \param start  This is the pointer to the start of the bus patch
	///               structure's data.
	/// \param end  This is the pointer to the first byte just past the
	///             end of the bus patch structure.
	/// \return If the bus patch's data was decoded without errors
	///      or we could recover from the errors, then true is returned.
	///      False is returned otherwise.

	// Assert that 'end' is always larger than start by n*sizeof(UInt_t)
	// where n is a positive integer. This should be the case because we
	// always add multiples of sizeof(UInt_t) to the 'current' pointer in
	// all the DecodeXYZ methods.
	assert( UInt_t(end - start) % 4 == 0 );
	
	// Now step through all the data words and issue OnData events.
	// We also need to check parity and signal OnError if it is not valid
	// for any of the data words.
	const UInt_t* data = reinterpret_cast<const UInt_t*>(start);
	const UInt_t* dataEnd = reinterpret_cast<const UInt_t*>(end);
	for (; data < dataEnd; data++)
	{
		if (ParityIsOk(*data))
		{
			fHandler.OnData(*data);
		}
		else
		{
			// Indicate we had a parity error and exit immediately
			// if the user so requested.
			fHandler.OnError(EventHandler::kParityError, data);
			fHadError = true;
			if (fExitOnError) return false;
			
			if (fSendDataOnParityError)
				fHandler.OnData(*data);
		}
	}
	
	return true;
}


template <class EventHandler>
typename AliMUONTrackerDDLDecoder<EventHandler>::RecoverResult
AliMUONTrackerDDLDecoder<EventHandler>::TryRecoverStruct(
		UInt_t expectedKey,
		UInt_t headerSize,
		UInt_t totalLength,
		UInt_t length,
		const UChar_t* structStart,
		const UChar_t* bufferEnd,
		const UChar_t*& dataEnd,
		const UChar_t*& structEnd,
		const UChar_t*& current
	)
{
	/// This method attempts to recover from a corrupt structure header by
	/// figuring out which of the structure size indicators is correct.
	/// This is possible because each header has some redundant information.
	/// The recovery procedure is only attempted if fTryRecover was set to
	/// true. If the recovery procedure is successful then this method will
	/// also update the pointers indicating the start of data, end of structure
	/// and current parsing position with the correct values.
	///
	/// [in]  \param expectedKey This is the expected block key for the header
	///           currently being processed.
	/// [in]  \param headerSize  The expected header size as given by the sizeof
	///           operator for example.
	/// [in]  \param totalLength The total length as given by the fTotalLength
	///           field in the current header being handled.
	/// [in]  \param length  The data length as given by the fLength field
	///           in the current header being handled.
	/// [in]  \param structStart A pointer to the start of the structure header.
	/// [in]  \param bufferEnd A pointer to the first byte just past the end
	///           of the buffer. This could be the pointer to the first byte
	///           just past the end of the parent structure if we are dealing
	///           with a DSP structure or bus patch. The parent structure for
	///           the DSP is a block structure and for a bus patch it is a DSP.
	/// [out] \param dataEnd This is the pointer to the first byte just past
	///           the end of the structure being processed. It should be equal to
	///           structStart + sizeof(structure header) + fLength, where fLength
	///           is the field found in the structure's header itself. This value
	///           will be corrected and updated if we could recover from the
	///           corruption in the header.
	/// [out] \param structEnd A pointer to the first byte just past the end of
	///           the structure. This value should be set equal to
	///           structStart + fTotalLength * sizeof(UInt_t), where fTotalLength
	///           is the field found in the structure's header itself. This value
	///           will be corrected and updated if we could recover from the
	///           corruption in the header.
	/// [out] \param current This is the pointer to the current location in
	///           the DDL payload being parsed. It should in principle point
	///           to the start of the structures data. This value will be
	///           corrected and updated if we could recover from the corruption
	///           in the header.
	///
	/// \return Returns the result of the recovery attempt, which can be one
	///    of the following:
	///      kRecoverFailed - The recovery failed completely so the caller
	///           cannot continue parsing any more structures. If the failure
	///           is within a DSP then one could still continue parsing
	///           from the next block. Similarly for bus patches, parsing could
	///           continue from the next DSP structure.
	///      kStructRecovered - Indicates that we recovered from a corrupt
	///           structure header and can continue processing the data of the
	///           structure in question.
	///      kContinueToNextStruct - Either fTryRecover was set to false or we
	///           could not recover from the corrupt header but we did find the
	///           start of another header matching the expected key so parsing
	///           can continue from the updated current position.

	// Check if the user wants us to try and recover from a corrupt header.
	if (not fTryRecover) return kContinueToNextStruct;
	
	// If the user wants us to try recover, then try to recover what the
	// correct values for dataEnd, structEnd and current were supposed to be.
	// The recovery procedure is as follows: We have 4 conditions for a correct
	// header:
	//   1) The header key is what we expect.
	//   2) The totalLength equals length + headerSize.
	//   3) The word at dataEnd contains a valid key. (implies length is
	//      correct.)
	//   4) The word at structEnd contains a valid key. (implies totalLength
	//      is correct.)
	// If any 2 of these conditions hold then we know that only one of the
	// header fields is corrupt and we have enough information to reconstruct
	// the third field. Note that if conditions 3 and 4 are true then this
	// implies 2 is also true. (not necessarily the other way around though.)
	// The valid key mentioned above at dataEnd and structEnd should be:
	//   a) A bus patch key, DSP key or end of buffer if expectedKey indicates
	//      a buspatch.
	//   b) A DSP key, block structure key or end of buffer if expectedKey
	//      indicates a DSP.
	//   c) A block structure key or end of buffer if expectedKey indicates
	//      a DSP.
	const UInt_t* headerKey = reinterpret_cast<const UInt_t*>(structStart);
	bool headerKeyOk = (expectedKey == *headerKey);
	
	bool lengthsMatch = (totalLength == length + headerSize);
	
	bool lengthIsCorrect = false;
	bool totalLengthIsCorrect = false;
	const UInt_t* keyAtDataEnd = reinterpret_cast<const UInt_t*>(dataEnd);
	const UInt_t* keyAtStructEnd = reinterpret_cast<const UInt_t*>(structEnd);
	
	switch (expectedKey)
	{
	case fgkBlockDataKey:
		if (dataEnd == bufferEnd)
		{
			// Are we at the end of the buffer?
			lengthIsCorrect = true;
		}
		else
		{
			// Must check that we can read another 4 bytes before
			// checking the key at dataEnd.
			if (dataEnd + sizeof(UInt_t) <= bufferEnd)
			{
				if (*keyAtDataEnd == fgkBlockDataKey)
					lengthIsCorrect = true;
			}
		}
		
		if (structEnd == bufferEnd)
		{
			// Are we at the end of the buffer?
			totalLengthIsCorrect = true;
		}
		else
		{
			// Must check that we can read another 4 bytes before
			// checking the key at structEnd.
			if (structEnd + sizeof(UInt_t) <= bufferEnd)
			{
				if (*keyAtStructEnd == fgkBlockDataKey)
					totalLengthIsCorrect = true;
			}
		}
			
		break;
	
	case fgkDSPDataKey:
		if (dataEnd == bufferEnd)
		{
			// Are we at the end of the buffer?
			lengthIsCorrect = true;
		}
		else
		{
			// Must check that we can read another 4 bytes before
			// checking the key at dataEnd.
			if (dataEnd + sizeof(UInt_t) <= bufferEnd)
			{
				if (*keyAtDataEnd == fgkBlockDataKey
				    or *keyAtDataEnd == fgkDSPDataKey)
					lengthIsCorrect = true;
			}
		}
		
		if (structEnd == bufferEnd)
		{
			// Are we at the end of the buffer?
			totalLengthIsCorrect = true;
		}
		else
		{
			// Must check that we can read another 4 bytes before
			// checking the key at structEnd.
			if (structEnd + sizeof(UInt_t) <= bufferEnd)
			{
				if (*keyAtStructEnd == fgkBlockDataKey
				    or *keyAtStructEnd == fgkDSPDataKey)
					totalLengthIsCorrect = true;
			}
		}
			
		break;
	
	case fgkBusPatchDataKey:
		if (dataEnd == bufferEnd)
		{
			// Are we at the end of the buffer?
			lengthIsCorrect = true;
		}
		else
		{
			// Must check that we can read another 4 bytes before
			// checking the key at dataEnd.
			if (dataEnd + sizeof(UInt_t) <= bufferEnd)
			{
				if (*keyAtDataEnd == fgkDSPDataKey
				    or *keyAtDataEnd == fgkBusPatchDataKey)
					lengthIsCorrect = true;
			}
		}
		
		if (structEnd == bufferEnd)
		{
			// Are we at the end of the buffer?
			totalLengthIsCorrect = true;
		}
		else
		{
			// Must check that we can read another 4 bytes before
			// checking the key at structEnd.
			if (structEnd + sizeof(UInt_t) <= bufferEnd)
			{
				if (*keyAtStructEnd == fgkDSPDataKey
				    or *keyAtStructEnd == fgkBusPatchDataKey)
					totalLengthIsCorrect = true;
			}
		}
			
		break;
		
	default:
		// lengthIsCorrect and totalLengthIsCorrect already set to false.
		break;
	}
	
	if (headerKeyOk and lengthIsCorrect)
	{
		// totalLength was wrong, dataEnd is correct.
		structEnd = dataEnd;
		current = dataEnd;
		return kStructRecovered;
	}
	if (headerKeyOk and totalLengthIsCorrect)
	{
		// Length was wrong, structEnd is correct.
		dataEnd = structEnd;
		current = structEnd;
		return kStructRecovered;
	}
	if (lengthsMatch and lengthIsCorrect and totalLengthIsCorrect)
	{
		// The header's key was wrong but the lengths and pointers are OK.
		return kStructRecovered;
	}
	
	// Could not recover the header from the available information, so find
	// the next key in the stream that is the same as the currently expected
	// one and continue decoding from there.
	const UChar_t* location = FindKey(
			expectedKey, structStart + sizeof(UInt_t), bufferEnd
		);
	if (location != NULL)
	{
		current = location;
		return kContinueToNextStruct;
	}

	return kRecoverFailed;
}


template <class EventHandler>
const UChar_t* AliMUONTrackerDDLDecoder<EventHandler>::FindKey(
		UInt_t key, const UChar_t* start, const UChar_t* end
	)
{
	/// Searches for the first occurrence of the key value in the buffer marked by
	/// 'start' and 'end'. 'start' should point to the start of the buffer and 'end'
	/// should point to 'start + bufferSize', i.e. just past the last byte of the
	/// buffer. If the key was found then the pointer to that location is returned
	/// otherwise NULL is returned.
 
	const UChar_t* current = start;
	while (current + sizeof(UInt_t) <= end)
	{
		UInt_t data = * reinterpret_cast<const UInt_t*>(current);
		if (data == key) return current;
		current++;
	}
	return NULL;
}


template <class EventHandler>
bool AliMUONTrackerDDLDecoder<EventHandler>::ParityIsOk(UInt_t data)
{
	/// Optimised parity check addapted from:
	/// http://graphics.stanford.edu/~seander/bithacks.html#ParityParallel
	
	// parity of the 32 bits must be zero if the last bit is equal
	// to the parity of the first 31 bits.
	// Reason: the parity bit xor the parity of the first 31 bits must give
	// zero, unless there was a bit error.
	data ^= data >> 16;
	data ^= data >> 8;
	data ^= data >> 4;
	data &= 0xf;
	data = ((0x6996 >> data) & 1);
	return data == 0;
}

#endif // ALIMUONTRACKERDDLDECODER_H
