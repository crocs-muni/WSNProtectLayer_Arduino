#ifndef CRYPTO_H
#define CRYPTO_H

// #include "AES.h"
#include "common.h"
#include "AES_crypto.h"
#include "KeyDistrib.h"
#include "ProtectLayerGlobals.h"
// #include "uTESLA.h"

#ifndef MAC_LENGTH
#define MAC_LENGTH 	AES_MAC_SIZE
#endif

#ifndef BLOCK_SIZE
#define BLOCK_SIZE 	AES_BLOCK_SIZE
#endif

#ifndef NULL
#define NULL 		(void*)0
#endif

#define MAX_OFFSET						20
#define COUNTER_SYNCHRONIZATION_WINDOW	5

extern uint8_t expanded_key[240]; //expanded key

enum { EWRONGHASH = 5, EWRONGMAC };

class Crypto {
    Cipher      *m_cipher;
    MAC         *m_mac;
    Hash        *m_hash;
    KeyDistrib  *m_keydistrib;

	PL_key_t 	*m_key1;
	uint8_t     *m_exp; //expanded key
public:
    Crypto(Cipher *cipher, MAC *mac, Hash *hash, KeyDistrib *keydistrib);

    //Node variants
	/**
			Command: Blocking version. Used by other components to calculate mac of buffer and then 
			encrypt it. Offset can be used to shift encryption, i.e. header is included in mac calculation, but 
			is not encrypted. Enough additional space in buffer to fit encrypted content is assumed.
			Function keeps track of couter values for independent nodes.
			@param[in] nodeID node identification of node
			@param[in out] buffer buffer to be encrypted, wil contain encrypted data
			@param[in] offset of encryption
			@param[in out] pLen length of buffer to be encrypted, will contain resulting length with mac
			@return uint8_t status
	*/	
	uint8_t protectBufferForNodeB( node_id_t nodeID, uint8_t* buffer, uint8_t offset, uint8_t* pLen);

	/**
			Command: Blocking version. Used by other components to start decryption of supplied buffer.
			Function verifies appended mac. Function keeps track of couter values for independent nodes.
			Function is capable of counter synchronization. Offset can be used for specificaton of used 
			encryption shift (i.e. header was included for mac calculation but not encrypted)
			@param[in] nodeID node identification of node			
			@param[in] buffer buffer to be decrypted
			@param[in] offset
			@param[in] len length of buffer to be decrypted
			@return uint8_t status
	*/
	uint8_t unprotectBufferFromNodeB( node_id_t nodeID, uint8_t* buffer, uint8_t offset, uint8_t* pLen);
	
	//BS variants
	/**
			Command: Blocking version. Used by other components to calculate mac of buffer and then 
			encrypt it for communication with BS. Offset can be used to shift encryption, i.e. header 
			is included in mac calculation, but is not encrypted. Enough additional space in buffer to fit 
			encrypted content is assumed. Function keeps track of couter values for independent nodes.
			@param[in out] buffer buffer to be encrypted, wil contain encrypted data
			@param[in] offset
			@param[in out] pLen length of buffer to be encrypted, will contain resulting length with mac
			@return uint8_t status
	*/
	uint8_t protectBufferForBSB( uint8_t* buffer, uint8_t offset, uint8_t* pLen);

	/**
			Command: Blocking version. Used by other components to start decryption of supplied buffer received from BS.
			Function verifies appended mac. Function keeps track of couter values for independent nodes.
			Function is capable of counter synchronization. Offset can be used for specificaton of used 
			encryption shift (i.e. header was included for mac calculation but not encrypted)
			@param[in] buffer buffer to be decrypted
			@param[in] offset shift in 
			@param[in] len length of buffer to be decrypted
			@return uint8_t status
	*/
	uint8_t unprotectBufferFromBSB( uint8_t* buffer, uint8_t offset, uint8_t* pLen);
	
	
	
	/**
			Command: Blocking version. Used by other components to calculate mac of data for node.
			Enough additional space in buffer to fit mac content is assumed
			computes mac over supplied buffer, starting from offset with pLen number of bytes
			Note that bytes before offset are not included to mac malculation.
			MAC Length is defined as MAC_LENGTH
			@param[in] nodeID node identification of node
			@param[in out] buffer buffer for mac calculation, mac will be appended to data
			@param[in] offset
			@param[in out] pLen length of buffer starting from offset, mac calculation, will contain length with appended mac
			@return uint8_t status
	*/
	uint8_t macBufferForNodeB(node_id_t nodeID, uint8_t* buffer, uint8_t offset, uint8_t* pLen);
	
