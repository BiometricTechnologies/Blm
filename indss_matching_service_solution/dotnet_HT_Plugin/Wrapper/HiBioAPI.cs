using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Runtime.InteropServices;
using System.Security.Permissions;
using System.Security;
namespace Hitachi
{

    internal static partial class NativeMethods
    {
        private const string DLLFilename = "HiBioAPI.dll";
        /// Return Type: BioAPI_RETURN->uint32_t->unsigned int
        ///Version: BioAPI_VERSION->uint8_t->unsigned char
        [DllImportAttribute(DLLFilename, EntryPoint = "BioAPI_Init", CallingConvention = CallingConvention.StdCall)]
        public static extern uint BioAPI_Init(byte Version);


        /// Return Type: BioAPI_RETURN->uint32_t->unsigned int
        [DllImportAttribute(DLLFilename, EntryPoint = "BioAPI_Terminate", CallingConvention = CallingConvention.StdCall)]
        public static extern uint BioAPI_Terminate();


        /// Return Type: BioAPI_RETURN->uint32_t->unsigned int
        ///FrameworkSchema: BioAPI_FRAMEWORK_SCHEMA*
        [DllImportAttribute(DLLFilename, EntryPoint = "BioAPI_GetFrameworkInfo", CallingConvention = CallingConvention.StdCall)]
        public static extern uint BioAPI_GetFrameworkInfo(ref bioapi_framework_schema FrameworkSchema);


        /// Return Type: BioAPI_RETURN->uint32_t->unsigned int
        ///BSPSchemaArray: BioAPI_BSP_SCHEMA**
        ///NumberOfElements: uint32_t*
        [DllImportAttribute(DLLFilename, EntryPoint = "BioAPI_EnumBSPs", CallingConvention = CallingConvention.StdCall)]
        public static extern uint BioAPI_EnumBSPs(ref System.IntPtr BSPSchemaArray, ref uint NumberOfElements);


        /// Return Type: BioAPI_RETURN->uint32_t->unsigned int
        ///BSPUuid: BioAPI_UUID*
        ///AppNotifyCallback: BioAPI_EventHandler
        ///AppNotifyCallbackCtx: void*
        [DllImportAttribute(DLLFilename, EntryPoint = "BioAPI_BSPLoad", CallingConvention = CallingConvention.StdCall)]
        public static extern uint BioAPI_BSPLoad(System.IntPtr BSPUuid, BioAPI.EventHandler AppNotifyCallback, System.IntPtr AppNotifyCallbackCtx);


        /// Return Type: BioAPI_RETURN->uint32_t->unsigned int
        ///BSPUuid: BioAPI_UUID*
        ///AppNotifyCallback: BioAPI_EventHandler
        ///AppNotifyCallbackCtx: void*
        [DllImportAttribute(DLLFilename, EntryPoint = "BioAPI_BSPUnload", CallingConvention = CallingConvention.StdCall)]
        public static extern uint BioAPI_BSPUnload(System.IntPtr BSPUuid, BioAPI.EventHandler AppNotifyCallback, System.IntPtr AppNotifyCallbackCtx);


        /// Return Type: BioAPI_RETURN->uint32_t->unsigned int
        ///BSPUuid: BioAPI_UUID*
        ///Version: BioAPI_VERSION->uint8_t->unsigned char
        ///UnitList: BioAPI_UNIT_LIST_ELEMENT*
        ///NumUnits: uint32_t->unsigned int
        ///NewBSPHandle: BioAPI_HANDLE*
        [DllImportAttribute(DLLFilename, EntryPoint = "BioAPI_BSPAttach", CallingConvention = CallingConvention.StdCall)]
        public static extern uint BioAPI_BSPAttach(System.IntPtr BSPUuid, byte Version, ref bioapi_unit_list_element UnitList, uint NumUnits, ref uint NewBSPHandle);


        /// Return Type: BioAPI_RETURN->uint32_t->unsigned int
        ///BSPHandle: BioAPI_HANDLE->uint32_t->unsigned int
        [DllImportAttribute(DLLFilename, EntryPoint = "BioAPI_BSPDetach", CallingConvention = CallingConvention.StdCall)]
        public static extern uint BioAPI_BSPDetach(uint BSPHandle);


