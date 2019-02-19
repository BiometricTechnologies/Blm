/**
* @file  bioapi_type.h
* @brief BioAPI basic data type definition
* All Rights Reserved.
* Copyright (C) 2011, Hitachi Solutions, Ltd.
* Copyright (C) 2006,2011, Hitachi, Ltd.
*/

#ifndef _BIOAPI_TYPE_H_INCLUDED
#define _BIOAPI_TYPE_H_INCLUDED

// BioAPI types and macros

#ifdef WIN32
#define BioAPI __stdcall
#else
#define BioAPI
#endif

// 
//typedef unsigned char  uint8_t;
//typedef char           sint8_t;
//typedef char           int8_t;
//typedef unsigned short uint16_t;
//typedef short          sint16_t;
//typedef short          int16_t;
//typedef unsigned int   uint32_t;
//typedef int            sint32_t;
//typedef int            int32_t;


// 

typedef uint32_t BioAPI_RETURN;
#define BioAPI_OK (0)

// 
typedef uint8_t BioAPI_BOOL;
#define BioAPI_FALSE (BioAPI_BOOL)(0)
#define BioAPI_TRUE (BioAPI_BOOL)(!BioAPI_FALSE)

typedef uint8_t BioAPI_VERSION;

typedef uint8_t BioAPI_STRING [269];

typedef uint8_t BioAPI_UUID[16];

typedef uint32_t BioAPI_DB_ACCESS_TYPE;
#define BioAPI_DB_ACCESS_READ (0x00000001)
#define BioAPI_DB_ACCESS_WRITE (0x00000002)

typedef uint32_t BioAPI_DB_MARKER_HANDLE;

typedef uint32_t BioAPI_CATEGORY;
#define BioAPI_CATEGORY_ARCHIVE (0x00000001)
#define BioAPI_CATEGORY_MATCHING_ALG (0x00000002)
#define BioAPI_CATEGORY_PROCESSING_ALG (0x00000004)
#define BioAPI_CATEGORY_SENSOR (0x00000008)

typedef uint8_t BioAPI_BIR_DATA_TYPE;
#define BioAPI_BIR_DATA_TYPE_RAW (0x01)
#define BioAPI_BIR_DATA_TYPE_INTERMEDIATE (0x02)
#define BioAPI_BIR_DATA_TYPE_PROCESSED (0x04)
#define BioAPI_BIR_DATA_TYPE_ENCRYPTED (0x10)
#define BioAPI_BIR_DATA_TYPE_SIGNED (0x20)
#define BioAPI_BIR_INDEX_PRESENT (0x80)

typedef int32_t BioAPI_BIR_HANDLE;
#define BioAPI_INVALID_BIR_HANDLE (-1)
#define BioAPI_UNSUPPORTED_BIR_HANDLE (-2)

typedef int8_t BioAPI_QUALITY;

typedef uint8_t BioAPI_BIR_PURPOSE;
#define BioAPI_PURPOSE_VERIFY (1)
#define BioAPI_PURPOSE_IDENTIFY (2)
#define BioAPI_PURPOSE_ENROLL (3)
#define BioAPI_PURPOSE_ENROLL_FOR_VERIFICATION_ONLY (4)
#define BioAPI_PURPOSE_ENROLL_FOR_IDENTIFICATION_ONLY (5)
#define BioAPI_PURPOSE_AUDIT (6)
#define BioAPI_NO_PURPOSE_AVAILABLE (0)

typedef uint32_t BioAPI_BIR_BIOMETRIC_TYPE;
#define BioAPI_NO_TYPE_AVAILABLE (0x00000000)
#define BioAPI_TYPE_MULTIPLE (0x00000001)
#define BioAPI_TYPE_FACIAL_FEATURES (0x00000002)
#define BioAPI_TYPE_VOICE (0x00000004)
#define BioAPI_TYPE_FINGERPRINT (0x00000008)
#define BioAPI_TYPE_IRIS (0x00000010)
#define BioAPI_TYPE_RETINA (0x00000020)
#define BioAPI_TYPE_HAND_GEOMETRY (0x00000040)
#define BioAPI_TYPE_SIGNATURE_DYNAMICS (0x00000080)
#define BioAPI_TYPE_KEYSTOKE_DYNAMICS (0x00000100)
#define BioAPI_TYPE_LIP_MOVEMENT (0x00000200)
#define BioAPI_TYPE_THERMAL_FACE_IMAGE (0x00000400)
#define BioAPI_TYPE_THERMAL_HAND_IMAGE (0x00000800)
#define BioAPI_TYPE_GAIT (0x00001000)
#define BioAPI_TYPE_OTHER (0x40000000)
#define BioAPI_TYPE_PASSWORD (0x80000000)

