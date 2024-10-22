/**
  ******************************************************************************
  * @file    x509.h
  * @author  AST Security
  * @version V0.2
  * @date    16-November-2016
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


#ifndef X509_H
#define X509_H

#include <stdint.h>

/** @brief Definitions of available ASN.1 TAGs */
#define TAG_BOOLEAN                     1	
#define TAG_INTEGER                     2	
#define TAG_BITSTRING                   3	
#define TAG_OCTETSTRING                 4	
#define TAG_NULL                        5	
#define TAG_OBJECT_IDENTIFIER           6	
#define TAG_ObjectDescriptor            7 
#define TAG_ENUMERATED                  10
#define TAG_UTF8String                  12
#define TAG_SEQUENCE                    0x30
#define TAG_SET                         0x31
#define TAG_PrintableString             19
#define TAG_IA5String                   22
#define TAG_UTCTime                     23
#define TAG_GeneralizedTime             24
#define TAG_x509VERSION                 0xA0
#define TAG_issuerUniqueID              0xA1
#define TAG_subjectUniqueID             0xA2
#define TAG_extensions                  0xA3

/** @brief Macro which checks if a TAG is valid (knwon) */
#define IS_VALID_TAG(tag) ((((tag) == TAG_BOOLEAN) || \
  ((tag) == TAG_INTEGER) || ((tag) == TAG_BITSTRING) || \
  ((tag) == TAG_OCTETSTRING) || ((tag) == TAG_UTF8String) || \
  ((tag) == TAG_OBJECT_IDENTIFIER) || ((tag) == TAG_SEQUENCE) || \
  ((tag) == TAG_PrintableString) || ((tag) == TAG_IA5String) || \
  ((tag) == TAG_GeneralizedTime) || ((tag) == TAG_GeneralizedTime) || \
  ((tag) == TAG_x509VERSION) || ((tag) == TAG_SET) || ((tag) == TAG_UTCTime) || \
  ((tag) == TAG_issuerUniqueID) || ((tag) == TAG_extensions) || \
  ((tag) == TAG_subjectUniqueID) || ((tag) == TAG_NULL)) ? 1 : 0)


/** @brief Definitions of available signature algorithms */
#define SIG_ECDSA_SHA1   0
#define SIG_ECDSA_SHA224 1
#define SIG_ECDSA_SHA256 2
#define SIG_ECDSA_SHA384 3
#define SIG_ECDSA_SHA512 4

/** @brief Structure used to keep SignatureAlgorithm OIDs */
struct SignatureAlgorithmOID_st
{
  int32_t len;    /*!< size of the encoded OID */
  int32_t type;   /*!< type of OID */
  uint8_t oid[8]; /*!< encoded OID */
};

/** @brief Definitions of supported Name Attributes */
#define ATTR_CN 3 /*!< CommonName */
#define ATTR_C 6  /*!< Country */
#define ATTR_SN 5 /*!< SerialNumber */ 
#define ATTR_DN 49 /*!< DistinguishedName */
#define ATTR_ON 10  /*!< Organization Name */
#define ATTR_OUN 11 /*!< Organization Unit Name */
#define ATTR_SOPN 8 /*!< stateOrProvinceName */
#define ATTR_LN 7 /*!< LocalityName */
#define ATTR_UID 45  /*!< UniqueIdentifier */

/** @brief Structure used to keep Name Attribute OIDs */
struct AttributeOID_st
{
  int32_t len;    /*!< size of the encoded OID */
  int32_t type;   /*!< type of OID */
  uint8_t oid[3]; /*!< encoded OID */
};

/** @brief Definitions of supported Elliptic Curve Names */
#define EC_P256   0    /*!< NIST P-256 */
#define EC_P384   1    /*!< NIST P-384 */
#define EC_P521   2    /*!< NIST P-521 */
#define EC_bp256r1  3  /*!< brainpoolP256r1 */
#define EC_bp256t1  4  /*!< brainpoolP256t1 */
#define EC_bp384r1  5  /*!< brainpoolP384r1 */
#define EC_bp384t1  6  /*!< brainpoolP384t1 */
#define EC_bp512r1  7  /*!< brainpoolP512r1 */
#define EC_bp512t1  8  /*!< brainpoolP512t1 */

/** @brief Structure used to keep named Elliptic Curve OIDs */
struct EllipticCurveOID_st
{
  int32_t len;    /*!< size of the encoded OID */
  int32_t type;   /*!< type of OID */
  uint8_t oid[9]; /*!< encoded OID */
};

/** @brief typedef for the attribute */
typedef struct attribute_st
{
  int32_t type;       /*!< type of attribute (corresponding to OID) */
  int32_t strFormat;  /*!< format of the string */
  const uint8_t *str; /*!< pointer to the string of the attribute */
  int32_t strSize;    /*!< size of the string */    
} attribute_stt;


/** @brief Definitions of supported Name Attributes */
#define EXTENSION_BC 19 /*!< BasicContrains */
#define EXTENSION_KU 15  /*!< KeyUsage */
#define EXTENSION_EKU 37 /*!< extKeyUsage */ 

