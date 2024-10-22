/**
  ******************************************************************************
  * @file    x509.c
  * @author  AST Security
  * @version V0.2
  * @date    24-February-2017
  * @brief   x509 certificate Parser
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


#include <stdint.h>
#include <string.h>
#include "x509.h"
#include "x509_prints.h"
#include "x509_subparsing.h"
#include "x509_crypto.h"

/* Parse tbsCertificate */
static void parsetbsCertificate(const uint8_t *tbs, int32_t tbsSize, intCert_stt *intCert);

/**
* @brief  Initialize a certificate (\ref intCert_stt) to an empty one
* @param[in,out]  *cert pointer to the intCert_stt to be initialized
* @note  This functions should be called before parsing a certificate
*/
void initIntCert(intCert_stt *cert)
{
  cert->tbs = NULL;
  cert->tbsSize = -1;
  cert->x509Version = -1;
  cert->serialNumber = NULL;
  cert->serialNumberSize = -1;
  cert->signature = -1;
  cert->issuer = NULL;
  cert->issuerSize = -1;
  cert->validity = NULL;
  cert->validitySize = -1;
  cert->subject = NULL;
  cert->subjectSize = -1;
  cert->EllipticCurve = -1;
  cert->PubKey.fsize = 0;
  cert->PubKey.pX = NULL;
  cert->PubKey.pY = NULL;
  cert->extensions = NULL;
  cert->extensionsSize = -1;

  cert->SignatureAlgorithm = -1;

  cert->Sign.pR = NULL;
  cert->Sign.pS = NULL;
  cert->Sign.rSize = 0;
  cert->Sign.sSize = 0;

  cert->extensionsFlags = 0;
}

/**
* @brief  Copy a certificate (\ref intCert_stt) into another structure
* @param[out]  *copiedCert pointer to the intCert_stt that will be written
* @param[out]  *originalCert pointer to the intCert_stt that will be copied
*/
void copyCert(intCert_stt *copiedCert, const intCert_stt *originalCert)
{
  *copiedCert = *originalCert;
}

/**
* @brief  Parse the extensions part of a certificate
* @param[in]  *extensions pointer to the extensions part of a certificate
* @param[in]  extensionsSize Size of the extensions field
* @param[out] *intCert pointer to the intCert_stt that will be filled
*/
static void parseExtensions(const uint8_t *extensions, int32_t extensionsSize, intCert_stt *intCert)
{
  const uint8_t *next_value;
  int32_t size, tag, parsed;
  extension_stt ext_st = {.type = -1, .value = NULL, .valueSize = 0};
  /* Start parsing the extensions, extensions is a sequence */
  tag = identifyASN1TLV(extensions, &parsed, &size, &next_value); 
  if (tag == TAG_SEQUENCE)
  {
    /* Now we loop over sequences */
    while (next_value < (extensions + extensionsSize))
    {
      tag = identifyASN1TLV(next_value, &parsed, &size, &next_value);
      if (tag == TAG_SEQUENCE)
      {      
        parseExtension(next_value, &intCert->extensionsFlags, &ext_st);      
      }
      next_value += size;
    }    
  }
}