typedef uint8_t BioAPI_BIR_SUBTYPE;
#define BioAPI_BIR_SUBTYPE_LEFT (0x01)
#define BioAPI_BIR_SUBTYPE_RIGHT (0x02)
#define BioAPI_BIR_SUBTYPE_THUMB (0x04)
#define BioAPI_BIR_SUBTYPE_POINTERFINGER (0x08)
#define BioAPI_BIR_SUBTYPE_MIDDLEFINGER (0x10)
#define BioAPI_BIR_SUBTYPE_RINGFINGER (0x20)
#define BioAPI_BIR_SUBTYPE_LITTLEFINGER (0x40)
#define BioAPI_BIR_SUBTYPE_MULTIPLE (0x80)
#define BioAPI_NO_SUBTYPE_AVAILABLE (0x00)

typedef uint8_t BioAPI_IDENTIFY_POPULATION_TYPE;
#define BioAPI_DB_TYPE (1)
#define BioAPI_ARRAY_TYPE (2)
#define BioAPI_PRESET_ARRAY_TYPE (3)

typedef int32_t BioAPI_DB_HANDLE;
#define BioAPI_DB_INVALID_HANDLE (-1)
#define BioAPI_DB_DEFAULT_HANDLE (0)
#define BioAPI_DB_DEFAULT_UUID_PTR (NULL)

typedef uint32_t BioAPI_EVENT;
#define BioAPI_NOTIFY_INSERT (1)
#define BioAPI_NOTIFY_REMOVE (2)
#define BioAPI_NOTIFY_FAULT (3)
#define BioAPI_NOTIFY_SOURCE_PRESENT (4)
#define BioAPI_NOTIFY_SOURCE_REMOVED (5)

typedef uint32_t BioAPI_EVENT_MASK;
#define BioAPI_NOTIFY_INSERT_BIT (0x00000001)
#define BioAPI_NOTIFY_REMOVE_BIT (0x00000002)
#define BioAPI_NOTIFY_FAULT_BIT (0x00000004)
#define BioAPI_NOTIFY_SOURCE_PRESENT_BIT (0x00000008)
#define BioAPI_NOTIFY_SOURCE_REMOVED_BIT (0x00000010)

typedef uint32_t BioAPI_UNIT_ID;
#define BioAPI_DONT_CARE (0x00000000)
#define BioAPI_DONT_INCLUDE (0xFFFFFFFF)

typedef uint32_t BioAPI_GUI_MESSAGE;

typedef uint8_t BioAPI_GUI_PROGRESS;

typedef uint8_t BioAPI_GUI_RESPONSE;
#define BioAPI_CAPTURE_SAMPLE (1) /* Instruction to BSP to capture sample */
#define BioAPI_CANCEL (2) /* User cancelled operation */
#define BioAPI_CONTINUE (3) /* User or application selects to proceed */
#define BioAPI_VALID_SAMPLE (4) /* Valid sample received */
#define BioAPI_INVALID_SAMPLE (5) /* Invalid sample received */

typedef uint32_t BioAPI_GUI_STATE;
#define BioAPI_SAMPLE_AVAILABLE (0x0001) /* Sample captured and available */
#define BioAPI_MESSAGE_PROVIDED (0x0002) /* BSP provided message for display */
#define BioAPI_PROGRESS_PROVIDED (0x0004) /* BSP provide progress for display */

typedef uint32_t BioAPI_HANDLE;
#define BioAPI_INVALID_BSP_HANDLE (0)

typedef uint8_t BioAPI_INDICATOR_STATUS;
#define BioAPI_INDICATOR_ACCEPT (1)
#define BioAPI_INDICATOR_REJECT (2)
#define BioAPI_INDICATOR_READY (3)
#define BioAPI_INDICATOR_BUSY (4)
#define BioAPI_INDICATOR_FAILURE (5)

typedef uint8_t BioAPI_INPUT_BIR_FORM;
#define BioAPI_DATABASE_ID_INPUT (1)
#define BioAPI_BIR_HANDLE_INPUT (2)
#define BioAPI_FULLBIR_INPUT (3)

