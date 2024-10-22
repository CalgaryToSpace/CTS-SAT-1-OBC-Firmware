/**
  ******************************************************************************
  * @file    x509_subparsing.c
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


#include "x509_subparsing.h"
#include <string.h>

/**
* @brief  Identify the ASN.1 TLV
* @param[in]  *asn1 pointer to the ASN.1 TLV
* @param[out] *parsed will contain the number of processed bytes (rarely used)
* @param[out] *size  will contain the size (in bytes) of the value field of this ASN.1 TLV
* @param[out] **value  will contain a pointer to the value field of this ASN.1 TLV
* @return   The internal TAG identifier of this TLV or -1 if failure
*/
int32_t identifyASN1TLV(const uint8_t *asn1, int32_t *parsed, int32_t *size, const uint8_t **value)
{
  /* TLV format has the TAG in the first byte*/
  int32_t tag = (int32_t)asn1[0];
  uint32_t count, i;

  *parsed = 0;
  /* Check if TAG is valid */
  if (IS_VALID_TAG(tag) == 0)
  {
    //printf("Error! Tag %d is not valid\n", tag);
    return -1;
  }
  /*Second byte tells us about the size if it's < 128. Otherwise if bit7 is set, it tell us how many bytes encode the size */
  if (asn1[1] >> 7 == 1U) /* if bit7 is set, then size is expressed over multiple byte*/
  {
    count = (uint32_t)asn1[1] & 127U; /* Read the number of bytes expressing the size */
    /* Read the size */
    *size = 0;
    for (i = 0; i < count; i++)
    {
      *size <<= 8U;
      *size += (int32_t)asn1[2U + i];
    }
  }
  else /* If bit7 is not set, then it is the size */
  {
    count = 0;
    *size = (int32_t)asn1[1];
  }
  /* We have written the output size */
  /* We effectivly write as output both the pointer to value (which follows the size) and the number of bytes we've scanned */
  *parsed = 2 + (int32_t)count;
  *value = asn1 + *parsed;
  /* And we return the tag */
  return(tag);
}


/**
* @brief  Parse an ASN.1 INTEGER
* @param[in]  *integer pointer to the expected INTEGER TLV
* @param[out] *outp will point to the starting byte of the integer
* @param[out] *outSize will contain the size of the integer in bytes
* @param[out] **next_thing  will contain a pointer to the next TLV
* @note This doesn't copy the integer. To keep memory small this just returns a pointer to it.
* @warning only non-negative integers are supported
*/
void parseInteger(const uint8_t *integer, const uint8_t **outp, int32_t *outSize, const uint8_t **next_thing)
{
  int32_t size=0, tag, parsed=0;
  const uint8_t *next;

  tag = identifyASN1TLV(integer, &parsed, &size, &next);
  if (tag == TAG_INTEGER)
  {
    int32_t i = 0;
    /* The first byte tells us if it's positive or negative. We don't support negative */
    if (next[0] >> 7 == 1U)
    {
      return;
    }
    /* First byte might be zero in case Integer is positive and first byte >127 */
    /* So skip it */
    if (next[0] == 0U)
    {
      i++;
    }
    /* Now we have our integer */
    *outp = &next[i];
    *outSize = size - i;
  }
  *next_thing = integer + parsed + size;
}

/**
* @brief  Parse an ECDSA signature
* @param[in]  *signature pointer to the expected signature field of the x509 certificate
* @param[out] *intCert pointer to the intCert_stt that will be filled
* @param[out] **next_thing  will contain a pointer to the next TLV
*/
void parseECDSAsignature(const uint8_t *signature, intCert_stt *intCert, const uint8_t **next_thing)
{    
  int32_t wholesize=0, size=0, tag, parsed;
  const uint8_t *next;
  tag = identifyASN1TLV(signature, &parsed, &wholesize, &next);
  /* We expect a BITSTRING with the first byte (indicating the number of bits to exclude form the LSB) to be zero */
  /* If it's not, we return */
  if ((tag == TAG_BITSTRING) && (next[0] == 0x00U))
  {
    tag = identifyASN1TLV((next + 1), &parsed, &size, &next);
    if (tag == TAG_SEQUENCE)
    {
      parseInteger(next, &intCert->Sign.pR, &intCert->Sign.rSize, &next);
      parseInteger(next, &intCert->Sign.pS, &intCert->Sign.sSize, &next);
    }
  }  
  *next_thing = signature + wholesize + parsed;
}