/**
* @brief  Parse the tbs part of a certificate
* @param[in]  *tbs pointer to the TBSCertificate field of a certificate 
* @param[in]  *tbsSize size of the TBSCertificate
* @param[out] *intCert pointer to the intCert_stt that will be filled
*/
static void parsetbsCertificate(const uint8_t *tbs, int32_t tbsSize, intCert_stt *intCert)
{
  const uint8_t *next_value;
  int32_t size, tag, parsed;
  /* Start parsing the TBS, tbs is a sequence */
  tag = identifyASN1TLV(tbs, &parsed, &size, &next_value);
  if (tag == TAG_SEQUENCE)
  {
    /* Now we expect the version */
    tag = identifyASN1TLV(next_value, &parsed, &size, &next_value);
    if (tag == TAG_x509VERSION)
    {
      parseX509version(next_value, intCert, &next_value);
    }
    else /* The Version is optional, if it's not present then 0 (v1) is assumed */
    {
      intCert->x509Version = 0;
      next_value -= parsed;
    }
    /* Move on to Serial Number */
    parseInteger(next_value, &intCert->serialNumber, &intCert->serialNumberSize, &next_value);
    /* Parse tbsSignature(Algorihtm)*/
    parseSignatureAlgorithm(next_value, &intCert->signature, &next_value);
    /* Now we face the issuer */
    tag = identifyASN1TLV(next_value, &parsed, &size, &next_value);
    if (tag == TAG_SEQUENCE)
    {
      intCert->issuer = next_value - parsed;
      intCert->issuerSize = size + parsed;
    }    
    next_value += size; /* Issuer is not parsed */
    /* Now we face validity */
    tag = identifyASN1TLV(next_value, &parsed, &size, &next_value);
    if (tag == TAG_SEQUENCE)
    {
      intCert->validity = next_value - parsed;
      intCert->validitySize = size + parsed;
    }
    next_value += size; /* Validity is not parsed */
    /* Now we face Subject */
    tag = identifyASN1TLV(next_value, &parsed, &size, &next_value);
    if (tag == TAG_SEQUENCE)
    {
      intCert->subject = next_value - parsed;
      intCert->subjectSize = size + parsed;
    }
    next_value += size; /* Subject is not parsed */
    /* Now we face SubjectPublicKey */
    parseECCPublicKey(next_value, intCert, &next_value);
    /* Now we face Optional Extensions, but those are optional */
    if (next_value < (tbs + tbsSize))
    {
      tag = identifyASN1TLV(next_value, &parsed, &size, &next_value);
      if (tag == TAG_extensions)
      {
        intCert->extensions = next_value;
        intCert->extensionsSize = size;
        parseExtensions(intCert->extensions, intCert->extensionsSize, intCert);
      }
    }    
  }
}

/**
* @brief  Parse an x509 certificate
* @param[in]  *cert pointer to the x509 certificate to be parsed 
* @param[in]  certSize size (in bytes) of the certificate
* @param[out] *intCert pointer to the intCert_stt that will be filled
* @param[out] **next pointer to cert array after the parsed certificate (it can be NULL)
* @return 0 if success, negative numbers for errors
* @retval 0 Success (This is \b not an indicator of the certificate validity)
* @retval ERR_EXPECTED_SEQUENCE A sequence was expected
* @warning The parsed certificated could be empty, it is necessary to call a function
*       which verifies the certificate to be valid before using it.
*/
int32_t parseCert(const uint8_t *cert, intCert_stt *intCert, const uint8_t **next)
{
  int32_t total_size, tag, size, parsed;
  const uint8_t *next_thing;

  initIntCert(intCert);
  /* First we start by looking at the first item, this will be used to set the size of the whole certificate */
  /* Then the parsing of the other subfields will start */
  tag = identifyASN1TLV(cert, &parsed, &total_size, &next_thing);
  if (tag != TAG_SEQUENCE)
  {     
    return(ERR_EXPECTED_SEQUENCE);
  }
//  total_size = total_size + parsed;
  /* Get tbs */
  tag = identifyASN1TLV(next_thing, &parsed, &size, &next_thing);
  if (tag != TAG_SEQUENCE)
  {
    return(ERR_EXPECTED_SEQUENCE);
  }
  intCert->tbs = next_thing-parsed;
  intCert->tbsSize = size+parsed;
  
  /* Get SignatureAlgorithm */
  next_thing += size;
  parseSignatureAlgorithm(next_thing, &intCert->SignatureAlgorithm, &next_thing);

  /* Now we should have the Signature. If it's ECDSA parse it! */
  if (intCert->SignatureAlgorithm != -1)
  {
    parseECDSAsignature(next_thing, intCert, &next_thing);
  }

  /* We have completed the parsing of the TOP level */
  /* Let's move on to parse the tbs */
  parsetbsCertificate(intCert->tbs, intCert->tbsSize, intCert);

  if (next != NULL)
  {
    *next = next_thing;
  }  

  return(0);
}