typedef uint32_t BioAPI_INSTALL_ACTION;
#define BioAPI_INSTALL_ACTION_INSTALL (1)
#define BioAPI_INSTALL_ACTION_REFRESH (2)
#define BioAPI_INSTALL_ACTION_UNINSTALL (3)

typedef uint32_t BioAPI_OPERATIONS_MASK;
#define BioAPI_ENABLEEVENTS (0x00000001)
#define BioAPI_SETGUICALLBACKS (0x00000002)
#define BioAPI_CAPTURE (0x00000004)
#define BioAPI_CREATETEMPLATE (0x00000008)
#define BioAPI_PROCESS (0x00000010)
#define BioAPI_PROCESSWITHAUXBIR (0x00000020)
#define BioAPI_VERIFYMATCH (0x00000040)
#define BioAPI_IDENTIFYMATCH (0x00000080)
#define BioAPI_ENROLL (0x00000100)
#define BioAPI_VERIFY (0x00000200)
#define BioAPI_IDENTIFY (0x00000400)
#define BioAPI_IMPORT (0x00000800)
#define BioAPI_PRESETIDENTIFYPOPULATION (0x00001000)
#define BioAPI_DATABASEOPERATIONS (0x00002000)
#define BioAPI_SETPOWERMODE (0x00004000)
#define BioAPI_SETINDICATORSTATUS (0x00008000)
#define BioAPI_GETINDICATORSTATUS (0x00010000)
#define BioAPI_CALIBRATESENSOR (0x00020000)
#define BioAPI_UTILITIES (0X00040000)
#define BioAPI_QUERYUNITS (0x00100000)
#define BioAPI_QUERYBFPS (0x00200000)
#define BioAPI_CONTROLUNIT (0X00400000)

typedef uint32_t BioAPI_OPTIONS_MASK;
#define BioAPI_RAW (0x00000001)
#define BioAPI_QUALITY_RAW (0x00000002)
#define BioAPI_QUALITY_INTERMEDIATE (0x00000004)
#define BioAPI_QUALITY_PROCESSED (0x00000008)
#define BioAPI_APP_GUI (0x00000010)
#define BioAPI_STREAMINGDATA (0x00000020)
#define BioAPI_SOURCEPRESENT (0x00000040)
#define BioAPI_PAYLOAD (0x00000080)
#define BioAPI_BIR_SIGN (0x00000100)
#define BioAPI_BIR_ENCRYPT (0x00000200)
#define BioAPI_TEMPLATEUPDATE (0x00000400)
#define BioAPI_ADAPTATION (0x00000800)
#define BioAPI_BINNING (0x00001000)
#define BioAPI_SELFCONTAINEDDEVICE (0x00002000)
#define BioAPI_MOC (0x00004000)
#define BioAPI_SUBTYPE_TO_CAPTURE (0x00008000)
#define BioAPI_SENSORBFP (0x00010000)
#define BioAPI_ARCHIVEBFP (0x00020000)
#define BioAPI_MATCHINGBFP (0x00040000)
#define BioAPI_PROCESSINGBFP (0x00080000)
#define BioAPI_COARSESCORES (0x00100000)

typedef uint32_t BioAPI_POWER_MODE;
/* All functions available */
#define BioAPI_POWER_NORMAL (1)
/* Able to detect (for example) insertion/finger on/person present type of events */
#define BioAPI_POWER_DETECT (2)
/* Minimum mode. All functions off */
#define BioAPI_POWER_SLEEP (3)

typedef struct bioapi_bir_biometric_data_format {
	uint16_t FormatOwner;
	uint16_t FormatType;
} BioAPI_BIR_BIOMETRIC_DATA_FORMAT;

typedef struct bioapi_bir_biometric_product_ID {
	uint16_t ProductOwner;
	uint16_t ProductType;
} BioAPI_BIR_BIOMETRIC_PRODUCT_ID;
#define BioAPI_NO_PRODUCT_OWNER_AVAILABLE (0x0000)
#define BioAPI_NO_PRODUCT_TYPE_AVAILABLE (0x0000)

typedef struct bioapi_date {
	uint16_t Year; /* valid range: 1900 . 9999 */
	uint8_t Month; /* valid range: 01 . 12 */
	uint8_t Day; /* valid range: 01 . 31, consistent with associated month/year */
} BioAPI_DATE;
#define BioAPI_NO_YEAR_AVAILABLE (0)
#define BioAPI_NO_MONTH_AVAILABLE (0)
#define BioAPI_NO_DAY_AVAILABLE (0)