/**
* @brief  Reads the value of an INTEGER and returns it as a int32_t 
* @param[in]  *value pointer to the value field of an INTEGER TLV (got from \ref identifyASN1TLV)
* @param[in]  size size of the integer (got from \ref identifyASN1TLV)
* @return the integer value as an int32_t 
* @warning only non-negative integers are supported
*/
int32_t getSmallInteger(const uint8_t *value, int32_t size)
{
  uint32_t retval = 0;
  int32_t i = 0;
  /*Check that it is small enough and that it is positive*/
  if (value[0] == 0U)
  {
    i++;
  }
  if ((((size - i) > 4) || (value[0] >> 7)) == 1U)
  {
    return(-1);
  }

  for (; i < size; i++)
  {
    retval <<= 8;
    retval += value[i];
  }
  if (retval < UINT32_MAX)
  {
    return((int32_t)retval);
  }
  else
  {
    return(-1);
  }
}

/**
* @brief  Parse a SignatureAlgorithm (or signature of a tbsCertificate)
* @param[in]  *SA pointer to the SignatureAlgorithm (or signature of a tbsCertificate)
* @param[out] *singatureAlgorithm integer that will be filled with the internal encoding of signatureAlgorithm
* @param[out] **next_thing output pointer to next TLV
* @note  Only a very limited set of SignatureAlgorithm is supported
*/
void parseSignatureAlgorithm(const uint8_t *SA, int32_t *singatureAlgorithm, const uint8_t **next_thing)
{
#define N_OF_IDENTIFIABLE_SIGNATURE_ALGORITHMS 5
  const struct SignatureAlgorithmOID_st signatureAlgorithms_oids[N_OF_IDENTIFIABLE_SIGNATURE_ALGORITHMS] = {
    { .len = 8, .type = SIG_ECDSA_SHA256, .oid = {0x2A, 0x86, 0x48, 0xCE, 0x3D, 0x04, 0x03, 0x02 } },
    { .len = 8, .type = SIG_ECDSA_SHA384, .oid = {0x2A, 0x86, 0x48, 0xCE, 0x3D, 0x04, 0x03, 0x03 } },
    { .len = 8, .type = SIG_ECDSA_SHA512, .oid = {0x2A, 0x86, 0x48, 0xCE, 0x3D, 0x04, 0x03, 0x04 } },
    { .len = 7, .type = SIG_ECDSA_SHA1  , .oid = {0x2A, 0x86, 0x48, 0xCE, 0x3D, 0x04, 0x01, 0x00 } },
    { .len = 8, .type = SIG_ECDSA_SHA224, .oid = {0x2A, 0x86, 0x48, 0xCE, 0x3D, 0x04, 0x03, 0x01 } }
  };
  int32_t i, total_size=0, size=0, parsed, tag;
  const uint8_t *next;

  tag = identifyASN1TLV(SA, &parsed, &total_size, &next);
  total_size += parsed;
  if (tag == TAG_SEQUENCE)
  {
    tag = identifyASN1TLV(next, &parsed, &size, &next);
    if (tag == TAG_OBJECT_IDENTIFIER)
    {
      for (i = 0; i < N_OF_IDENTIFIABLE_SIGNATURE_ALGORITHMS; i++)
      {
        if (size == signatureAlgorithms_oids[i].len)
        {
          if (memcmp(signatureAlgorithms_oids[i].oid, next, (uint32_t)size) == 0)
          {
            *singatureAlgorithm = signatureAlgorithms_oids[i].type;
            break;
          }
        }
      }
    }
  }  
  *next_thing = SA+total_size;
#undef N_OF_IDENTIFIABLE_SIGNATURE_ALGORITHMS
}