        /// Return Type: BioAPI_RETURN->uint32_t->unsigned int
        ///BSPUuid: BioAPI_UUID*
        ///UnitSchemaArray: BioAPI_UNIT_SCHEMA**
        ///NumberOfElements: uint32_t*
        [DllImportAttribute(DLLFilename, EntryPoint = "BioAPI_QueryUnits", CallingConvention = CallingConvention.StdCall)]
        public static extern uint BioAPI_QueryUnits(System.IntPtr BSPUuid, ref System.IntPtr UnitSchemaArray, ref uint NumberOfElements);



        /// Return Type: BioAPI_RETURN->uint32_t->unsigned int
        ///BSPHandle: BioAPI_HANDLE->uint32_t->unsigned int
        ///UnitID: BioAPI_UNIT_ID->uint32_t->unsigned int
        ///ControlCode: uint32_t->unsigned int
        ///InputData: BioAPI_DATA*
        ///OutputData: BioAPI_DATA*
        [DllImportAttribute(DLLFilename, EntryPoint = "BioAPI_ControlUnit", CallingConvention = CallingConvention.StdCall)]
        public static extern uint BioAPI_ControlUnit(uint BSPHandle, uint UnitID, uint ControlCode, ref bioapi_data InputData, IntPtr OutputData);

        /// Return Type: BioAPI_RETURN->uint32_t->unsigned int
        ///Ptr: void*
        [DllImportAttribute(DLLFilename, EntryPoint = "BioAPI_Free", CallingConvention = CallingConvention.StdCall)]
        public static extern uint BioAPI_Free(System.IntPtr Ptr);


        /// Return Type: BioAPI_RETURN->uint32_t->unsigned int
        ///BSPHandle: BioAPI_HANDLE->uint32_t->unsigned int
        ///Handle: BioAPI_BIR_HANDLE->int32_t->int
        [DllImportAttribute(DLLFilename, EntryPoint = "BioAPI_FreeBIRHandle", CallingConvention = CallingConvention.StdCall)]
        public static extern uint BioAPI_FreeBIRHandle(uint BSPHandle, int Handle);


        /// Return Type: BioAPI_RETURN->uint32_t->unsigned int
        ///BSPHandle: BioAPI_HANDLE->uint32_t->unsigned int
        ///Handle: BioAPI_BIR_HANDLE->int32_t->int
        ///BIR: BioAPI_BIR*
        [DllImportAttribute(DLLFilename, EntryPoint = "BioAPI_GetBIRFromHandle", CallingConvention = CallingConvention.StdCall)]
        public static extern uint BioAPI_GetBIRFromHandle(uint BSPHandle, int Handle, ref bioapi_bir BIR);


        /// Return Type: BioAPI_RETURN->uint32_t->unsigned int
        ///BSPHandle: BioAPI_HANDLE->uint32_t->unsigned int
        ///Handle: BioAPI_BIR_HANDLE->int32_t->int
        ///Header: BioAPI_BIR_HEADER*
        [DllImportAttribute(DLLFilename, EntryPoint = "BioAPI_GetHeaderFromHandle", CallingConvention = CallingConvention.StdCall)]
        public static extern uint BioAPI_GetHeaderFromHandle(uint BSPHandle, int Handle, ref bioapi_bir_header Header);


        /// Return Type: BioAPI_RETURN->uint32_t->unsigned int
        ///BSPHandle: BioAPI_HANDLE->uint32_t->unsigned int
        ///Events: BioAPI_EVENT_MASK->uint32_t->unsigned int
        [DllImportAttribute(DLLFilename, EntryPoint = "BioAPI_EnableEvents", CallingConvention = CallingConvention.StdCall)]
        public static extern uint BioAPI_EnableEvents(uint BSPHandle, uint Events);


