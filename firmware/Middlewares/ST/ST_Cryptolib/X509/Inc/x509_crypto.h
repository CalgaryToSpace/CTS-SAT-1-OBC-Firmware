/**
  ******************************************************************************
  * @file    x509_crypto.h
  * @author  AST Security
  * @version V0.1
  * @date    24-February-2017
  * @brief   Crypto functionalities for x509 verification
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2016 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software
  * distributed under the License is distributed on an "AS IS" BASIS,
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */


#ifndef X509_CRYPTO_H
#define X509_CRYPTO_H

#include "x509.h"
#include "crypto.h"

/*
* Verify the signature of a digest with a public key extracted from the certificate
*/
int32_t verifySignature(const intCert_stt *cert,
  const uint8_t *digest,
  int32_t digestSize,
  const uint8_t *signatureR,
  int32_t signatureRsize,
  const uint8_t *signatureS,
  int32_t signatureSsize);

/* Check whether "parent" signed "child" */
int32_t verifyCertSignature(const intCert_stt *parent, const intCert_stt *child);

#endif /*  X509_CRYPTO_H */
