/**
* @file bioapi_api.h
* @brief BioAPI framework function definition
* All Rights Reserved.
* Copyright (C) 2011, Hitachi Solutions, Ltd.
* Copyright (C) 2006,2011, Hitachi, Ltd.
*/

#ifndef _BIOAPI_API_H_INCLUDED
#define _BIOAPI_API_H_INCLUDED

#include "bioapi_type.h"

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

/**********/

DLLAPI BioAPI_RETURN BioAPI
BioAPI_Init(BioAPI_VERSION Version);

DLLAPI BioAPI_RETURN BioAPI
BioAPI_Terminate (void);

DLLAPI BioAPI_RETURN BioAPI
BioAPI_GetFrameworkInfo(BioAPI_FRAMEWORK_SCHEMA *FrameworkSchema);

DLLAPI BioAPI_RETURN BioAPI
BioAPI_EnumBSPs(BioAPI_BSP_SCHEMA **BSPSchemaArray,
				uint32_t *NumberOfElements);

DLLAPI BioAPI_RETURN BioAPI
BioAPI_BSPLoad(const BioAPI_UUID *BSPUuid,
			   BioAPI_EventHandler AppNotifyCallback,
			   void *AppNotifyCallbackCtx);

DLLAPI BioAPI_RETURN BioAPI
BioAPI_BSPUnload(const BioAPI_UUID *BSPUuid,
				 BioAPI_EventHandler AppNotifyCallback,
				 void *AppNotifyCallbackCtx);

DLLAPI BioAPI_RETURN BioAPI
BioAPI_BSPAttach(const BioAPI_UUID *BSPUuid,
				 BioAPI_VERSION Version,
				 const BioAPI_UNIT_LIST_ELEMENT *UnitList,
				 uint32_t NumUnits,
				 BioAPI_HANDLE *NewBSPHandle);

DLLAPI BioAPI_RETURN BioAPI
BioAPI_BSPDetach(BioAPI_HANDLE BSPHandle);

DLLAPI BioAPI_RETURN BioAPI
BioAPI_QueryUnits(const BioAPI_UUID *BSPUuid,
				  BioAPI_UNIT_SCHEMA **UnitSchemaArray,
				  uint32_t *NumberOfElements);

DLLAPI BioAPI_RETURN BioAPI
BioAPI_EnumBFPs(BioAPI_BFP_SCHEMA **BFPSchemaArray,
				uint32_t *NumberOfElements);

DLLAPI BioAPI_RETURN BioAPI
BioAPI_QueryBFPs(const BioAPI_UUID *BSPUuid,
				 BioAPI_BFP_LIST_ELEMENT **BFPList,
				 uint32_t *NumberOfElements);

DLLAPI BioAPI_RETURN BioAPI
BioAPI_ControlUnit(BioAPI_HANDLE BSPHandle,
				   BioAPI_UNIT_ID UnitID,
				   uint32_t ControlCode,
				   const BioAPI_DATA *InputData,
				   BioAPI_DATA *OutputData);


/**********/

DLLAPI BioAPI_RETURN BioAPI
BioAPI_FreeBIRHandle(BioAPI_HANDLE BSPHandle,
					 BioAPI_BIR_HANDLE Handle);

DLLAPI BioAPI_RETURN BioAPI
BioAPI_GetBIRFromHandle(BioAPI_HANDLE BSPHandle,
						BioAPI_BIR_HANDLE Handle,
						BioAPI_BIR *BIR);

DLLAPI BioAPI_RETURN BioAPI
BioAPI_GetHeaderFromHandle(BioAPI_HANDLE BSPHandle,
						   BioAPI_BIR_HANDLE Handle,
						   BioAPI_BIR_HEADER *Header);


/**********/

DLLAPI BioAPI_RETURN BioAPI
BioAPI_EnableEvents(BioAPI_HANDLE BSPHandle,
					BioAPI_EVENT_MASK Events);

DLLAPI BioAPI_RETURN BioAPI
BioAPI_SetGUICallbacks(BioAPI_HANDLE BSPHandle,
					   BioAPI_GUI_STREAMING_CALLBACK GuiStreamingCallback,
					   void *GuiStreamingCallbackCtx,
					   BioAPI_GUI_STATE_CALLBACK GuiStateCallback,
					   void *GuiStateCallbackCtx);


/**********/

DLLAPI BioAPI_RETURN BioAPI
BioAPI_Capture(BioAPI_HANDLE BSPHandle,
			   BioAPI_BIR_PURPOSE Purpose,
			   BioAPI_BIR_SUBTYPE Subtype,
			   const BioAPI_BIR_BIOMETRIC_DATA_FORMAT *OutputFormat,
			   BioAPI_BIR_HANDLE *CapturedBIR,
			   int32_t Timeout,
			   BioAPI_BIR_HANDLE *AuditData);