        /// Return Type: BioAPI_RETURN->uint32_t->unsigned int
        ///BSPHandle: BioAPI_HANDLE->uint32_t->unsigned int
        ///GuiStreamingCallback: BioAPI_GUI_STREAMING_CALLBACK
        ///GuiStreamingCallbackCtx: void*
        ///GuiStateCallback: BioAPI_GUI_STATE_CALLBACK
        ///GuiStateCallbackCtx: void*
        [DllImportAttribute(DLLFilename, EntryPoint = "BioAPI_SetGUICallbacks", CallingConvention = CallingConvention.StdCall)]
        public static extern uint BioAPI_SetGUICallbacks(uint BSPHandle, BioAPI.GUI_STREAMING_CALLBACK GuiStreamingCallback, System.IntPtr GuiStreamingCallbackCtx, BioAPI.GUI_STATE_CALLBACK GuiStateCallback, System.IntPtr GuiStateCallbackCtx);


        /// Return Type: BioAPI_RETURN->uint32_t->unsigned int
        ///BSPHandle: BioAPI_HANDLE->uint32_t->unsigned int
        ///Purpose: BioAPI_BIR_PURPOSE->uint8_t->unsigned char
        ///Subtype: BioAPI_BIR_SUBTYPE->uint8_t->unsigned char
        ///OutputFormat: BioAPI_BIR_BIOMETRIC_DATA_FORMAT*
        ///CapturedBIR: BioAPI_BIR_HANDLE*
        ///Timeout: int32_t->int
        ///AuditData: BioAPI_BIR_HANDLE*
        [DllImportAttribute(DLLFilename, EntryPoint = "BioAPI_Capture", CallingConvention = CallingConvention.StdCall)]
        public static extern uint BioAPI_Capture(uint BSPHandle, byte Purpose, byte Subtype, IntPtr OutputFormat, ref int CapturedBIR, int Timeout, IntPtr AuditData);


        /// Return Type: BioAPI_RETURN->uint32_t->unsigned int
        ///BSPHandle: BioAPI_HANDLE->uint32_t->unsigned int
        ///CapturedBIR: BioAPI_INPUT_BIR*
        ///ReferenceTemplate: BioAPI_INPUT_BIR*
        ///OutputFormat: BioAPI_BIR_BIOMETRIC_DATA_FORMAT*
        ///NewTemplate: BioAPI_BIR_HANDLE*
        ///Payload: BioAPI_DATA*
        ///TemplateUUID: BioAPI_UUID*
        [DllImportAttribute(DLLFilename, EntryPoint = "BioAPI_CreateTemplate", CallingConvention = CallingConvention.StdCall)]
        public static extern uint BioAPI_CreateTemplate(uint BSPHandle, ref bioapi_input_bir CapturedBIR, IntPtr ReferenceTemplate, IntPtr OutputFormat, ref int NewTemplate, IntPtr Payload, IntPtr TemplateUUID);


        /// Return Type: BioAPI_RETURN->uint32_t->unsigned int
        ///BSPHandle: BioAPI_HANDLE->uint32_t->unsigned int
        ///CapturedBIR: BioAPI_INPUT_BIR*
        ///OutputFormat: BioAPI_BIR_BIOMETRIC_DATA_FORMAT*
        ///ProcessedBIR: BioAPI_BIR_HANDLE*
        [DllImportAttribute(DLLFilename, EntryPoint = "BioAPI_Process", CallingConvention = CallingConvention.StdCall)]
        public static extern uint BioAPI_Process(uint BSPHandle, ref bioapi_input_bir CapturedBIR, IntPtr OutputFormat, ref int ProcessedBIR);


        /// Return Type: BioAPI_RETURN->uint32_t->unsigned int
        ///BSPHandle: BioAPI_HANDLE->uint32_t->unsigned int
        ///MaxFMRRequested: BioAPI_FMR->int32_t->int
        ///ProcessedBIR: BioAPI_INPUT_BIR*
        ///ReferenceTemplate: BioAPI_INPUT_BIR*
        ///AdaptedBIR: BioAPI_BIR_HANDLE*
        ///Result: BioAPI_BOOL*
        ///FMRAchieved: BioAPI_FMR*
        ///Payload: BioAPI_DATA*
        [DllImportAttribute(DLLFilename, EntryPoint = "BioAPI_VerifyMatch", CallingConvention = CallingConvention.StdCall)]
        public static extern uint BioAPI_VerifyMatch(uint BSPHandle, int MaxFMRRequested, ref bioapi_input_bir ProcessedBIR, ref bioapi_input_bir ReferenceTemplate, IntPtr AdaptedBIR, ref byte Result, ref int FMRAchieved, IntPtr Payload);


