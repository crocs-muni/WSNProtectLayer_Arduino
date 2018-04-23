
/*
* Modified by Martin Sarkany, 2018
*/

#include <string.h>

#include "AES.h"

#define AES_MODE_ENCRYPT    0
#define AES_MODE_DECRYPT    1

extern const unsigned char sbox[256];
// round constant
extern const unsigned char Rcon[10];


void aes_enc_dec(unsigned char *state, unsigned char *key, unsigned char dir);


void AES::keyExpansion(uint8_t *expkey, const uint8_t *key)
{
    uint8_t i;
    uint8_t tmp0,tmp1,tmp2,tmp3,tmp4;

    /*
        The first bytes of the expanded key is the key itself.
    */
    for(i=0;i< KEY_SIZE ;i++){
        expkey[i] = key[i];
    }

    /*
        Key schedule.
    */
    for( i = Nk; i < Nb * (NB_ROUNDS + 1); i++ ){
        tmp0 = expkey[4*i - 4];
        tmp1 = expkey[4*i - 3];
        tmp2 = expkey[4*i - 2];
        tmp3 = expkey[4*i - 1];

        if( !(i % Nk) ) {
            tmp4 = tmp3;
            tmp3 = sbox[tmp0];
            tmp0 = sbox[tmp1] ^ Rcon[i/Nk];
            tmp1 = sbox[tmp2];
            tmp2 = sbox[tmp4];
        } else if( Nk > 6 && i % Nk == 4 ) {
            tmp0 = sbox[tmp0];
            tmp1 = sbox[tmp1];
            tmp2 = sbox[tmp2];
            tmp3 = sbox[tmp3];
        }

        expkey[4*i+0] = expkey[4*i - 4*Nk + 0] ^ tmp0;
        expkey[4*i+1] = expkey[4*i - 4*Nk + 1] ^ tmp1;
        expkey[4*i+2] = expkey[4*i - 4*Nk + 2] ^ tmp2;
        expkey[4*i+3] = expkey[4*i - 4*Nk + 3] ^ tmp3;
    }
}
 
static void aes128_block(unsigned const char *key, unsigned const char* plainText, unsigned char *ciphertext, uint8_t mode) {
    uint8_t key_copy[AES_BLOCK_SIZE];
    memcpy(ciphertext, plainText, AES_BLOCK_SIZE); // prepare plaintext into output buffer as first step
    memcpy(key_copy, key, AES_BLOCK_SIZE);         // create copy as aes_enc_dec will modify key_copy array
    aes_enc_dec(ciphertext, key_copy, mode);
}

// static void aes128_block_encrypt(unsigned const char *key, unsigned const char* plainText, unsigned char *ciphertext) {
//     aes128_block(key, plainText, ciphertext, AES_MODE_ENCRYPT);
// }

#define aes128_block_encrypt(key, plaintext, ciphertext) aes128_block(key, plaintext, ciphertext, AES_MODE_ENCRYPT)
#define aes128_block_decrypt(key, plaintext, ciphertext) aes128_block(key, plaintext, ciphertext, AES_MODE_DECRYPT)

// static void aes128_block_decrypt(unsigned const char *key, unsigned const char* plainText, unsigned char *ciphertext) {
//     aes128_block(key, plainText, ciphertext, AES_MODE_DECRYPT);
// }


bool AES::encrypt(const uint8_t *in_block, uint8_t *expkey, uint8_t *out_block)
{
    if(!in_block || !expkey || !out_block){
        return false;
    }

    aes128_block_encrypt(expkey, in_block, out_block);

    return true;
}

bool AES::decrypt(const uint8_t *in_block, uint8_t *expkey, uint8_t *out_block)
{
    if(!in_block || !expkey || !out_block){
        return false;
    }

    aes128_block_decrypt(expkey, in_block, out_block);

    return true;
}