/**
* @brief  Parse the x509 Version of a certificate
* @param[in]  *x509VersionField pointer to the version field of a certificate
* @param[out] *intCert pointer to the intCert_stt that will be filled
* @param[out] **next_thing output pointer to next TLV
*/
void parseX509version(const uint8_t *x509VersionField, intCert_stt *intCert, const uint8_t **next_thing)
{
  int32_t size=0, tag, parsed;
  const uint8_t *next;

  tag = identifyASN1TLV(x509VersionField, &parsed, &size, &next);
  if (tag == TAG_INTEGER)
  {
    intCert->x509Version = getSmallInteger(next, size);
  }
  *next_thing = x509VersionField + parsed + size;
  
}

/**
* @brief  Parse an ECC public Key from a certificate
* @param[in]  *EccPK pointer to the ECC public Key
* @param[out] *intCert pointer to the intCert_stt that will be filled
* @param[out] **next_thing output pointer to next TLV
*/
void parseECCPublicKey(const uint8_t *EccPK, intCert_stt *intCert, const uint8_t **next_thing)
{
  int32_t i, total_size=0, size=0, parsed, tag;
  const uint8_t *next;
#define N_OF_IDENTIFIABLE_ECS 9
  const struct EllipticCurveOID_st ellipticCurves_oids[N_OF_IDENTIFIABLE_ECS] = {
    { .len = 8, .type = EC_P256   , .oid = { 0x2A, 0x86, 0x48, 0xCE, 0x3D, 0x03, 0x01, 0x07, 0    } },
    { .len = 5, .type = EC_P384   , .oid = { 0x2B, 0x81, 0x04, 0x00, 34  , 0   , 0   , 0   , 0    } },
    { .len = 5, .type = EC_P521   , .oid = { 0x2B, 0x81, 0x04, 0x00, 35  , 0   , 0   , 0   , 0    } },
    { .len = 9, .type = EC_bp256r1, .oid = { 0x2B, 0x24, 0x03, 0x03, 0x02, 0x08, 0x01, 0x01, 0x07 } },
    { .len = 9, .type = EC_bp256t1, .oid = { 0x2B, 0x24, 0x03, 0x03, 0x02, 0x08, 0x01, 0x01, 0x08 } },
    { .len = 9, .type = EC_bp384r1, .oid = { 0x2B, 0x24, 0x03, 0x03, 0x02, 0x08, 0x01, 0x01, 11   } },
    { .len = 9, .type = EC_bp384t1, .oid = { 0x2B, 0x24, 0x03, 0x03, 0x02, 0x08, 0x01, 0x01, 12   } },
    { .len = 9, .type = EC_bp512r1, .oid = { 0x2B, 0x24, 0x03, 0x03, 0x02, 0x08, 0x01, 0x01, 13   } },
    { .len = 9, .type = EC_bp512t1, .oid = { 0x2B, 0x24, 0x03, 0x03, 0x02, 0x08, 0x01, 0x01, 14   } },
  };

  tag = identifyASN1TLV(EccPK, &parsed, &total_size, &next);
  total_size += parsed;
  if (tag == TAG_SEQUENCE)
  {
    tag = identifyASN1TLV(next, &parsed, &size, &next);
    if (tag == TAG_SEQUENCE)
    {
      tag = identifyASN1TLV(next, &parsed, &size, &next);
      if (tag == TAG_OBJECT_IDENTIFIER)
      {
        if ((memcmp(next, "\x2A\x86\x48\xCE\x3D\x02\x01", 7) == 0) && (size == 7))
        {
          next += 7;
          tag = identifyASN1TLV(next, &parsed, &size, &next);
          if (tag == TAG_OBJECT_IDENTIFIER)
          {
            for (i = 0; i < N_OF_IDENTIFIABLE_ECS; i++)
            {
              if (size == ellipticCurves_oids[i].len)
              {
                if (memcmp(ellipticCurves_oids[i].oid, next, (uint32_t)size) == 0)
                {
                  intCert->EllipticCurve = ellipticCurves_oids[i].type;
                  break;
                }
              }
            }
            next += size;
            tag = identifyASN1TLV(next, &parsed, &size, &next);
            if (tag == TAG_BITSTRING)
            {
              if ((next[0] == 0x00U) && (next[1] == 0x04U))
              {
                intCert->PubKey.fsize = (size - 2) / 2;
                intCert->PubKey.pX = &next[2];
                intCert->PubKey.pY = &next[2 + intCert->PubKey.fsize];
              }
            }
          }
        }
      }
    }
  }
  *next_thing = EccPK + total_size;  
#undef N_OF_IDENTIFIABLE_ECS
}

