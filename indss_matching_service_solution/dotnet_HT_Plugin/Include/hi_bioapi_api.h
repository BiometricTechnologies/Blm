/**
* @file	hi_bioapi_api.h    
* @brief Hitachi Secure BioAPI function definition
* All Rights Reserved.    
* Copyright (C) 2011, Hitachi Solutions, Ltd.
* Copyright (C) 2006,2011, Hitachi, Ltd.
*/

#ifndef _HI_BIOAPI_API_H_INCLUDED
#define _HI_BIOAPI_API_H_INCLUDED

#include "hi_bioapi.h"
#include <wincrypt.h>

#ifdef WIN32
#define DllImport __declspec( dllimport )
#define DllExport __declspec( dllexport )
#else
#define DllImport
#define DllExport
#endif

#ifdef BIOAPI_EXPORTS
#define DLLAPI DllExport
#else
#define DLLAPI DllImport
#endif

#ifdef __cplusplus
extern "C" {
#endif

DLLAPI
BioAPI_RETURN BioAPI HiBioAPI_InitSecretKey (
	BioAPI_STRING* application_filename,
	ALG_ID algorithm_ID,
	BioAPI_STRING* inputseed,
	BioAPI_STRING* pin);

DLLAPI
BioAPI_RETURN BioAPI HiBioAPI_RestrictBSPFunction (
		BioAPI_STRING* application_filename,
    	int32_t usage,
		uint32_t	bioapi_operation_mask,
		BioAPI_STRING* pin);

DLLAPI
BioAPI_RETURN BioAPI HiBioAPI_InitVerifyParameter (
		BioAPI_STRING* application_filename,
		BioAPI_FMR	MaxFMRRequest,
    	int32_t trialNumber,
		int32_t minute,
		BioAPI_BOOL setFMRAchieved,
		hi_bioapi_securitycodeID securityCodeID,
		BioAPI_STRING* pin);

DLLAPI
BioAPI_RETURN BioAPI HiBioAPI_InitIdentifyParameter (
		BioAPI_STRING* application_filename,
		BioAPI_FMR	MaxFMRRequest,
    	int32_t trialNumber,
		int32_t minute,
		BioAPI_BOOL setFMRAchieved,
		hi_bioapi_securitycodeID securityCodeID,
		BioAPI_STRING* pin);

DLLAPI
BioAPI_RETURN BioAPI HiBioAPI_DeleteParameter (
		BioAPI_STRING* application_filename);

DLLAPI
BioAPI_RETURN BioAPI HiBioAPI_SetEndUserParameter (
		BioAPI_STRING* application_filename,
		CERT_ISSUER_SERIAL_NUMBER* cert);
		
DLLAPI
BioAPI_RETURN BioAPI HiBioAPI_VerifyBSP (
		const BioAPI_UUID *BSPUuid,
		BioAPI_STRING* bsphash
	);

DLLAPI
BioAPI_RETURN BioAPI HiBioAPI_VerifyChallenge (
		BioAPI_STRING* challengecode,
		hi_bioapi_verifystatus* verifystatus
	);

DLLAPI
BioAPI_RETURN BioAPI HiBioAPI_IdentifyChallenge (
		BioAPI_STRING* challengecode,
		hi_bioapi_identifystatus* identifystatus
	);

DLLAPI
BioAPI_RETURN BioAPI HiBioAPI_InitCertificate (
		BioAPI_STRING* application_filename,
		CERT_ISSUER_SERIAL_NUMBER* cert,
		BioAPI_STRING* pin);
#define HiBioAPI_InitCerfificate HiBioAPI_InitCertificate

DLLAPI
BioAPI_RETURN BioAPI HiBioAPI_VerifyReferenceBIR (
		BioAPI_BIR* referenceBIR);

DLLAPI
BioAPI_RETURN BioAPI HiBioAPI_DigitalSignBIR (
		BioAPI_BIR* encryptedBIR,
		BioAPI_BIR* referenceBIR);


#ifdef __cplusplus
}
#endif

#endif  /* _HI_BIOAPI_SPI_H_INCLUDED */