typedef struct bioapi_time {
	uint8_t Hour; /* valid range: 00 . 23, 99 */
	uint8_t Minute ; /* valid range: 00 . 59, 99 */
	uint8_t Second ; /* valid range: 00 . 59, 99 */
} BioAPI_TIME;
#define BioAPI_NO_HOUR_AVAILABLE (99)
#define BioAPI_NO_MINUTE_AVAILABLE (99)
#define BioAPI_NO_SECOND_AVAILABLE (99)

typedef struct bioapi_DTG {
	BioAPI_DATE Date;
	BioAPI_TIME Time;
} BioAPI_DTG;

typedef struct bioapi_bir_security_block_format {
	uint16_t SecurityFormatOwner;
	uint16_t SecurityFormatType;
} BioAPI_BIR_SECURITY_BLOCK_FORMAT;

typedef struct bioapi_bir_header {
	BioAPI_VERSION HeaderVersion;
	BioAPI_BIR_DATA_TYPE Type;
	BioAPI_BIR_BIOMETRIC_DATA_FORMAT Format;
	BioAPI_QUALITY Quality;
	BioAPI_BIR_PURPOSE Purpose;
	BioAPI_BIR_BIOMETRIC_TYPE FactorsMask;
	BioAPI_BIR_BIOMETRIC_PRODUCT_ID ProductID;
	BioAPI_DTG CreationDTG;
	BioAPI_BIR_SUBTYPE Subtype;
	BioAPI_DATE ExpirationDate;
	BioAPI_BIR_SECURITY_BLOCK_FORMAT SBFormat;
	BioAPI_UUID Index;
} BioAPI_BIR_HEADER;

typedef struct bioapi_data{
	uint32_t Length; /* in bytes */
	void *Data;
} BioAPI_DATA;

typedef struct bioapi_bir {
	BioAPI_BIR_HEADER Header;
	BioAPI_DATA BiometricData;
	BioAPI_DATA SecurityBlock; /* SecurityBlock.Data=NULL if no SB */
} BioAPI_BIR;


typedef struct bioapi_bir_array_population {
	uint32_t NumberOfMembers;
	BioAPI_BIR *Members; /* A pointer to an array of BIRs */
} BioAPI_BIR_ARRAY_POPULATION;

typedef struct bioapi_identify_population {
	BioAPI_IDENTIFY_POPULATION_TYPE Type;
	union {
		BioAPI_DB_HANDLE *BIRDataBase;
		BioAPI_BIR_ARRAY_POPULATION *BIRArray;
	} BIRs;
} BioAPI_IDENTIFY_POPULATION;

typedef int32_t BioAPI_FMR;
#define BioAPI_NOT_SET (-1)

typedef struct bioapi_candidate {
	BioAPI_IDENTIFY_POPULATION_TYPE Type;
	union {
		BioAPI_UUID *BIRInDataBase;
		uint32_t *BIRInArray;
	} BIR;
	BioAPI_FMR FMRAchieved;
} BioAPI_CANDIDATE;

typedef struct bioapi_dbbir_id {
	BioAPI_DB_HANDLE DbHandle;
	BioAPI_UUID KeyValue;
} BioAPI_DBBIR_ID;

typedef struct bioapi_gui_bitmap {
	uint32_t Width; /* Width of bitmap in pixels (number of pixels for each line) */
	uint32_t Height; /* Height of bitmap in pixels (number of lines) */
	BioAPI_DATA Bitmap;
} BioAPI_GUI_BITMAP;

typedef BioAPI_RETURN (BioAPI *BioAPI_GUI_STATE_CALLBACK)
	(void *GuiStateCallbackCtx,
	BioAPI_GUI_STATE GuiState,
	BioAPI_GUI_RESPONSE *Response,
	BioAPI_GUI_MESSAGE Message,
	BioAPI_GUI_PROGRESS Progress,
	const BioAPI_GUI_BITMAP *SampleBuffer);

typedef BioAPI_RETURN (BioAPI *BioAPI_GUI_STREAMING_CALLBACK)
	(void *GuiStreamingCallbackCtx,
	const BioAPI_GUI_BITMAP *Bitmap);

