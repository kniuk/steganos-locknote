#ifndef AESPHM_H_INCLUDED
#define AESPHM_H_INCLUDED

#include "algparam.h"


NAMESPACE_BEGIN(CryptoPP)


// PADDED AND HASHED AES MESSAGE ENCRYPTION
// ==========================================================================
// This class implements a message-based, rather than byte-based or
// block-based, encryption method. The class uses CFB, but is not itself a
// stream encryptor; it merely uses AES in CFB mode for the internal
// encryption process. The minimum ciphertext size is 46 bytes, and the
// generated ciphertext will be 32 to 46 bytes larger than the corresponding
// plaintext. Adding random padding is necessary to hide the real length of
// small messages, and to make some attacks a bit more difficult.


// SECURITY
// ==========================================================================
// AES PHM provides the following security features:
// - Encrypting twice with the same plaintext and the same key will never
//   produce the same ciphertext. The minimum estimated possibility that any
//   two encryptions of the same message with the same key will produce the
//   same plaintext is 2^-96, assuming a flawless random number generator
//   is supplied. (Padding is at least 32 bits, IV seed is 64 bits)
// - The exact length of plaintext cannot be determined by looking at the
//   ciphertext. Plaintext is padded so that padded plaintext length is
//   always (3 + 15*n) bytes, where n = 1, 2, 3, 4, ...
// - The decryption routine is always able to validate that the supplied
//   combination of ciphertext and key is valid. Any tampering with the
//   ciphertext will be detected. The estimated failure rate for this
//   detection process is 2^-160, assuming SHA1 is an ideal hash function.
//   (An HMAC<SHA> code is appended to encrypted messages)


// ENCRYPTION PROCESS
// ==========================================================================
// input: <plaintext>, output: <ciphertext>
//
// <encrypted> = encrypted-with-AES-in-CFB-mode(<padding>, <plaintext>)
// <ciphertext> = <encrypted>, <iv-seed>, <mac>
//
// The total length of padding is equal to the value of the lowest 4 bits of
// the first byte of the padding, plus 3.
//
// Due to unfortunate placement of the padding length indicator at the
// very beginning of the ciphertext, the whole message must be loaded in
// memory before it can be encrypted. Likewise, storage of the IV seed
// after the ciphertext prevents decryption from commencing before the
// whole message has been read. This makes AES PHM inappropriate for
// encrypting very large messages.
//
// AES CFB encryption parameters are:
//  - the encryption key is obtained by hashing the passphrase with SHA256;
//  - the IV is obtained from a randomly generated 64-bit seed.
//
// The MAC is HMAC-SHA over <raw-ciphertext> and <iv-seed>.


// KEY LIFETIME
// ==========================================================================
// An attacker that spots two messages encrypted with the same key and IV
// will be able to recover the plaintext. Note that the IV is calculated from
// a 64-bit seed. Assuming the underlying random data provider is flawless,
// the following table lists 1) the number of total messages encrypted with
// the same key, and 2) the corresponding probability that at least two of
// these messages will have been encrypted with the same IV:
//
//      total      collision probability
//      --------------------------------
//      2^27       00.05%
//      2^28       00.2 %
//      2^29       00.8 %
//      2^30       03   %
//      2^31       11   %
//      2^32       39   %
//      2^33       86   %
//      2^34       99.9 %
//
// If you plan to encrypt a very large number of messages without changing
// the key, consider these numbers and use them to calculate the lifetime of
// your encryption keys.



class AESPHM
{
public:
	// Provides an upper limit, but not an exact limit, to the length of the ciphertext
	// that will be produced when encrypting plaintext of a specified size.
	static unsigned int MaxCiphertextLen(unsigned int plaintextLen) { return plaintextLen + 50; }

	// Use MaxCiphertextLen() to allocate an output buffer of appropriate size
	static unsigned int Encrypt(
		RandomNumberGenerator& rng,
		ConstByteArrayParameter const& passphrase,
		byte* output,
		ConstByteArrayParameter const& input);

	// Allocate an output buffer that is as large as the input
	static DecodingResult Decrypt(
		ConstByteArrayParameter const& passphrase,
		byte* output,
		ConstByteArrayParameter const& input);
};