/**
* @brief  Identify the Name Attribute from its OID
* @param[in]  *oid The value of the OID of the Attribute (got from \ref identifyASN1TLV )
* @param[in]  size size (in bytes) of the value field of this OID (got from \ref identifyASN1TLV )
* @note  Only a very limited set of Attributes is supported
* @return    The internal type identifing the Attribute or -1 for failure
*/
int32_t identifyAttribute(const uint8_t *oid, int32_t size)
{
#define N_OF_IDENTIFIABLE_ATTRIBUTES 9
  const struct AttributeOID_st attributes_oids[N_OF_IDENTIFIABLE_ATTRIBUTES] = {
    { .len = 3, .type = ATTR_CN  , .oid = { 0x55, 0x04, ATTR_CN   } },
    { .len = 3, .type = ATTR_C   , .oid = { 0x55, 0x04, ATTR_C    } },
    { .len = 3, .type = ATTR_SN  , .oid = { 0x55, 0x04, ATTR_SN   } },
    { .len = 3, .type = ATTR_DN  , .oid = { 0x55, 0x04, ATTR_DN   } },
    { .len = 3, .type = ATTR_ON  , .oid = { 0x55, 0x04, ATTR_ON   } },
    { .len = 3, .type = ATTR_OUN , .oid = { 0x55, 0x04, ATTR_OUN  } },
    { .len = 3, .type = ATTR_SOPN, .oid = { 0x55, 0x04, ATTR_SOPN } },
    { .len = 3, .type = ATTR_LN  , .oid = { 0x55, 0x04, ATTR_LN   } },
    { .len = 3, .type = ATTR_UID , .oid = { 0x55, 0x04, ATTR_UID  } },
  };
  int32_t i;
  for (i = 0; i < N_OF_IDENTIFIABLE_ATTRIBUTES; i++)
  {
    if (size == attributes_oids[i].len)
    {
      if (memcmp(attributes_oids[i].oid, oid, (uint32_t)size) == 0)
      {
        return(attributes_oids[i].type);
      }
    }
  }
  return(-1);
#undef N_OF_IDENTIFIABLE_ATTRIBUTES
}



/**
* @brief  Parse an RelativeDistinguishedName 
* @param[in]  *p pointer to the RelativeDistinguishedName (expeting a SET)
* @param[out] **nextRDN output pointer to next RDN
* @param[out] **attribute output pointer to RDN first attribute
*/
void parseRDN (const uint8_t *p, const uint8_t **nextRDN, const uint8_t **attribute)
{
  int32_t size=0, parsed;
  (void)identifyASN1TLV(p, &parsed, &size, attribute);
  *nextRDN = *attribute + size;
}


/**
* @brief  Identify the Extension from its OID
* @param[in]  *oid The value of the OID of the Extension (got from \ref identifyASN1TLV )
* @param[in]  size size (in bytes) of the value field of this OID (got from \ref identifyASN1TLV )
* @note  Only a very limited set of Extension is supported
* @return    The internal type identifing the Extension or -1 for failure
*/
int32_t identifyExtension(const uint8_t *oid, int32_t size)
{
#define N_OF_IDENTIFIABLE_EXTENSIONS 3
  const struct ExtensionOID_st extensions_oids[N_OF_IDENTIFIABLE_EXTENSIONS] = {
    { .len = 3, .type = EXTENSION_BC , .oid = { 0x55, 0x1D, 0x13, 0, 0 } },
    { .len = 3, .type = EXTENSION_KU , .oid = { 0x55, 0x1D, 0x0F, 0, 0 } },
    { .len = 3, .type = EXTENSION_EKU, .oid = { 0x55, 0x1D, 0x25, 0, 0 } },
  };
  int32_t i;
  for (i = 0; i < N_OF_IDENTIFIABLE_EXTENSIONS; i++)
  {
    if (size == extensions_oids[i].len)
    {
      if (memcmp(extensions_oids[i].oid, oid, (uint32_t)size) == 0)
      {
        return(extensions_oids[i].type);
      }
    }
  }
  return(-1);
#undef N_OF_IDENTIFIABLE_EXTENSIONS
}