        /// Return Type: BioAPI_RETURN->uint32_t->unsigned int
        ///BSPHandle: BioAPI_HANDLE->uint32_t->unsigned int
        ///MaxFMRRequested: BioAPI_FMR->int32_t->int
        ///ProcessedBIR: BioAPI_INPUT_BIR*
        ///Population: BioAPI_IDENTIFY_POPULATION*
        ///TotalNumberOfTemplates: uint32_t->unsigned int
        ///Binning: BioAPI_BOOL->uint8_t->unsigned char
        ///MaxNumberOfResults: uint32_t->unsigned int
        ///NumberOfResults: uint32_t*
        ///Candidates: BioAPI_CANDIDATE**
        ///Timeout: int32_t->int
        [DllImportAttribute(DLLFilename, EntryPoint = "BioAPI_IdentifyMatch", CallingConvention = CallingConvention.StdCall)]
        public static extern uint BioAPI_IdentifyMatch(uint BSPHandle, int MaxFMRRequested, ref bioapi_input_bir ProcessedBIR, ref bioapi_identify_population Population, uint TotalNumberOfTemplates, byte Binning, uint MaxNumberOfResults, ref uint NumberOfResults, ref System.IntPtr Candidates, int Timeout);


        /// Return Type: BioAPI_RETURN->uint32_t->unsigned int
        ///BSPHandle: BioAPI_HANDLE->uint32_t->unsigned int
        ///Purpose: BioAPI_BIR_PURPOSE->uint8_t->unsigned char
        ///Subtype: BioAPI_BIR_SUBTYPE->uint8_t->unsigned char
        ///OutputFormat: BioAPI_BIR_BIOMETRIC_DATA_FORMAT*
        ///ReferenceTemplate: BioAPI_INPUT_BIR*
        ///NewTemplate: BioAPI_BIR_HANDLE*
        ///Payload: BioAPI_DATA*
        ///Timeout: int32_t->int
        ///AuditData: BioAPI_BIR_HANDLE*
        ///TemplateUUID: BioAPI_UUID*
        [DllImportAttribute(DLLFilename, EntryPoint = "BioAPI_Enroll", CallingConvention = CallingConvention.StdCall)]
        public static extern uint BioAPI_Enroll(uint BSPHandle, byte Purpose, byte Subtype, IntPtr OutputFormat, IntPtr ReferenceTemplate, ref int NewTemplate, IntPtr Payload, int Timeout, IntPtr AuditData, IntPtr TemplateUUID);


        /// Return Type: BioAPI_RETURN->uint32_t->unsigned int
        ///BSPHandle: BioAPI_HANDLE->uint32_t->unsigned int
        ///MaxFMRRequested: BioAPI_FMR->int32_t->int
        ///ReferenceTemplate: BioAPI_INPUT_BIR*
        ///Subtype: BioAPI_BIR_SUBTYPE->uint8_t->unsigned char
        ///AdaptedBIR: BioAPI_BIR_HANDLE*
        ///Result: BioAPI_BOOL*
        ///FMRAchieved: BioAPI_FMR*
        ///Payload: BioAPI_DATA*
        ///Timeout: int32_t->int
        ///AuditData: BioAPI_BIR_HANDLE*
        [DllImportAttribute(DLLFilename, EntryPoint = "BioAPI_Verify", CallingConvention = CallingConvention.StdCall)]
        public static extern uint BioAPI_Verify(uint BSPHandle, int MaxFMRRequested, ref bioapi_input_bir ReferenceTemplate, byte Subtype, IntPtr AdaptedBIR, System.IntPtr Result, ref int FMRAchieved, IntPtr Payload, int Timeout, IntPtr AuditData);


