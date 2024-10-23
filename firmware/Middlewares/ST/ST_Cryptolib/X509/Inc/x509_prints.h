/**
  ******************************************************************************
  * @file    x509_prints.h
  * @author  AST Security
  * @version V0.2
  * @date    16-November-2016
  * @brief   informational functions for the x509 parsing
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


#ifndef X509_PRINTS_H
#define X509_PRINTS_H

#include "x509.h"

void printAttribute(int32_t type);

void printSignatureAlgorithm(int32_t type);

void printTag(int32_t tag);

void printEllipticCurve(int32_t type);

int32_t printInteger(const uint8_t *value, int32_t size);

int32_t printBitString(const uint8_t *value, int32_t size);

void printParsedCert(intCert_stt *intCert);

void parsePrintName(const uint8_t *name, int32_t size);

void printValidity(const uint8_t *validity);

void simpleParseAttribute(const uint8_t *p, attribute_stt *attribute_st, const uint8_t **next_thing);



#endif /* X509_PRINTS_H */