/**
* @brief  String Comparison (case insensitive and for utf8)
* @param[in]  *p1 first string to compare
* @param[in]  *p2 second string to compare
* @return  result of comparison
* @retval -1 strings are different
* @retval 0 strings match
*/
int32_t caseInsensitiveCmp(const uint8_t *p1, const uint8_t *p2, int32_t size)
{
  int32_t i; /* For loop*/
  uint8_t d; /* byte difference */

  /* Try a raw comparison, if it's ok then we are ok */
  if (memcmp(p1, p2, (uint32_t)size) == 0)
  {
    return 0;
  }
  /* Otherwise try to be case insensitive */
  for (i = 0; i < size; i++)
  {
    d = p1[i] ^ p2[i];
    if (d != 0U)
    {
      if (d == 32U || ((p1[i] >= (uint8_t)'a' && p1[i] <= (uint8_t)'z') ||
                       (p1[i] >= (uint8_t)'A' && p1[i] <= (uint8_t)'Z')))
      {
        continue;
      }
      else
      {
        return -1;
      }
    }      
  }
  return(0);
}

/**
* @brief  Parse a Name Attribute
* @param[in]  *p pointer to the AttributeTypeAndValue SEQUENCE to be parsed
* @param[out] *attribute_st pointer to attribute_stt structure that will be filled
* @param[out] **next_thing output pointer to next TLV
*/
void parseAttribute(const uint8_t *p, attribute_stt *attribute_st, const uint8_t **next_thing)
{
  int32_t total_size=0, size=0, parsed, tag;
  const uint8_t *next = p;
  tag = identifyASN1TLV(next, &parsed, &total_size, &next);
  total_size += parsed;
  if (tag == TAG_SEQUENCE)
  {
    tag = identifyASN1TLV(next, &parsed, &size, &next);
    if (tag == TAG_OBJECT_IDENTIFIER)
    {
      attribute_st->type = identifyAttribute(next, size);
      if (attribute_st->type != -1)
      {
        next += size;
        tag = identifyASN1TLV(next, &parsed, &size, &next);
        attribute_st->strFormat = tag;
        attribute_st->str = next;
        attribute_st->strSize = size;
      }
    }
  }
  *next_thing = p + total_size;
}

/**
* @brief  Count the number of Attributes within a NAME
* @param[in]  *p pointer to the RDNSequence to be parsed
* @returun The number of Attributes
*/
int32_t countAttributes(const uint8_t *p)
{
  int32_t size=0, parsed, tag,total_size=0, count=0;
  const uint8_t *next=p;
  tag = identifyASN1TLV(next, &parsed, &total_size, &next);
  total_size += parsed;
  if (tag == TAG_SEQUENCE)
  {
    /* Scans all RDNs */
    while (next < p + total_size)
    {
      const uint8_t *EoSet = next;
      tag = identifyASN1TLV(next, &parsed, &size, &next);
      EoSet += parsed + size;
      if (tag == TAG_SET)
      {
        while (next < EoSet)
        {
          tag = identifyASN1TLV(next, &parsed, &size, &next);
          if (tag == TAG_SEQUENCE)
          {
            next += size;
            count++;
          }
        }
      }
    }
  }
  return (count);
}