DLLAPI BioAPI_RETURN BioAPI
BioAPI_CreateTemplate(BioAPI_HANDLE BSPHandle,
					  const BioAPI_INPUT_BIR *CapturedBIR,
					  const BioAPI_INPUT_BIR *ReferenceTemplate,
					  const BioAPI_BIR_BIOMETRIC_DATA_FORMAT *OutputFormat,
					  BioAPI_BIR_HANDLE *NewTemplate,
					  const BioAPI_DATA *Payload,
					  BioAPI_UUID *TemplateUUID);

DLLAPI BioAPI_RETURN BioAPI
BioAPI_Process(BioAPI_HANDLE BSPHandle,
			   const BioAPI_INPUT_BIR *CapturedBIR,
			   const BioAPI_BIR_BIOMETRIC_DATA_FORMAT *OutputFormat,
			   BioAPI_BIR_HANDLE *ProcessedBIR);

DLLAPI BioAPI_RETURN BioAPI
BioAPI_ProcessWithAuxBIR(BioAPI_HANDLE BSPHandle,
						 const BioAPI_INPUT_BIR *CapturedBIR,
						 const BioAPI_INPUT_BIR *AuxiliaryData,
						 const BioAPI_BIR_BIOMETRIC_DATA_FORMAT *OutputFormat,
						 BioAPI_BIR_HANDLE *ProcessedBIR);

DLLAPI BioAPI_RETURN BioAPI
BioAPI_VerifyMatch(BioAPI_HANDLE BSPHandle,
				   BioAPI_FMR MaxFMRRequested,
				   const BioAPI_INPUT_BIR *ProcessedBIR,
				   const BioAPI_INPUT_BIR *ReferenceTemplate,
				   BioAPI_BIR_HANDLE *AdaptedBIR,
				   BioAPI_BOOL *Result,
				   BioAPI_FMR *FMRAchieved,
				   BioAPI_DATA *Payload);

DLLAPI BioAPI_RETURN BioAPI
BioAPI_IdentifyMatch(BioAPI_HANDLE BSPHandle,
					 BioAPI_FMR MaxFMRRequested,
					 const BioAPI_INPUT_BIR *ProcessedBIR,
					 const BioAPI_IDENTIFY_POPULATION *Population,
					 uint32_t TotalNumberOfTemplates,
					 BioAPI_BOOL Binning,
					 uint32_t MaxNumberOfResults,
					 uint32_t *NumberOfResults,
					 BioAPI_CANDIDATE **Candidates,
					 int32_t Timeout);

DLLAPI BioAPI_RETURN BioAPI
BioAPI_Enroll(BioAPI_HANDLE BSPHandle,
			  BioAPI_BIR_PURPOSE Purpose,
			  BioAPI_BIR_SUBTYPE Subtype,
			  const BioAPI_BIR_BIOMETRIC_DATA_FORMAT *OutputFormat,
			  const BioAPI_INPUT_BIR *ReferenceTemplate,
			  BioAPI_BIR_HANDLE *NewTemplate,
			  const BioAPI_DATA *Payload,
			  int32_t Timeout,
			  BioAPI_BIR_HANDLE *AuditData,
			  BioAPI_UUID *TemplateUUID);

DLLAPI BioAPI_RETURN BioAPI
BioAPI_Verify(BioAPI_HANDLE BSPHandle,
			  BioAPI_FMR MaxFMRRequested,
			  const BioAPI_INPUT_BIR *ReferenceTemplate,
			  BioAPI_BIR_SUBTYPE Subtype,
			  BioAPI_BIR_HANDLE *AdaptedBIR,
			  BioAPI_BOOL *Result,
			  BioAPI_FMR *FMRAchieved,
			  BioAPI_DATA *Payload,
			  int32_t Timeout,
			  BioAPI_BIR_HANDLE *AuditData);

DLLAPI BioAPI_RETURN BioAPI
BioAPI_Identify(BioAPI_HANDLE BSPHandle,
				BioAPI_FMR MaxFMRRequested,
				BioAPI_BIR_SUBTYPE Subtype,
				const BioAPI_IDENTIFY_POPULATION *Population,
				uint32_t TotalNumberOfTemplates,
				BioAPI_BOOL Binning,
				uint32_t MaxNumberOfResults,
				uint32_t *NumberOfResults,
				BioAPI_CANDIDATE **Candidates,
				int32_t Timeout,
				BioAPI_BIR_HANDLE *AuditData);

DLLAPI BioAPI_RETURN BioAPI
BioAPI_Import(BioAPI_HANDLE BSPHandle,
			  const BioAPI_DATA *InputData,
			  const BioAPI_BIR_BIOMETRIC_DATA_FORMAT *InputFormat,
			  const BioAPI_BIR_BIOMETRIC_DATA_FORMAT *OutputFormat,
			  BioAPI_BIR_PURPOSE Purpose,
			  BioAPI_BIR_HANDLE *ConstructedBIR);

DLLAPI BioAPI_RETURN BioAPI
BioAPI_PresetIdentifyPopulation(BioAPI_HANDLE BSPHandle,
					 			const BioAPI_IDENTIFY_POPULATION *Population);