/**
* @brief  Check that an imported x509 certificate is valid
* @param[in]  *intCert pointer to the parsed x509 certificate to be validated
* @param[in]  *currentTime pointer to a validity_stt with the current DateTime. If NULL no date check is done.
* @return Validity of certificate
* @retval 1 Certificate is Valid
* @retval -1 Certificate is Not Valid
* @warning If currentTime==NULL the check on the validity dates of the certificate will be bypassed.
*/
int32_t isValidCert(const intCert_stt *intCert, const validity_stt *currentTime)
{
  if (
    ((intCert->issuer == NULL) || (intCert->issuerSize == -1)) ||
    ((intCert->subject == NULL) || (intCert->subjectSize == -1)) ||
    ((intCert->serialNumber == NULL) || (intCert->serialNumberSize == -1)) ||
    ((intCert->signature == -1) || (intCert->SignatureAlgorithm != intCert->signature)) ||
    ((intCert->validity == NULL) || (intCert->validitySize == -1)) ||
    ((intCert->EllipticCurve == -1) || (intCert->x509Version == -1))||
    ((intCert->PubKey.fsize <= 0) || (intCert->PubKey.pX == NULL) || (intCert->PubKey.pY == NULL)) ||
    ((intCert->Sign.pR == NULL) || (intCert->Sign.rSize == -1)) ||
    ((intCert->Sign.pS == NULL) || (intCert->Sign.sSize == -1))
    )
  {
    return(-1);
  }

  if (currentTime != NULL)
  {
    validity_stt notBefore_st, notAfter_st;    
    parseValidity(intCert->validity, &notBefore_st, &notAfter_st, NULL);

    if (dateCompare(currentTime, &notBefore_st) < 0)
    {
      return(-1);
    }

    if (dateCompare(currentTime, &notAfter_st) > 0)
    {
      return(-1);
    }
  }

  return(1);
}


/**
* @brief  Check whether a certificate is marked as belonging to a CA
* @param[in]  *cert pointer to the parsed x509 certificate to be checked
* @return CA Status
* @retval -1 certificate doesn't belong to a CA
* @retval 0 certificate belongs to a CA with no pathLenConstraint 
* @retval positive integer, certificate belongs to a CA  pathLenConstraint
*/
int32_t isCA(const intCert_stt *cert)
{
  if (((cert->extensionsFlags >> 0) & 1U) == 1U)
  {
    /* BasicContrain is present */
    if (((cert->extensionsFlags >> 2) & 1U) == 1U)
    {
      /* This is a CA */
      uint32_t tmp = (cert->extensionsFlags >> 4) & 15U;
      return (int32_t)tmp;
    }
  }
  return -1;
}