/**
* @brief  Parse the validity of a certificate
* @param[in]  *p pointer to the Name Attribute SET to be parsed
* @param[out] *notBefore_st pointer to validity_stt structure that will be filled with the "not before" date
* @param[out] *notAfter_st  pointer to validity_stt structure that will be filled with the "not after" date
* @param[out] **next_thing output pointer to next TLV
* @note In this function next_thing can be NULL
*/
void parseValidity(const uint8_t *p, validity_stt *notBefore_st, validity_stt *notAfter_st, const uint8_t **next_thing)
{
  int32_t i, total_size=0, size=0, parsed, tag;
  const uint8_t *next = p;
  validity_stt *pValidity_st;

  tag = identifyASN1TLV(next, &parsed, &total_size, &next);
  total_size += parsed;
  if (tag == TAG_SEQUENCE)
  {
    for (i = 0; i < 2; i++)
    {    
      if (i == 0)
      {
        pValidity_st = notBefore_st;
      }
      else
      {
        pValidity_st = notAfter_st;
      }
      tag = identifyASN1TLV(next, &parsed, &size, &next);
      if (tag == TAG_UTCTime || tag == TAG_GeneralizedTime)
      {
        uint32_t timevalue;
        if (tag == TAG_UTCTime)
        {
          timevalue = ((uint32_t)next[0] - 0x30U) * 10U + ((uint32_t)next[1] - 0x30U);
          next += 2; 
          size -= 2;
          if (timevalue > 50U)
          {
            pValidity_st->year = timevalue + 1900U;
          }
          else
          {
            pValidity_st->year = timevalue + 2000U;
          }
        }
        else
        {
          pValidity_st->year = ((uint32_t)next[0] - 0x30U) * 1000U + ((uint32_t)next[1] - 0x30U) * 100U +
                               ((uint32_t)next[2] - 0x30U) * 10U + ((uint32_t)next[3] - 0x30U);
          next += 4;
          size -= 4;
        }
        pValidity_st->month = ((uint8_t)next[0] - 0x30U) * 10U + (uint8_t)next[1] - 0x30U;
        pValidity_st->days = ((uint8_t)next[2] - 0x30U) * 10U + (uint8_t)next[3] - 0x30U;
        pValidity_st->hours = ((uint8_t)next[4] - 0x30U) * 10U + (uint8_t)next[5] - 0x30U;
        pValidity_st->minutes = ((uint8_t)next[6] - 0x30U) * 10U + (uint8_t)next[7] - 0x30U;
        pValidity_st->seconds = ((uint8_t)next[8] - 0x30U) * 10U + (uint8_t)next[9] - 0x30U;
        next += size;
      }
    }
  }
  if (next_thing != NULL)
  {
    *next_thing = p + total_size;
  }   
}


/**
* @brief  Compare two validity_stt structures
* @param[in]  *D1 pointer to the first validity_stt
* @param[in]  *D2 pointer to the second validity_stt
* @return Result of Comparison
* @retval   -1  D1 < D2
* @retval    0  D1 = D2
* @retval    1  D1 > D2
*/
int32_t dateCompare(const validity_stt *D1, const validity_stt *D2)
{
  int32_t test = 0;
  /* First check if year is the same */
  if (D1->year != D2->year)
  {
    /* If it's not then we have our answer as the difference of the two */
    test = (int32_t)D1->year - (int32_t)D2->year;
  }
  /* If years are equal check the months field */
  if ((test == 0) && (D1->month != D2->month))
  {
    test = (int32_t)D1->month - (int32_t)D2->month;
  }
  /* And so on... */
  if ((test == 0) && (D1->days != D2->days))
  {
    test = (int32_t)D1->days - (int32_t)D2->days;
  }
  if ((test == 0) && (D1->hours != D2->hours))
  {
    test = (int32_t)D1->hours - (int32_t)D2->hours;
  }
  if ((test == 0) && (D1->minutes != D2->minutes))
  {
    test = (int32_t)D1->minutes - (int32_t)D2->minutes;
  }
  if ((test == 0) && (D1->minutes != D2->minutes))
  {
    test = (int32_t)D1->minutes - (int32_t)D2->minutes;
  }
  if ((test == 0) && (D1->seconds != D2->seconds))
  {
    test = (int32_t)D1->seconds - (int32_t)D2->seconds;
  }
  /* Now we have three cases */
  /* test < 0 if D1 < D2 */
  /* test = 0 if D1 = D2 */
  /* test > 0 if D1 > D2 */
  if (test < 0)
  {
    return (-1);
  }
  else
  {
    if (test > 0)
    {
      return (1);
    }
  }
  return (0);
}

