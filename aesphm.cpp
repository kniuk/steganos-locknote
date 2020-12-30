// Crypto++
#include "sha.h"
#include "aes.h"
#include "modes.h"
#include "hmac.h"
#include "fltrimpl.h"

// AES PHM
#include "aesphm.h"


// debug
#include "hex.h"
#include "files.h"
#include <iostream>
using namespace std;



NAMESPACE_BEGIN(CryptoPP)



enum
{
	MINIMUM_CIPHERTEXT_LENGTH = 46,		// 18 bytes padded plaintext, 20 bytes MAC, 8 bytes IV seed
	IV_SEED_SIZE = 8,
	IV_SIZE = 16
}; 



void AESPHM_GenerateIvFromSeed(byte const* ivSeed, byte* iv)
{
	SecByteBlock ivHash(32);
	SHA256().CalculateDigest(ivHash, ivSeed, IV_SEED_SIZE);

	// Assumes IV_SIZE == 16
	for (unsigned int i=0; i!=16; ++i)
		iv[i] = ivHash[i] ^ ivHash[i+16];
}



unsigned int AESPHM::Encrypt(
	RandomNumberGenerator& rng,
	ConstByteArrayParameter const& passphrase,
	byte* output,
	ConstByteArrayParameter const& input)
{
	// Generate padding
	unsigned int paddingLen = (15 - (input.size() % 15)) + 3;
	rng.GenerateBlock(output, paddingLen);
	output[0] = ((output[0] ^ (output[0] << 4)) & 0xf0) | ((paddingLen - 3) & 0x0f);

	// Establish ciphertext segment locations
	byte* payload = output + paddingLen;
	byte* ivSeed = payload + input.size();
	byte* digest = ivSeed + IV_SEED_SIZE;
	unsigned int outputLen = (digest + HMAC<SHA>::DIGESTSIZE) - output;

	// Generate IV seed, IV, and key; setup encryptor
	rng.GenerateBlock(ivSeed, IV_SEED_SIZE);

	SecByteBlock key(SHA256::DIGESTSIZE);
	SHA256().CalculateDigest(key.begin(), passphrase.begin(), passphrase.size());

	SecByteBlock iv(IV_SIZE);
	AESPHM_GenerateIvFromSeed(ivSeed, iv.begin());

	CFB_Mode<AES>::Encryption encryptor(key.begin(), key.size(), iv.begin());

	// Encrypt padding and payload
	encryptor.ProcessString(output, paddingLen);
	encryptor.ProcessString(payload, input.begin(), input.size());

	// Calculate MAC
	HMAC<SHA>(key.begin(), key.size()).CalculateDigest(digest, output, digest-output);

	// Done
	return outputLen;
}



DecodingResult AESPHM::Decrypt(
	ConstByteArrayParameter const& passphrase,
	byte* output,
	ConstByteArrayParameter const& input)
{
	if (input.size() < MINIMUM_CIPHERTEXT_LENGTH)
		return DecodingResult();

	// Establish locations of IV seed and MAC
	byte const* digest = input.end() - HMAC<SHA>::DIGESTSIZE;
	byte const* ivSeed = digest - IV_SEED_SIZE;

	// Generate key, verify MAC
	SecByteBlock key(SHA256::DIGESTSIZE);
	SHA256().CalculateDigest(key.begin(), passphrase.begin(), passphrase.size());

	byte checkDigest[HMAC<SHA>::DIGESTSIZE];
	HMAC<SHA>(key.begin(), key.size()).CalculateDigest(checkDigest, input.begin(), digest-input.begin());
	if (memcmp(digest, checkDigest, sizeof checkDigest) != 0)
		return DecodingResult();

	// Generate IV, setup decryptor
	SecByteBlock iv(IV_SIZE);
	AESPHM_GenerateIvFromSeed(ivSeed, iv.begin());

	CFB_Mode<AES>::Decryption decryptor(key.begin(), key.size(), iv.begin());

	// See how much padding we have
	byte firstPadByte = 0;
	decryptor.ProcessString(&firstPadByte, input.begin(), 1);

	unsigned int paddingLen = (firstPadByte & 0x0f) + 3;
	if (input.begin() + paddingLen > ivSeed)
		return DecodingResult();

	byte const* payload = input.begin() + paddingLen;
	unsigned int payloadLen = ivSeed - payload;

	// Decrypt payload
	if (payloadLen)
	{
		decryptor.ProcessString(output, input.begin()+1, paddingLen-1);
		decryptor.ProcessString(output, payload, payloadLen);
	}

	return DecodingResult(payloadLen);
}