/** @brief Structure used to keep Name Attribute OIDs */
struct ExtensionOID_st
{
  int32_t len;    /*!< size of the encoded OID */
  int32_t type;   /*!< type of OID */
  uint8_t oid[5]; /*!< encoded OID */
};

/** @brief typedef for the extension */
typedef struct extension_st
{
  int32_t type;       /*!< type of extension (corresponding to OID) */
  int32_t critical;   /*!< critical */
  const uint8_t *value; /*!< pointer to the value hold by the extension (this is an octet string)  */
  int32_t valueSize;    /*!< size of the string */
} extension_stt;


/** @brief typedef for the structure keeping the validity */
typedef struct validity_st {
   uint8_t seconds;     /*!< seconds,  range 0 to 59          */
   uint8_t minutes;     /*!< minutes, range 0 to 59           */
   uint8_t hours;       /*!< hours, range 0 to 23             */
   uint8_t days;        /*!< days, range 1 to 31              */
   uint8_t month;       /*!< month, range 1 to 12             */
   uint32_t year;       /*!< years 0 to whatever   */
} validity_stt;

/** @brief typedef for the structure keeping the Ellipitc Curve Public Key */
typedef struct EcPubKey_st
{
  const uint8_t *pX; /*!<  Pointer to X Coordinate */
  const uint8_t *pY; /*!<  Pointer to Y Coordiante */
  int32_t fsize; /*!< Size of the field in bytes */
} EcPubKey_stt;

/** @brief typedef for the structure keeping the ECDSA Signature */
typedef struct ECDSAsign_st
{
  const uint8_t *pR; /*!< Pointer to r */
  int32_t  rSize;    /*!< size to r */
  const uint8_t *pS; /*!<  Pointer to s*/
  int32_t  sSize;    /*!< size to s */
} ECDSAsign_stt; 

/** @brief internal structure used to keep the values parsed from the x509 */
struct internalCertificate_st {
  const uint8_t *tbs;               /*!<  Pointer to tcs field */
  int32_t tbsSize;            /*!<  Size of tcs */
  int32_t x509Version;        /*!<  x509 Version */
  const uint8_t *serialNumber;      /*!<  Pointer to SerialNumber*/
  int32_t serialNumberSize;   /*!<  Size of SerialNumber */
  int32_t signature;          /*!<  Signature (algorithm) but from tcs */
  const uint8_t *issuer;            /*!<  Pointer to Issuer */
  int32_t issuerSize;         /*!<  Size of Issuer */
  const uint8_t *validity;          /*!<  Pointer to Validity */
  int32_t validitySize;       /*!<  Size of Validity */
  const uint8_t *subject;           /*!<  Pointer to subject */
  int32_t subjectSize;        /*!<  Size of Subject */
  int32_t EllipticCurve;      /*!<  Identifies the Elliptic Curve */
  EcPubKey_stt PubKey;        /*!<  Contain the struct EcPubKey_st */
  const uint8_t *extensions;  /*!<  Pointer to Extensions */
  int32_t extensionsSize;
  uint32_t extensionsFlags;   /*!<  Integer encoding the fields present in the tcs:
                              bit 0: BasicConstraints is present
                              bit 1: BasicConstraints is critical
                              bit 2: BasicConstraints indicates this is a CA
                              bit 3: BasicConstraints has a pathLenConstraint       
                              bit 4-7: pathLen
                              bit 8: keyUsage is present
                              bit 9: keyUsage is critical
                              bit 15: keyUsage field decipherOnly
                              bit 16: keyUsage field digitalSignature                   
                              bit 17: keyUsage field contentCommitment/nonRepudiation          
                              bit 18: keyUsage field keyEncipherment         
                              bit 19: keyUsage field dataEncipherment        
                              bit 20: keyUsage field keyAgreement            
                              bit 21: keyUsage field keyCertSign             
                              bit 22: keyUsage field cRLSign                 
                              bit 23: keyUsage field encipherOnly            
                              bit 24: ExtendedKeyUsage is present
                              bit 25: ExtendedKeyUsage is critical
                              bit 27-29: unused

                              */
                              
  int32_t SignatureAlgorithm; /*!<  SignatureAlgorithm */  

  ECDSAsign_stt Sign;         /*!<  Contain the struct ECDSAsign_st */
   
};

/** @brief typedef for the intCert_stt */
typedef struct internalCertificate_st intCert_stt;

/** @brief Errors  */
#define ERR_EXPECTED_SEQUENCE -2
#define ERR_EXPECTED_BITSTRING -3
#define ERR_OBJECT_IDENTIFIER -4

/* Exported Functions */

/* Init a certificate structure to "zero" */
void initIntCert(intCert_stt *cert);

/* Parse a certificate */
int32_t parseCert(const uint8_t *cert, intCert_stt *intCert, const uint8_t **next);

/* Check certificate validity */
int32_t isValidCert(const intCert_stt *intCert, const validity_stt *currentTime);

/* Check if a certificate belongs to a CA (through basicContrains extension) */
int32_t isCA(const intCert_stt *cert);

/* Check if "parent" generated "child" and if both are valid certificates  */
int32_t isParent(const intCert_stt *parent, const intCert_stt *child, const validity_stt *currentTime);

void copyCert(intCert_stt *copiedCert, const intCert_stt *originalCert);

#endif /* X509_H */