/**
* @brief  Parse an x509 Extension
* @param[in]  *ext pointer to the extension
* @param[out]  extFlags pointer to a field extensionsFlags of an intCert_stt structure
* @param[out]  *ext_st pointer to an extension_stt structure
*/
void parseExtension(const uint8_t *ext, uint32_t *extFlags, extension_stt *ext_st)
{
  int32_t i, size = 0, parsed, tag, seqSize;
  const uint8_t *next = ext;

  /* we expect the OID */
  tag = identifyASN1TLV(next, &parsed, &size, &next);
  if (tag == TAG_OBJECT_IDENTIFIER)
  {
    ext_st->type = identifyExtension(next, size);
    next += size;

    switch (ext_st->type)
    {
      case EXTENSION_BC:
        ext_st->critical = 0;
        *extFlags |= 1U; /* BC is present */
        tag = identifyASN1TLV(next, &parsed, &size, &next);
        /* Either we get the boolean or the octect string */
        if (tag == TAG_BOOLEAN)
        {
          if (size == 1 && next[0] != 0U)
          {
            ext_st->critical = 1;
            *extFlags |= 1U << 1; /* BC is critical */
          }
          next += size;
          tag = identifyASN1TLV(next, &parsed, &size, &next);
        }
        if (tag == TAG_OCTETSTRING)
        {
          ext_st->value = next;
          ext_st->valueSize = size;

          tag = identifyASN1TLV(next, &parsed, &seqSize, &next);
          /* It should start with a sequence */
          if (tag == TAG_SEQUENCE)
          {
            /* Not CA and No Path */
            if (seqSize > 0) /* There is a CA or PATH */
            {
              tag = identifyASN1TLV(next, &parsed, &size, &next);
              /* Boolean and Optional pathLen (integer) */
              if (((tag == TAG_BOOLEAN) && next[0]) != 0U)
              {
                *extFlags |= 1U << 2; /* BC says it's CA */
              }
              /* Is there more ? */
              if (next < (ext_st->value + ext_st->valueSize))
              { /* Yes, then it's the path */
                tag = identifyASN1TLV(next, &parsed, &size, &next);
                if (tag == TAG_INTEGER)
                {
                  *extFlags |= 1U << 3; /* Path is present */
                  *extFlags |= ((uint32_t)getSmallInteger(next, size) & 15U) << 4; /* 4-bit encoding for pathLen */
                }
              }
            }
          }
        }
        break;
      case EXTENSION_KU:
        ext_st->critical = 0;
        *extFlags |= 1U << 8; /* KU is present */
        tag = identifyASN1TLV(next, &parsed, &size, &next);
        /* Either we get the boolean or the octect string */
        if (tag == TAG_BOOLEAN)
        {
          if (((size == 1) && next[0]) != 0U)
          {
            ext_st->critical = 1;
            *extFlags |= 1U << 9; /* KU is critical */
          }
          next += size;
          tag = identifyASN1TLV(next, &parsed, &size, &next);
        }
        if (tag == TAG_OCTETSTRING)
        {
          ext_st->value = next;
          ext_st->valueSize = size;

          tag = identifyASN1TLV(next, &parsed, &seqSize, &next);
          /* It should start with a sequence */
          if (tag == TAG_BITSTRING)
          {
#define MAX(a,b) (((a)>(b))?(a):(b))
            /* This reads the 8 bits (or less) and put it in the right place */
            for (i = 0; i < MAX((int32_t)next[0],8); i++)
            {
              /* Not very readable */
              *extFlags |= (((uint32_t)next[1] >> (uint8_t)(7U - (uint8_t)i)) & 1U) << (uint8_t)(16U + (uint8_t)i);
            }
            if ((next[0] == 9U) && (seqSize == 2))
            {
              *extFlags |= (((uint32_t)next[2] >> 7) & 1U) << (15);
            }
#undef MAX
          }
        }
        break;
      default:
        break;
    }
  }  
}