        /// Return Type: BioAPI_RETURN->uint32_t->unsigned int
        ///BSPHandle: BioAPI_HANDLE->uint32_t->unsigned int
        ///MaxFMRRequested: BioAPI_FMR->int32_t->int
        ///Subtype: BioAPI_BIR_SUBTYPE->uint8_t->unsigned char
        ///Population: BioAPI_IDENTIFY_POPULATION*
        ///TotalNumberOfTemplates: uint32_t->unsigned int
        ///Binning: BioAPI_BOOL->uint8_t->unsigned char
        ///MaxNumberOfResults: uint32_t->unsigned int
        ///NumberOfResults: uint32_t*
        ///Candidates: BioAPI_CANDIDATE**
        ///Timeout: int32_t->int
        ///AuditData: BioAPI_BIR_HANDLE*
        [DllImportAttribute(DLLFilename, EntryPoint = "BioAPI_Identify", CallingConvention = CallingConvention.StdCall)]
        public static extern uint BioAPI_Identify(uint BSPHandle, int MaxFMRRequested, byte Subtype, ref bioapi_identify_population Population, uint TotalNumberOfTemplates, byte Binning, uint MaxNumberOfResults, ref uint NumberOfResults, ref System.IntPtr Candidates, int Timeout, ref int AuditData);



        /// Return Type: BioAPI_RETURN->uint32_t->unsigned int
        ///BSPHandle: BioAPI_HANDLE->uint32_t->unsigned int
        [DllImportAttribute(DLLFilename, EntryPoint = "BioAPI_Cancel", CallingConvention = CallingConvention.StdCall)]
        public static extern uint BioAPI_Cancel(uint BSPHandle);



        /// Return Type: BioAPI_RETURN->uint32_t->unsigned int
        ///Action: BioAPI_INSTALL_ACTION->uint32_t->unsigned int
        ///Error: BioAPI_INSTALL_ERROR*
        ///BSPSchema: BioAPI_BSP_SCHEMA*
        [DllImportAttribute(DLLFilename, EntryPoint = "BioAPI_Util_InstallBSP", CallingConvention = CallingConvention.StdCall)]
        public static extern uint BioAPI_Util_InstallBSP(uint Action, ref install_error Error, ref bioapi_bsp_schema BSPSchema);



    }
    internal static partial class BioAPI
    {

        public static uint Init(byte Version)
        {
            return NativeMethods.BioAPI_Init(Version);
        }

        public static uint Terminate()
        {
            return NativeMethods.BioAPI_Terminate();
        }

        public static uint GetFrameworkInfo(ref bioapi_framework_schema FrameworkSchema)
        {
            return NativeMethods.BioAPI_GetFrameworkInfo(ref FrameworkSchema);
        }


        public static uint EnumBSPs(ref System.IntPtr BSPSchemaArray, ref uint NumberOfElements)
        {
            return NativeMethods.BioAPI_EnumBSPs(ref BSPSchemaArray, ref NumberOfElements);
        }

        public static uint _EventHandler(System.IntPtr BSPUuid, uint UnitID, System.IntPtr AppNotifyCallbackCtx, ref bioapi_unit_schema UnitSchema, uint EventType)
        {
            return 0;
        }

        public static uint BSPLoad(System.IntPtr BSPUuid, BioAPI.EventHandler AppNotifyCallback, System.IntPtr AppNotifyCallbackCtx)
        {
            return NativeMethods.BioAPI_BSPLoad(BSPUuid, AppNotifyCallback, AppNotifyCallbackCtx);
        }


        public static uint BSPUnload(System.IntPtr BSPUuid, BioAPI.EventHandler AppNotifyCallback, System.IntPtr AppNotifyCallbackCtx)
        {
            return NativeMethods.BioAPI_BSPUnload(BSPUuid, AppNotifyCallback, AppNotifyCallbackCtx);
        }


        public static uint BSPAttach(System.IntPtr BSPUuid, byte Version, ref bioapi_unit_list_element UnitList, uint NumUnits, ref uint NewBSPHandle)
        {
            return NativeMethods.BioAPI_BSPAttach(BSPUuid, Version, ref UnitList, NumUnits, ref NewBSPHandle);
        }


        public static uint BSPDetach(uint BSPHandle)
        {
            return NativeMethods.BioAPI_BSPDetach(BSPHandle);
        }


        public static uint QueryUnits(System.IntPtr BSPUuid, ref System.IntPtr UnitSchemaArray, ref uint NumberOfElements)
        {
            return NativeMethods.BioAPI_QueryUnits(BSPUuid, ref UnitSchemaArray, ref NumberOfElements);
        }


        public static uint ControlUnit(uint BSPHandle, uint UnitID, uint ControlCode, ref bioapi_data InputData)
        {
            return NativeMethods.BioAPI_ControlUnit(BSPHandle, UnitID, ControlCode, ref InputData, IntPtr.Zero);
        }

