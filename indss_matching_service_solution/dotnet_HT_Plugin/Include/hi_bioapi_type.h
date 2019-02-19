/**
* @file	hi_bioapi_type.h   
* @brief Hitachi Secure BioAPI data type definition
* All Rights Reserved.    
* Copyright (C) 2011, Hitachi Solutions, Ltd.
* Copyright (C) 2006,2011, Hitachi, Ltd.
*/

#include <windows.h>
#include <wincrypt.h>//CryptoAPIŽg—p
#ifndef _HI_BIOAPI_TYPE_H_INCLUDED
#define _HI_BIOAPI_TYPE_H_INCLUDED

//
#define HI_BIOAPI_ALG_SIGNATURE			( 30 )
#define HI_BIOAPI_ALG_HMAC				( 40 )

//1:1
typedef struct hi_bioapi_verifystatus{
	BioAPI_BOOL Result;
	BioAPI_FMR  achievedFMR;
	BioAPI_QUALITY samplaQuality;
	BioAPI_STRING HashReferenceBIR;
	BioAPI_STRING challengecode;
	BioAPI_DATA Securitycode;
}HI_BIOAPI_VERIFYSTATUS;

//1:N
typedef struct hi_bioapi_identifystatus{
	BioAPI_BOOL Result;
	BioAPI_CANDIDATE** candidate;
	BioAPI_QUALITY samplaQuality;
	BioAPI_STRING challengecode;
	BioAPI_DATA Securitycode;
}HI_BIOAPI_IDENTIFYSTATUS;

//
typedef struct hi_bioapi_securitycodeID{
	int32_t securitycodeType;
	BioAPI_STRING macSeed;
}HI_BIOAPI_SECURITYCODEID;




#endif 