	/**
			Command: Blocking version. Used by other components to calculate mac of data for BS.
			Enough additional space in buffer to fit mac content is assumed.			
			computes mac over supplied buffer, starting from offset with pLen number of bytes
			Note that bytes before offset are not included to mac malculation.
			MAC Length is defined as MAC_LENGTH
			@param[in out] buffer buffer for mac calculation, mac will be appended to data
			@param[in] offset
			@param[in out] pLen length of buffer for mac calculation, will contain length with appended mac
			@return uint8_t status
	*/
	uint8_t macBufferForBSB( uint8_t* buffer, uint8_t offset, uint8_t* pLen);
	
	/**
			Command: Blocking version. Used by other components to verify mac of data for node.						
			@param[in] nodeID node identification of node
			@param[in out] buffer buffer containing data and appended mac
			@param[in] offset
			@param[in out] pLen length of buffer with mac
			@return uint8_t status
	*/
	uint8_t verifyMacFromNodeB( node_id_t nodeID, uint8_t* buffer, uint8_t offset, uint8_t* pLen);
	
	/**
			Command: Blocking version. Used by other components to verify mac of data for BS.
			@param[in out] buffer buffer containing data and appended mac
			@param[in] offset
			@param[in out] pLen length of buffer with mac
			@return uint8_t status
	*/
	uint8_t verifyMacFromBSB( uint8_t* buffer, uint8_t offset, uint8_t* pLen);
	
	/**
			Command: Blocking function to initialize shared keys between nodes.
			Gets nodeID of neighbours from SavedData, for these finds predistributed keys in KDCPrivData.
			derives new shared key and stores key in KDCData.
			@return uint8_t status
	*/
	uint8_t initCryptoIIB();
	
	/**	
			Command: function to calculate AES based hash of data in buffer.
			Resulting hash has AES BLOCK_LENGTH
			Output array can be same as input array.
			@param[in] buffer with data
			@param[in] offset
			@param[in] len
			@param[out] hash calculated hash of data
			@return uint8_t status
	*/
	uint8_t hashDataB( uint8_t* buffer, uint8_t offset, uint8_t len, uint8_t* hash);
		
	/**	
			Command: function to calculate AES based hash of data in buffer.
			Resulting hash has uint64_t format
			@param[in] buffer with data
			@param[in] offset
			@param[in] pLen
			@param[out] hash calculated hash of data
			@return uint8_t status
	*/
	uint8_t hashDataShortB( uint8_t* buffer, uint8_t offset, uint8_t len, uint32_t* hash);
	
	/**	
			Command: function to verify hash of data
			@param[in] buffer with data
			@param[in] offset			
			@param[in] pLen
			@param[in] hash to verify
			@return uint8_t result
	*/
	uint8_t verifyHashDataB( uint8_t* buffer, uint8_t offset, uint8_t pLen, uint8_t* hash);
	
	/**	
			Command: function to verify first half of hash
			@param[in] buffer with data
			@param[in] offset			
			@param[in] pLen
			@param[in] hash to verify
			@return uint8_t result
	*/
	uint8_t verifyHashDataShortB( uint8_t* buffer, uint8_t offset, uint8_t pLen, uint32_t hash);
	
	/**	
			Command: Command to calculate hash chain of buffer and verifies result of calculation 
			according to privacy level specified. Input Length is SIGNATURE_LENGTH.
			Optionally returns updated signature, which can be stored using updateSignature function.
			@param[in] buffer with signature to verify
			@param[in] offset
			@param[in] level privacy level
			@param[in] counter supposed placement in hash chain for verified signature, 0 is for predistributed value
			@param[out] signature optional, when not NULL, then filled with updated signature, array must have length of HASH_LENGTH
			@return uint8_t return verification result. 
	*/
	// uint8_t verifySignature( uint8_t* buffer, uint8_t offset, PRIVACY_LEVEL level, uint16_t counter, Signature_t* signature);
	
	/**
	                Command: command to update last verified signature stored in memory
	                @param[in] signature value to update, length is required to be HASH_LENGTH
	*/	
	// void updateSignature( Signature_t* signature);

private:

