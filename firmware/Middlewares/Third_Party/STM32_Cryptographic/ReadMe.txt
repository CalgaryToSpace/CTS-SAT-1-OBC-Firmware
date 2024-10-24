/*************************************************************************************************
*
* Software implementation of STM32 Cryptographic libraries: High Level API
*
**************************************************************************************************
*
* Copyright (c) 2021 STMicroelectronics.
* All rights reserved.
*
* This software is licensed under terms that can be found in the LICENSE file
* in the root directory of this software component.
**************************************************************************************************
*
* This directory contains the header files of the Cryptographic services exposed by the
* STM32 Cryptographic libraries.
* The Cryptographic services are the followings:
* * AES-128/192/256 Encryption/Decryption in ECB/CBC/CFB/CTR/OFB/XTS/KeyWrap modes
* * AES-128/192/256 CCM/GCM Authenticated Encryption/Decryption
* * SM4 Encryption/Decryption in ECB/CBC/CFB/CTR/OFB modes
* * ChaCha20-Poly1305 Authenticated Encryption/Decryption
* * CTR-DRBG based on AES-128/256
* * SHA1/SHA2/SHA3/SHAKE/SM3 Hash functions
* * HMAC/CMAC/KMAC Message Authentication Code functions
* * RSA PKCS#1 v1.5/v2.2 Signature Creation/Verification and Encryption/Decryption functions
* * ECDSA/EdDSA/SM2 Key Generation and Signature Creation/Verification functions
* * ECDH Key Exchange functions
*
* Some parts of the STM32 Cryptographic libraries has been inspired from third-party SW components
* from:
* * muNaCl, by Michael Hutter, Peter Schwabe, Björn Haase and Ana Helena Sánchez
* * libtomcrypt, by T. St-Denis
* * eXtended Keccak Code Package, by Guido Bertoni, Joan Daemen, Seth Hoffert, Michaël Peeters,
*   Gilles Van Assche and Ronny Van Keer
* * poly1305-donna, by Andrew Moon
* * chacha-ref.c, by D. J. Bernstein
***************************************************************************************************/