/**********/

DLLAPI BioAPI_RETURN BioAPI
BioAPI_DbOpen(BioAPI_HANDLE BSPHandle,
			  const BioAPI_UUID *DbUuid,
			  BioAPI_DB_ACCESS_TYPE AccessRequest,
			  BioAPI_DB_HANDLE *DbHandle,
			  BioAPI_DB_MARKER_HANDLE *MarkerHandle);

DLLAPI BioAPI_RETURN BioAPI
BioAPI_DbClose(BioAPI_HANDLE BSPHandle,
			   BioAPI_DB_HANDLE DbHandle);

DLLAPI BioAPI_RETURN BioAPI
BioAPI_DbCreate(BioAPI_HANDLE BSPHandle,
				const BioAPI_UUID *DbUuid,
				uint32_t NumberOfRecords,
				BioAPI_DB_ACCESS_TYPE AccessRequest,
				BioAPI_DB_HANDLE *DbHandle);

DLLAPI BioAPI_RETURN BioAPI
BioAPI_DbDelete(BioAPI_HANDLE BSPHandle,
				const BioAPI_UUID *DbUuid);

DLLAPI BioAPI_RETURN BioAPI
BioAPI_DbSetMarker(BioAPI_HANDLE BSPHandle,
				   BioAPI_DB_HANDLE DbHandle,
				   const BioAPI_UUID *KeyValue,
				   BioAPI_DB_MARKER_HANDLE MarkerHandle);

DLLAPI BioAPI_RETURN BioAPI
BioAPI_DbFreeMarker(BioAPI_HANDLE BSPHandle,
					BioAPI_DB_MARKER_HANDLE MarkerHandle);

DLLAPI BioAPI_RETURN BioAPI
BioAPI_DbStoreBIR(BioAPI_HANDLE BSPHandle,
				  const BioAPI_INPUT_BIR *BIRToStore,
				  BioAPI_DB_HANDLE DbHandle,
				  BioAPI_UUID *BirUuid);

DLLAPI BioAPI_RETURN BioAPI
BioAPI_DbGetBIR(BioAPI_HANDLE BSPHandle,
				BioAPI_DB_HANDLE DbHandle,
				const BioAPI_UUID *KeyValue,
				BioAPI_BIR_HANDLE *RetrievedBIR,
				BioAPI_DB_MARKER_HANDLE *MarkerHandle);

DLLAPI BioAPI_RETURN BioAPI
BioAPI_DbGetNextBIR(BioAPI_HANDLE BSPHandle,
					BioAPI_DB_HANDLE DbHandle,
					BioAPI_DB_MARKER_HANDLE MarkerHandle,
					BioAPI_BIR_HANDLE *RetrievedBIR,
					BioAPI_UUID *BirUuid);

DLLAPI BioAPI_RETURN BioAPI
BioAPI_DbDeleteBIR(BioAPI_HANDLE BSPHandle,
				   BioAPI_DB_HANDLE DbHandle,
				   const BioAPI_UUID *KeyValue);


/**********/

DLLAPI BioAPI_RETURN BioAPI
BioAPI_SetPowerMode(BioAPI_HANDLE BSPHandle,
					BioAPI_UNIT_ID UnitId,
					BioAPI_POWER_MODE PowerMode);

DLLAPI BioAPI_RETURN BioAPI
BioAPI_SetIndicatorStatus(BioAPI_HANDLE BSPHandle,
						  BioAPI_UNIT_ID UnitId,
						  BioAPI_INDICATOR_STATUS IndicatorStatus);

DLLAPI BioAPI_RETURN BioAPI
BioAPI_GetIndicatorStatus(BioAPI_HANDLE BSPHandle,
						  BioAPI_UNIT_ID UnitId,
						  BioAPI_INDICATOR_STATUS *IndicatorStatus);

DLLAPI BioAPI_RETURN BioAPI
BioAPI_CalibrateSensor(BioAPI_HANDLE BSPHandle,
					   int32_t Timeout);


/**********/

DLLAPI BioAPI_RETURN BioAPI
BioAPI_Cancel(BioAPI_HANDLE BSPHandle);

DLLAPI BioAPI_RETURN BioAPI
BioAPI_Free(void *Ptr);


/**********/

DLLAPI BioAPI_RETURN BioAPI
BioAPI_Util_InstallBSP(BioAPI_INSTALL_ACTION Action,
					   BioAPI_INSTALL_ERROR *Error,
					   const BioAPI_BSP_SCHEMA *BSPSchema);

DLLAPI BioAPI_RETURN BioAPI
BioAPI_Util_InstallBFP(BioAPI_INSTALL_ACTION Action,
					   BioAPI_INSTALL_ERROR *Error,
					   const BioAPI_BFP_SCHEMA *BFPSchema);


#ifdef __cplusplus
}
#endif

#endif  /* _BIOAPI_API_H_INCLUDED */