/*
class AES_PHM_Encryption : public Filter
{
public:
	// The key may be arbitrary data; may also be textual passphrase
	AES_PHM_Encryption(RandomNumberGenerator& rng, byte const* pbKey, unsigned int nKeyLen, BufferedTransformation* pOutQueue = 0);

	// Provides an upper limit, but not an exact limit, to the length of the ciphertext
	// that will be produced when encrypting plaintext of a specified size.
	static unsigned int MaxCiphertextLen(unsigned int nPlaintextLen) { return nPlaintextLen + 50; }
	unsigned int MaxCiphertextLen() const { return MaxCiphertextLen(GetPlaintextLen()); }

	// Store data for later encryption (see MessageEnd())
	void Put(byte b) { Put(&b, 1); }
	void Put(byte const* pbData, unsigned int nDataLen);

	// For applications which want to write to m_sbbPlaintext directly
	// to avoid creating multiple copies of input data, or to avoid
	// excess memory allocations. When feeding input data other than through
	// the Put() interface, always call SetPlaintextLen() before calling MessageEnd().
	SecByteBlock& GetPlaintextBuffer() { return m_sbbPlaintext; }
	unsigned GetPlaintextLen() const { return m_nPlaintextLen; }
	void SetPlaintextLen(unsigned int nPlaintextLen) { m_nPlaintextLen = nPlaintextLen; }

	// Starts the actual encryption process
	void MessageEnd(int nPropagation = -1);

private:
	RandomNumberGenerator& m_rng;
	SecByteBlock m_sbbKey;
	SecByteBlock m_sbbPlaintext;
	unsigned int m_nPlaintextLen;
};


template <class InStringType, class OutStringType>
void AES_PHM_Encrypt(
	RandomNumberGenerator& rng,
	byte const* pbKey,
	unsigned int nKeyLen,
	InStringType const& sInput,
	OutStringType& sOutput)
{
	CryptoPP::StringSource(sInput, true,
		new CryptoPP::AES_PHM_Encryption(rng, pbKey, nKeyLen,
			new CryptoPP::StringSinkTemplate<OutStringType>(sOutput)));
}

template <class KeyStringType, class InStringType, class OutStringType>
void AES_PHM_Encrypt(
	RandomNumberGenerator& rng,
	KeyStringType const& sKey,
	InStringType const& sInput,
	OutStringType& sOutput)
{
	AES_PHM_Encrypt(rng, (byte const*) sKey.data(), sKey.length(), sInput, sOutput);
}



class AES_PHM_Decryption : public Filter
{
public:

	class InvalidCiphertextOrKey : public BufferedTransformation::Err
	{
	public:
		InvalidCiphertextOrKey() : BufferedTransformation::Err(
			DATA_INTEGRITY_CHECK_FAILED,
			"AES_PHM_Decryption: Invalid Ciphertext or Key") {}
	};

	AES_PHM_Decryption(byte const* pbKey, unsigned int nKeyLen, BufferedTransformation* pOutQueue = 0);

	// Store data for later decryption (see MessageEnd())
	void Put(byte b) { Put(&b, 1); }
	void Put(byte const* pbData, unsigned int nDataLen);

	// For applications which want to write to m_sbbCiphertext directly
	// to avoid creating multiple copies of input data, or to avoid
	// excess memory allocations. When feeding input data other than through
	// the Put() interface, always call SetPlaintextLen() before calling MessageEnd().
	SecByteBlock& GetCiphertextBuffer() { return m_sbbCiphertext; }
	unsigned int GetCiphertextLen() const {return m_nCiphertextLen; }
	void SetCiphertextLen(unsigned int nCiphertextLen) { m_nCiphertextLen = nCiphertextLen; }

	// Starts the actual decryption process
	void MessageEnd(int nPropagation = -1);

private:
	SecByteBlock m_sbbKey;
	SecByteBlock m_sbbCiphertext;
	unsigned int m_nCiphertextLen;
};


template <class InStringType, class OutStringType>
void AES_PHM_Decrypt(
	byte const* pbKey,
	unsigned int nKeyLen,
	InStringType const& sInput,
	OutStringType& sOutput)
{
	CryptoPP::StringSource(sInput, true,
		new CryptoPP::AES_PHM_Decryption(pbKey, nKeyLen,
			new CryptoPP::StringSinkTemplate<OutStringType>(sOutput)));
}

template <class KeyStringType, class InStringType, class OutStringType>
void AES_PHM_Decrypt(
	KeyStringType const& sKey,
	InStringType const& sInput,
	OutStringType& sOutput)
{
	AES_PHM_Decrypt((byte const*) sKey.data(), sKey.length(), sInput, sOutput);
}
*/


NAMESPACE_END


#endif