/*


/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////


void AES_PHM_GenerateIvFromSeed(byte const* pbIvSeed, byte* pbIv)
{
	SecByteBlock sbbIvHash(SHA256::DIGESTSIZE);
	SHA256().CalculateDigest(sbbIvHash, pbIvSeed, IV_SEED_SIZE);

	// Assuming that SHA256::DIGESTSIZE == 32, IV_SIZE == 16
	for (unsigned int i=0; i!=IV_SIZE; ++i)
		pbIv[i] = sbbIvHash[i] ^ sbbIvHash[i+16];
}

void AES_PHM_AppendToBuffer(
	SecByteBlock& sbb, unsigned int& nTotalLen,
	byte const* pbData, unsigned int nDataLen)
{
	if (nTotalLen + nDataLen > sbb.Size())
		sbb.Grow(2*(nTotalLen + nDataLen));

	memcpy(sbb+nTotalLen, pbData, nDataLen);
	nTotalLen += nDataLen;
}



/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////


AES_PHM_Encryption::AES_PHM_Encryption(RandomNumberGenerator& rng, 
	byte const* pbKey, unsigned int nKeyLen, BufferedTransformation* pOutQueue)
		: 	Filter(pOutQueue), m_rng(rng), m_nPlaintextLen(0)
{
	m_sbbKey.Resize(SHA256::DIGESTSIZE);
	SHA256().CalculateDigest(m_sbbKey, pbKey, nKeyLen);
}

void AES_PHM_Encryption::Put(byte const* pbData, unsigned int nDataLen)
{
	AES_PHM_AppendToBuffer(m_sbbPlaintext, m_nPlaintextLen, pbData, nDataLen);
}

void AES_PHM_Encryption::MessageEnd(int nPropagation)
{
	SecByteBlock sbbPadding(18);

	// We want the plaintext length to be a multiple of 15 bytes
	unsigned int nRandomPaddingLen = 15 - (m_nPlaintextLen%15);
	byte byFirst = m_rng.GenerateByte();
	sbbPadding[0] = ((byFirst ^ (byFirst << 4)) & 0xf0) | (nRandomPaddingLen & 0x0f);
	nRandomPaddingLen += 3;
	m_rng.GenerateBlock(sbbPadding+1, nRandomPaddingLen-1);

	// Generate IV seed
	SecByteBlock sbbIvSeed(IV_SEED_SIZE);
	m_rng.GenerateBlock(sbbIvSeed, IV_SEED_SIZE);

	// Generate IV from seed
	SecByteBlock sbbIv(IV_SIZE);
	AES_PHM_GenerateIvFromSeed(sbbIvSeed, sbbIv);

	// Initialize AES
	AESEncryption aes(m_sbbKey, SHA256::DIGESTSIZE);
	CFBEncryption encryptor(aes, sbbIv);

	// Encrypt
	encryptor.ProcessString(sbbPadding, nRandomPaddingLen);
	encryptor.ProcessString(m_sbbPlaintext, m_nPlaintextLen);

	// Calculate MAC over encrypted plaintext and IV seed
	SecByteBlock sbbMac(HMAC<SHA>::DIGESTSIZE);
	HMAC<SHA> mac(m_sbbKey, SHA256::DIGESTSIZE);
	mac.Update(sbbPadding, nRandomPaddingLen);		// encrypted
	mac.Update(m_sbbPlaintext, m_nPlaintextLen);	// encrypted
	mac.Update(sbbIvSeed, IV_SEED_SIZE);			// plain
	mac.Final(sbbMac);

	// Output complete ciphertext
	if (AttachedTransformation())
	{
		AttachedTransformation()->Put(sbbPadding, nRandomPaddingLen); 	// encrypted
		AttachedTransformation()->Put(m_sbbPlaintext, m_nPlaintextLen); // encrypted
		AttachedTransformation()->Put(sbbIvSeed, IV_SEED_SIZE);			// plain
		AttachedTransformation()->Put(sbbMac, HMAC<SHA>::DIGESTSIZE);	// plain
	}

	m_sbbPlaintext.Resize(0);
	m_nPlaintextLen = 0;

	Filter::MessageEnd(nPropagation);
}



/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////


AES_PHM_Decryption::AES_PHM_Decryption(
	byte const* pbKey, unsigned int nKeyLen, BufferedTransformation* pOutQueue)
		: 	Filter(pOutQueue), m_nCiphertextLen(0)
{
	m_sbbKey.Resize(SHA256::DIGESTSIZE);
	SHA256().CalculateDigest(m_sbbKey, pbKey, nKeyLen);
}

void AES_PHM_Decryption::Put(byte const* pbData, unsigned int nDataLen)
{
	AES_PHM_AppendToBuffer(m_sbbCiphertext, m_nCiphertextLen, pbData, nDataLen);
}

void AES_PHM_Decryption::MessageEnd(int nPropagation)
{
	if (m_nCiphertextLen < MINIMUM_CIPHERTEXT_LENGTH)
		throw InvalidCiphertextOrKey();

	// Make the code less cluttered by using the following values
	unsigned int nMacPosition = m_nCiphertextLen - HMAC<SHA>::DIGESTSIZE;
	unsigned int nIvSeedPosition = nMacPosition - IV_SEED_SIZE;

	// Verify MAC
	if (!HMAC<SHA>(m_sbbKey, SHA256::DIGESTSIZE).VerifyDigest(
			m_sbbCiphertext + nMacPosition,
			m_sbbCiphertext, nMacPosition))
		throw InvalidCiphertextOrKey();

	// Generate IV from seed
	SecByteBlock sbbIv(IV_SIZE);
	AES_PHM_GenerateIvFromSeed(m_sbbCiphertext + nIvSeedPosition, sbbIv);

	// Initialize AES
	AESEncryption aes(m_sbbKey, SHA256::DIGESTSIZE);
	CFBDecryption decryptor(aes, sbbIv);

	// Decrypt
	decryptor.ProcessString(m_sbbCiphertext, nIvSeedPosition);

	// See how much padding we have
	unsigned int nRandomPaddingLen = (m_sbbCiphertext[0] & 0x0f) + 3;

	// Output plaintext
	if (AttachedTransformation())
		AttachedTransformation()->Put(
			m_sbbCiphertext + nRandomPaddingLen,
			nIvSeedPosition - nRandomPaddingLen);

	m_sbbCiphertext.Resize(0);
	m_nCiphertextLen = 0;

	Filter::MessageEnd(nPropagation);
}


/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////


*/


NAMESPACE_END
