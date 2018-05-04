/*
* Copyright (c) 2010 Centre for Electronics Design and Technology (CEDT),
*  Indian Institute of Science (IISc) and Laboratory for Cryptologic
*  Algorithms (LACAL), Ecole Polytechnique Federale de Lausanne (EPFL).
*
* Author: Sylvain Pelissier <sylvain.pelissier@gmail.com>
*
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions
* are met:
* - Redistributions of source code must retain the above copyright
*   notice, this list of conditions and the following disclaimer.
* - Redistributions in binary form must reproduce the above copyright
*   notice, this list of conditions and the following disclaimer in the
*   documentation and/or other materials provided with the
*   distribution.
* - Neither the name of INSERT_AFFILIATION_NAME_HERE nor the names of
*   its contributors may be used to endorse or promote products derived
*   from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
* ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
* FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL STANFORD
* UNIVERSITY OR ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
* INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
* SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
* HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
* STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
* OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/*
* Modified by Martin Sarkany, 03/2018
*/

#ifndef AES_H
#define AES_H

#include <stdint.h>

#include "ProtectLayerGlobals.h"

/*
    The key size in bytes. It can be 16, 24 or 32.
*/
// #define KEY_SIZE    AES_KEY_SIZE

/*
    Number of rounds. It should be 10 for a 16-byte key, 12 for a 24-byte key and 14 for a 32-byte key.
*/
#define NB_ROUNDS   10

/*
    Number of columns in the state and expanded key. It is independent of the key size.
*/
#define Nb 4

/*
    Number of columns in a key. It is 4 for a 16-byte key, 6 for a 24-byte key and 8 for a 32-byte key.
*/
#define Nk 4


class AES: public Cipher {
public:
    virtual void keyExpansion(uint8_t *expkey, const uint8_t *key);

    virtual bool encrypt(const uint8_t *in_block, uint8_t *expkey, uint8_t *out_block);

    virtual bool decrypt(const uint8_t *in_block, uint8_t *expkey, uint8_t *out_block);
};

#endif // AES_H