	/**
		Command: Blocking version. Used by other components to start encryption of supplied buffer by supplied key.
		Enough additional space in buffer to fit encrypted content is assumed.
		@param[in] key handle to the key that should be used for encryption
		@param[in out] counter counter value before, updated to new value after encryption
		@param[in out] buffer buffer to be encrypted, wil contain encrypted data
		@param[in] offset
		@param[in out] pLen length of buffer to be encrypted, will contain resulting length
		@return error_t status
	*/
	uint8_t encryptBufferB(PL_key_t* key, uint8_t* buffer, uint8_t offset, uint8_t len);
	
	/**
		Command: Blocking version. Used by other components to start decryption of supplied buffer by supplied key.
		Enough space in buffer to fit decrypted content is assumed.
		Because of use of counter mode, this function uses encrypt buffer function.
		@param[in] key handle to the key that should be used for encryption
		@param[in out] counter counter value before, updated to new value after encryption
		@param[in out] buffer buffer to be encrypted, wil contain encrypted data
		@param[in] offset
		@param[in out] pLen length of buffer to be encrypted, will contain resulting length
		@return error_t status
	*/
	uint8_t decryptBufferB(PL_key_t* key, uint8_t* buffer, uint8_t offset, uint8_t len);
		
	/**
		Command: Used by other components to derive new key from master key and derivation data. 
		@param[in] masterKey handle to the master key that will be used to derive new one
		@param[in] derivationData buffer containing derivation data
		@param[in] offset offset inside derivationData buffer from which derivation data start
		@param[in] len length of derivation data, should be AES block size
		@param[out] derivedKey resulting derived key
		@return error_t status
	*/	 
	uint8_t deriveKeyB(PL_key_t* masterKey, uint8_t* derivationData, uint8_t offset, uint8_t len, PL_key_t* derivedKey);
		
	/**	
		Command: function to calculate AES based hash of data in buffer.
		makes one iteration. Length of data is aes block size
		@param[in out] buffer with data, replaced with calculated hash
		@param[in] offset			
		@param[in] key key for encryption
		@param[out] hash calculated value
		@return error_t status
	*/
	uint8_t hashDataBlockB( uint8_t* buffer, uint8_t offset, PL_key_t* key, uint8_t* hash);
	/**
		Command: Used by Crypto component as inner function for calculating mac of buffer.		
		@param[in] key handle for key, that will be used foe mac calculation
		@param[in] buffer data that will be processed
		@param[in] offset of buffer
		@param[in] pLen length of data in buffer
		@param[out] mac calculated mac of data, space of MAC_LENGTH must be available in memory
		@return error_t status
	*/
	uint8_t macBuffer(PL_key_t* key, uint8_t* buffer, uint8_t offset, uint8_t* pLen, uint8_t* mac);
	
	/**
		Command: Used by Crypto component as inner function for verification of mac.
		@param[in] key handle for key, that was used for mac calculation
		@param[in] buffer with original data
		@param[in] offset of buffer
		@param[in] pLen length of data in buffer with mac to verify
		@param[in] mac value that will be verified against supplied data in buffer
		@return error_t status
	*/
	uint8_t verifyMac(PL_key_t* key, uint8_t* buffer, uint8_t offset, uint8_t* pLen);
	
	/**
		Command: Used by Crypto component as inner function for mac calculation and encryption of buffer.
		mac is appended to the buffer, so additional space of MAC_LENGTH is required.
		offset can be used for shift of encryption, therefore mac will be calculated from whole payload including header
		but header will stay unecrypted. Mac is calculated first and then is encrypted payload including mac.
		@param[in] key handle for key fro encryption and mac calculation
		@param[in out] buffer with original data
		@param[in] offset of encryption
		@param[in out] pLen length of data, will contain length of data with mac
		@return error_t status
	*/
	uint8_t protectBufferB( PL_key_t* key, uint8_t* buffer, uint8_t offset, uint8_t* pLen);
	
	/**
		Command: Used by Crypto component as inner function for mac verification and decryption of buffer.
		buffer if first decrypted and then if verified mac. If mac does not match, attempt is made to 
		synchronize counter value in range of COUNTER_SYNCHRONIZATION_WINDOW. If synchronization is 
		succesfull, counter is updated to right value. Offset is used to specifie ecryption offset used.
		(i.e. header is not encrypted, but included in mac calculation)
		@param[in] key handle for key for decryption and mac verification
		@param[in out] buffer with original data
		@param[in] offset of encryption
		@param[in] pLen length of data in buffer
		@return error_t status
	*/
	uint8_t unprotectBufferB( PL_key_t* key, uint8_t* buffer, uint8_t offset, uint8_t* pLen);
};

#endif //  CRYPTO_H