/**
* @brief  Check whether "parent" issued "child"
* @param[in]  *parent pointer to the parsed x509 certificate of the supposed issuer of child
* @param[in]  *child  pointer to the parsed x509 certificate of the certificated supposedly issued by parent
* @param[in]  *currentTime pointer to a validity_stt with the current DateTime. If NULL no date check is done.
* @return Validity of the parentship
* @retval 1 child was issued by parent
* @retval -1 child was \b not issued by parent
* @note RFC 5280 mandates an extremely expensive comparison, which we don't perform.
* As result of this simplification some legitimate parentship relation could be not acknowledged (the opposite should not occur). 
* @warning This check only a single level parent relashion (i.e. whether parent issued child)
*/
int32_t isParent(const intCert_stt *parent, const intCert_stt *child, const validity_stt *currentTime)
{
  /* Get the two name structure to be compared */
  const uint8_t *name1 = child->issuer;
  int32_t name1Size = child->issuerSize;
  const uint8_t *name2 = parent->subject;
  int32_t name2Size = parent->subjectSize;
  int32_t found, parsed;
  const uint8_t *attr2_loop;
  const uint8_t *attr1, *attr2;
  attribute_stt attr1_st, attr2_st;
  
  /* Do not compare two invalid certificates */
  if (isValidCert(parent, currentTime) == -1)
  {
    return(-1);
  }

  if (isValidCert(child, currentTime) == -1)
  {
    return(-1);
  }

  if ( isCA(parent) < 0)
  {
    return(-1);
  }


  /* Check that they have the same number of attributes */
  if (countAttributes(name1) != countAttributes(name2))
  {
    return (-1);
  }  

  /* Now we need to scan name1 and look for corresponding attribute in name2 */
  /* Start entering in the root sequence */
  if (identifyASN1TLV(name1, &parsed, &found, &name1) != TAG_SEQUENCE)
  {
    return -1;
  }

  if (identifyASN1TLV(name2, &parsed, &found, &name2) != TAG_SEQUENCE)
  {
    return -1;
  }

  /* Loop on all RDN */
  while ( (name1 < (child->issuer + name1Size)) && (name2<parent->subject+name2Size) )
  {
    /* Parse RDN1 and RDN2. The SEQUENCE is order so this should match */
    parseRDN(name1, &name1, &attr1);
    parseRDN(name2, &name2, &attr2);
    /* Now loop on the attributes inside the RDN, which are not ordered */
    while (attr1 < name1) /*Loop untill the next RDN1 */
    {
      /* A single DN is a SET which might (but usually doesn't) contain more Attributes */
      /* Since the SET is not ordered we need to select the attribute from the name1 and scan
         all the SET of name2 for a matching attribute */
      
      parseAttribute(attr1, &attr1_st, &attr1); /*Read the attribute of Name1 */

      attr2_loop = attr2; /* copy attr2 as attr2loop will loop for each attribute of RDN1 */      
      found = 0; /* Flag to tell us if we found or not the attribute in Name2 */
      do /* Scans the whole DN SET in Name2 */
      {
        /* Read Attribute */
        parseAttribute(attr2_loop, &attr2_st, &attr2_loop);
        /* Check if it match with the attribute of name 1 */
        if (attr1_st.type == attr2_st.type) 
        {
          found = 1; /* Yes, we found the matching attribute */
          
          /* Now we need to compare */
          /* In this if we need to either break the do/while or return -1 */
          /* We don't support IA5String so we support only BITSTRING, UTF8 and PrintableString */
                   
          /* Now consider the case where attr1 is a string (either Printable or UTF8) */
          if ((attr1_st.strFormat == TAG_PrintableString) || (attr1_st.strFormat == TAG_UTF8String))
          {
            /* Then the attr2 should be of string type with the same size */
            if (((attr2_st.strFormat != TAG_PrintableString) && (attr2_st.strFormat != TAG_UTF8String)) ||
              (attr1_st.strSize != attr2_st.strSize))
            {
              /* If not return -1 */
              return(-1);
            }
            /* Now they are both string type and of same size, they should match */
            if (caseInsensitiveCmp(attr1_st.str, attr2_st.str, attr1_st.strSize) != 0)
            {
              return(-1);
            }
            /* Here they are both string type, of same size, and match. So break the loop */
            break;
          }
          /* If we are here we didn't return -1 and we didn't break */
          /* Therefore the type of attr1 was not among the allowed ones (e.g. it might be IA5String) */
          /* Just try a very raw comparison.. it's the only thing we can do */
          /* Note: this perfectly cover the BITSTRING case */
          /* attributes should be of same type and size */
          if ((attr1_st.strFormat == attr2_st.strFormat) && (attr1_st.strSize == attr2_st.strSize))
          {           
            /* And they should match*/
            if (memcmp(attr1_st.str, attr2_st.str, (uint32_t)attr1_st.strSize) != 0)
            {
              return(-1);
            }
            /* Here they match, so we break the loop */
            break;
          }
          /* We did all we can. If they don't match.. return error */
          return(-1);
        }
      } while (attr2_loop < name2);
      /* Did we found the attribute ? If not return -1 */
      if (found != 1)
      {
        return(-1);
      }
    }
  }

  /* If we are here then certificates are ok. */
  /* But is the signature valid ? */
  return(verifyCertSignature(parent, child));
}