typedef struct bioapi_input_bir {
	BioAPI_INPUT_BIR_FORM Form;
	union {
		BioAPI_DBBIR_ID *BIRinDb;
		BioAPI_BIR_HANDLE *BIRinBSP;
		BioAPI_BIR *BIR;
	} InputBIR;
} BioAPI_INPUT_BIR;

typedef struct install_error{
	BioAPI_RETURN ErrorCode;
	BioAPI_STRING ErrorString;
} BioAPI_INSTALL_ERROR;

typedef struct bioapi_unit_list_element {
	BioAPI_CATEGORY UnitCategory;
	BioAPI_UNIT_ID UnitId;
} BioAPI_UNIT_LIST_ELEMENT;

typedef struct bioapi_bfp_list_element {
	BioAPI_CATEGORY BFPCategory;
	BioAPI_UUID BFPUuid;
} BioAPI_BFP_LIST_ELEMENT;

typedef struct bioapi_framework_schema {
	BioAPI_UUID FrameworkUuid;
	BioAPI_STRING FwDescription;
	uint8_t *Path;
	BioAPI_VERSION SpecVersion;
	BioAPI_STRING ProductVersion;
	BioAPI_STRING Vendor;
	BioAPI_UUID FwPropertyId;
	BioAPI_DATA FwProperty;
} BioAPI_FRAMEWORK_SCHEMA;

typedef struct bioapi_bsp_schema {
	BioAPI_UUID BSPUuid;
	BioAPI_STRING BSPDescription;
	uint8_t *Path;
	BioAPI_VERSION SpecVersion;
	BioAPI_STRING ProductVersion;
	BioAPI_STRING Vendor;
	BioAPI_BIR_BIOMETRIC_DATA_FORMAT *BSPSupportedFormats;
	uint32_t NumSupportedFormats;
	BioAPI_BIR_BIOMETRIC_TYPE FactorsMask;
	BioAPI_OPERATIONS_MASK Operations;
	BioAPI_OPTIONS_MASK Options;
	BioAPI_FMR PayloadPolicy;
	uint32_t MaxPayloadSize;
	int32_t DefaultVerifyTimeout;
	int32_t DefaultIdentifyTimeout;
	int32_t DefaultCaptureTimeout;
	int32_t DefaultEnrollTimeout;
	int32_t DefaultCalibrateTimeout;
	uint32_t MaxBSPDbSize;
	uint32_t MaxIdentify;
}BioAPI_BSP_SCHEMA;

typedef struct bioapi_unit_schema {
	BioAPI_UUID BSPUuid;
	BioAPI_UUID UnitManagerUuid;
	BioAPI_UNIT_ID UnitId;
	BioAPI_CATEGORY UnitCategory;
	BioAPI_UUID UnitProperties;
	BioAPI_STRING VendorInformation;
	uint32_t SupportedEvents;
	BioAPI_UUID UnitPropertyID;
	BioAPI_DATA UnitProperty;
	BioAPI_STRING HardwareVersion;
	BioAPI_STRING FirmwareVersion;
	BioAPI_STRING SoftwareVersion;
	BioAPI_STRING HardwareSerialNumber;
	BioAPI_BOOL AuthenticatedHardware;
	uint32_t MaxBSPDbSize;
	uint32_t MaxIdentify;
} BioAPI_UNIT_SCHEMA;

typedef struct bioapi_bfp_schema {
	BioAPI_UUID BFPUuid;
	BioAPI_CATEGORY BFPCategory;
	BioAPI_STRING BFPDescription;
	uint8_t *Path;
	BioAPI_VERSION SpecVersion;
	BioAPI_STRING ProductVersion;
	BioAPI_STRING Vendor;
	BioAPI_BIR_BIOMETRIC_DATA_FORMAT *BFPSupportedFormats;
	uint32_t NumSupportedFormats;
	BioAPI_BIR_BIOMETRIC_TYPE FactorsMask;
	BioAPI_UUID BFPPropertyID;
	BioAPI_DATA BFPProperty;
} BioAPI_BFP_SCHEMA;

typedef BioAPI_RETURN (BioAPI *BioAPI_EventHandler)
	(const BioAPI_UUID *BSPUuid,
	BioAPI_UNIT_ID UnitID,
	void* AppNotifyCallbackCtx,
	const BioAPI_UNIT_SCHEMA *UnitSchema,
	BioAPI_EVENT EventType);

#endif// _BIOAPI_TYPE_H_INCLUDED