        public static uint Free(System.IntPtr Ptr)
        {
            return NativeMethods.BioAPI_Free(Ptr);
        }


        public static  uint FreeBIRHandle(uint BSPHandle, int Handle)
        {
            return NativeMethods.BioAPI_FreeBIRHandle(BSPHandle, Handle);
        }


        public static  uint GetBIRFromHandle(uint BSPHandle, int Handle, ref bioapi_bir BIR)
        {
            return NativeMethods.BioAPI_GetBIRFromHandle(BSPHandle, Handle, ref BIR);
        }


        public static  uint GetHeaderFromHandle(uint BSPHandle, int Handle, ref bioapi_bir_header Header)
        {
            return NativeMethods.BioAPI_GetHeaderFromHandle(BSPHandle, Handle, ref Header);
        }


        public static uint SetGUICallbacks(uint BSPHandle, GUI_STREAMING_CALLBACK GuiStreamingCallback, System.IntPtr GuiStreamingCallbackCtx, GUI_STATE_CALLBACK GuiStateCallback, System.IntPtr GuiStateCallbackCtx)
        {
            return NativeMethods.BioAPI_SetGUICallbacks(BSPHandle, GuiStreamingCallback, GuiStreamingCallbackCtx, GuiStateCallback, GuiStateCallbackCtx);
        }


        public static  uint Capture(uint BSPHandle, byte Purpose, ref int CapturedBIR, int Timeout)
        {
            return NativeMethods.BioAPI_Capture(BSPHandle, Purpose, BioAPI.NO_SUBTYPE_AVAILABLE, IntPtr.Zero, ref CapturedBIR, Timeout, IntPtr.Zero);
        }


        public static  uint CreateTemplate(uint BSPHandle, ref bioapi_input_bir CapturedBIR, ref int NewTemplate)
        {
            return NativeMethods.BioAPI_CreateTemplate(BSPHandle, ref CapturedBIR, IntPtr.Zero, IntPtr.Zero, ref NewTemplate, IntPtr.Zero, IntPtr.Zero);
        }


        public static  uint Process(uint BSPHandle, ref bioapi_input_bir CapturedBIR, ref int ProcessedBIR)
        {
            return NativeMethods.BioAPI_Process(BSPHandle, ref CapturedBIR, IntPtr.Zero, ref ProcessedBIR);
        }


        public static  uint VerifyMatch(uint BSPHandle, int MaxFMRRequested, ref bioapi_input_bir ProcessedBIR, ref bioapi_input_bir ReferenceTemplate, ref byte Result, ref int FMRAchieved)
        {
            return NativeMethods.BioAPI_VerifyMatch(BSPHandle, MaxFMRRequested, ref ProcessedBIR, ref ReferenceTemplate, IntPtr.Zero, ref Result, ref FMRAchieved, IntPtr.Zero);
        }

        public static  uint IdentifyMatch(uint BSPHandle, int MaxFMRRequested, ref bioapi_input_bir ProcessedBIR, ref bioapi_identify_population Population, uint TotalNumberOfTemplates, uint MaxNumberOfResults, ref uint NumberOfResults, ref System.IntPtr Candidates, int Timeout)
        {
            return NativeMethods.BioAPI_IdentifyMatch(BSPHandle, MaxFMRRequested, ref ProcessedBIR, ref Population, TotalNumberOfTemplates, 0, MaxNumberOfResults, ref NumberOfResults, ref Candidates, Timeout);
        }

        public static  uint Cancel(uint BSPHandle)
        {
            return NativeMethods.BioAPI_Cancel(BSPHandle);
        }

        public static  uint Util_InstallBSP(uint Action, ref install_error Error, ref bioapi_bsp_schema BSPSchema)
        {
            return NativeMethods.BioAPI_Util_InstallBSP(Action, ref Error, ref BSPSchema);
        }

        public static bioapi_bsp_schema BSPInfo(IntPtr BSPSchemaArray, int index)
        {
            return (bioapi_bsp_schema)Marshal.PtrToStructure(BSPSchemaArray, typeof(bioapi_bsp_schema));
        }
    }

}

