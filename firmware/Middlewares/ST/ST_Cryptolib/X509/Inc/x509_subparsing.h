/**
  ******************************************************************************
  * @file    x509_subparsing.h
  * @author  AST Security
  * @version V0.2
  * @date    16-November-2016
  * @brief   helper for the x509 certificate Parser
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


#ifndef X509_SUBPARSING_H
#define X509_SUBPARSING_H

#include "x509.h"

int32_t identifyASN1TLV(const uint8_t *asn1, int32_t *parsed, int32_t *size, const uint8_t **value);

int32_t identifyAttribute(const uint8_t *oid, int32_t size);

void parseIntegerValue(const uint8_t *value, int32_t size, const uint8_t **outp, int32_t *outSize);

void parseECDSAsignature(const uint8_t *signature, intCert_stt *intCert, const uint8_t **next_thing);

void parseX509version(const uint8_t *x509VersionField, intCert_stt *intCert, const uint8_t **next_thing);

void parseSignatureAlgorithm(const uint8_t *SA, int32_t *singatureAlgorithm, const uint8_t **next_thing);

void parseECCPublicKey(const uint8_t *EccPK, intCert_stt *intCert, const uint8_t **next_thing);

void parseInteger(const uint8_t *integer, const uint8_t **outp, int32_t *outSize, const uint8_t **next_thing);

void parseValidity(const uint8_t *p, validity_stt *notBefore_st, validity_stt *notAfter_st, const uint8_t **next_thing);

void parseRDN(const uint8_t *p, const uint8_t **nextRDN, const uint8_t **attribute);

void parseAttribute(const uint8_t *p, attribute_stt *attribute_st, const uint8_t **next_thing);

int32_t dateCompare(const validity_stt *D1, const validity_stt *D2);

int32_t countAttributes(const uint8_t *p);

int32_t caseInsensitiveCmp(const uint8_t *p1, const uint8_t *p2, int32_t size);

int32_t identifyExtension(const uint8_t *oid, int32_t size);

void parseExtension(const uint8_t *ext, uint32_t *extFlags, extension_stt *ext_st);

int32_t getSmallInteger(const uint8_t *value, int32_t size);

#endif /* X509_SUBPARSING_H */
