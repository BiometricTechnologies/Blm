using System;
using System.Runtime.InteropServices;
//using System.Runtime.Serialization;
//using System.Runtime.Serialization.Formatters.Binary;
namespace Hitachi
{
    internal partial class BioAPI
    {

        /// BioAPI_OK -> (0)
        public const int OK = 0;

        ///// Warning: Generation of Method Macros is not supported at this time
        ///// FALSE -> "(BioAPI_BOOL) (0)"
        //public const string FALSE = "(BioAPI_BOOL) (0)";

        ///// Warning: Generation of Method Macros is not supported at this time
        ///// TRUE -> "(BioAPI_BOOL) (!BioAPI_FALSE)"
        //public const string TRUE = "(BioAPI_BOOL) (!BioAPI_FALSE)";

        /// DB_ACCESS_READ -> (0x00000001)
        public const int DB_ACCESS_READ = 1;

        /// DB_ACCESS_WRITE -> (0x00000002)
        public const int DB_ACCESS_WRITE = 2;

        /// CATEGORY_ARCHIVE -> (0x00000001)
        public const int CATEGORY_ARCHIVE = 1;

        /// CATEGORY_MATCHING_ALG -> (0x00000002)
        public const int CATEGORY_MATCHING_ALG = 2;

        /// CATEGORY_PROCESSING_ALG -> (0x00000004)
        public const int CATEGORY_PROCESSING_ALG = 4;

        /// CATEGORY_SENSOR -> (0x00000008)
        public const int CATEGORY_SENSOR = 8;

        /// BIR_DATA_TYPE_RAW -> (0x01)
        public const int BIR_DATA_TYPE_RAW = 1;

        /// BIR_DATA_TYPE_INTERMEDIATE -> (0x02)
        public const int BIR_DATA_TYPE_INTERMEDIATE = 2;

        /// BIR_DATA_TYPE_PROCESSED -> (0x04)
        public const int BIR_DATA_TYPE_PROCESSED = 4;

        /// BIR_DATA_TYPE_ENCRYPTED -> (0x10)
        public const int BIR_DATA_TYPE_ENCRYPTED = 16;

        /// BIR_DATA_TYPE_SIGNED -> (0x20)
        public const int BIR_DATA_TYPE_SIGNED = 32;

        /// BIR_INDEX_PRESENT -> (0x80)
        public const int BIR_INDEX_PRESENT = 128;

        /// INVALID_BIR_HANDLE -> (-1)
        public const int INVALID_BIR_HANDLE = -1;

        /// UNSUPPORTED_BIR_HANDLE -> (-2)
        public const int UNSUPPORTED_BIR_HANDLE = -2;

        /// PURPOSE_VERIFY -> (1)
        public const int PURPOSE_VERIFY = 1;

        /// PURPOSE_IDENTIFY -> (2)
        public const int PURPOSE_IDENTIFY = 2;

        /// PURPOSE_ENROLL -> (3)
        public const int PURPOSE_ENROLL = 3;

        /// PURPOSE_ENROLL_FOR_VERIFICATION_ONLY -> (4)
        public const int PURPOSE_ENROLL_FOR_VERIFICATION_ONLY = 4;

        /// PURPOSE_ENROLL_FOR_IDENTIFICATION_ONLY -> (5)
        public const int PURPOSE_ENROLL_FOR_IDENTIFICATION_ONLY = 5;

        /// PURPOSE_AUDIT -> (6)
        public const int PURPOSE_AUDIT = 6;

        /// NO_PURPOSE_AVAILABLE -> (0)
        public const int NO_PURPOSE_AVAILABLE = 0;

        /// NO_TYPE_AVAILABLE -> (0x00000000)
        public const int NO_TYPE_AVAILABLE = 0;

        /// TYPE_MULTIPLE -> (0x00000001)
        public const int TYPE_MULTIPLE = 1;

        /// TYPE_FACIAL_FEATURES -> (0x00000002)
        public const int TYPE_FACIAL_FEATURES = 2;

        /// TYPE_VOICE -> (0x00000004)
        public const int TYPE_VOICE = 4;

        /// TYPE_FINGERPRINT -> (0x00000008)
        public const int TYPE_FINGERPRINT = 8;

        /// TYPE_IRIS -> (0x00000010)
        public const int TYPE_IRIS = 16;

        /// TYPE_RETINA -> (0x00000020)
        public const int TYPE_RETINA = 32;

        /// TYPE_HAND_GEOMETRY -> (0x00000040)
        public const int TYPE_HAND_GEOMETRY = 64;

        /// TYPE_SIGNATURE_DYNAMICS -> (0x00000080)
        public const int TYPE_SIGNATURE_DYNAMICS = 128;

        /// TYPE_KEYSTOKE_DYNAMICS -> (0x00000100)
        public const int TYPE_KEYSTOKE_DYNAMICS = 256;

        /// TYPE_LIP_MOVEMENT -> (0x00000200)
        public const int TYPE_LIP_MOVEMENT = 512;

        /// TYPE_THERMAL_FACE_IMAGE -> (0x00000400)
        public const int TYPE_THERMAL_FACE_IMAGE = 1024;

        /// TYPE_THERMAL_HAND_IMAGE -> (0x00000800)
        public const int TYPE_THERMAL_HAND_IMAGE = 2048;

        /// TYPE_GAIT -> (0x00001000)
        public const int TYPE_GAIT = 4096;

        /// TYPE_OTHER -> (0x40000000)
        public const int TYPE_OTHER = 1073741824;

        /// TYPE_PASSWORD -> (0x80000000)
        public const int TYPE_PASSWORD = -2147483648;

        /// BIR_SUBTYPE_LEFT -> (0x01)
        public const int BIR_SUBTYPE_LEFT = 1;

        /// BIR_SUBTYPE_RIGHT -> (0x02)
        public const int BIR_SUBTYPE_RIGHT = 2;

        /// BIR_SUBTYPE_THUMB -> (0x04)
        public const int BIR_SUBTYPE_THUMB = 4;

        /// BIR_SUBTYPE_POINTERFINGER -> (0x08)
        public const int BIR_SUBTYPE_POINTERFINGER = 8;

        /// BIR_SUBTYPE_MIDDLEFINGER -> (0x10)
        public const int BIR_SUBTYPE_MIDDLEFINGER = 16;

        /// BIR_SUBTYPE_RINGFINGER -> (0x20)
        public const int BIR_SUBTYPE_RINGFINGER = 32;

        /// BIR_SUBTYPE_LITTLEFINGER -> (0x40)
        public const int BIR_SUBTYPE_LITTLEFINGER = 64;

        /// BIR_SUBTYPE_MULTIPLE -> (0x80)
        public const int BIR_SUBTYPE_MULTIPLE = 128;

        /// NO_SUBTYPE_AVAILABLE -> (0x00)
        public const int NO_SUBTYPE_AVAILABLE = 0;

        /// DB_TYPE -> (1)
        public const int DB_TYPE = 1;

        /// ARRAY_TYPE -> (2)
        public const int ARRAY_TYPE = 2;

        /// PRESET_ARRAY_TYPE -> (3)
        public const int PRESET_ARRAY_TYPE = 3;

        /// DB_INVALID_HANDLE -> (-1)
        public const int DB_INVALID_HANDLE = -1;

        /// DB_DEFAULT_HANDLE -> (0)
        public const int DB_DEFAULT_HANDLE = 0;

        /// DB_DEFAULT_UUID_PTR -> (NULL)
        public const int DB_DEFAULT_UUID_PTR = 0;

        /// NOTIFY_INSERT -> (1)
        public const int NOTIFY_INSERT = 1;

        /// NOTIFY_REMOVE -> (2)
        public const int NOTIFY_REMOVE = 2;

        /// NOTIFY_FAULT -> (3)
        public const int NOTIFY_FAULT = 3;

        /// NOTIFY_SOURCE_PRESENT -> (4)
        public const int NOTIFY_SOURCE_PRESENT = 4;

        /// NOTIFY_SOURCE_REMOVED -> (5)
        public const int NOTIFY_SOURCE_REMOVED = 5;

        /// NOTIFY_INSERT_BIT -> (0x00000001)
        public const int NOTIFY_INSERT_BIT = 1;

        /// NOTIFY_REMOVE_BIT -> (0x00000002)
        public const int NOTIFY_REMOVE_BIT = 2;

        /// NOTIFY_FAULT_BIT -> (0x00000004)
        public const int NOTIFY_FAULT_BIT = 4;

        /// NOTIFY_SOURCE_PRESENT_BIT -> (0x00000008)
        public const int NOTIFY_SOURCE_PRESENT_BIT = 8;

        /// NOTIFY_SOURCE_REMOVED_BIT -> (0x00000010)
        public const int NOTIFY_SOURCE_REMOVED_BIT = 16;

        /// DONT_CARE -> (0x00000000)
        public const int DONT_CARE = 0;

        /// DONT_INCLUDE -> (0xFFFFFFFF)
        public const int DONT_INCLUDE = -1;

        /// CAPTURE_SAMPLE -> (1)
        public const int CAPTURE_SAMPLE = 1;

        /// CANCEL -> (2)
        public const int CANCEL = 2;

        /// CONTINUE -> (3)
        public const int CONTINUE = 3;

        /// VALID_SAMPLE -> (4)
        public const int VALID_SAMPLE = 4;

        /// INVALID_SAMPLE -> (5)
        public const int INVALID_SAMPLE = 5;

        /// SAMPLE_AVAILABLE -> (0x0001)
        public const int SAMPLE_AVAILABLE = 1;

        /// MESSAGE_PROVIDED -> (0x0002)
        public const int MESSAGE_PROVIDED = 2;

        /// PROGRESS_PROVIDED -> (0x0004)
        public const int PROGRESS_PROVIDED = 4;

        /// INVALID_BSP_HANDLE -> (0)
        public const int INVALID_BSP_HANDLE = 0;

        /// INDICATOR_ACCEPT -> (1)
        public const int INDICATOR_ACCEPT = 1;

        /// INDICATOR_REJECT -> (2)
        public const int INDICATOR_REJECT = 2;

        /// INDICATOR_READY -> (3)
        public const int INDICATOR_READY = 3;

        /// INDICATOR_BUSY -> (4)
        public const int INDICATOR_BUSY = 4;

        /// INDICATOR_FAILURE -> (5)
        public const int INDICATOR_FAILURE = 5;

        /// DATABASE_ID_INPUT -> (1)
        public const int DATABASE_ID_INPUT = 1;

        /// BIR_HANDLE_INPUT -> (2)
        public const int BIR_HANDLE_INPUT = 2;

        /// FULLBIR_INPUT -> (3)
        public const int FULLBIR_INPUT = 3;

        /// INSTALL_ACTION_INSTALL -> (1)
        public const int INSTALL_ACTION_INSTALL = 1;

        /// INSTALL_ACTION_REFRESH -> (2)
        public const int INSTALL_ACTION_REFRESH = 2;

        /// INSTALL_ACTION_UNINSTALL -> (3)
        public const int INSTALL_ACTION_UNINSTALL = 3;

        /// ENABLEEVENTS -> (0x00000001)
        public const int ENABLEEVENTS = 1;

        /// SETGUICALLBACKS -> (0x00000002)
        public const int SETGUICALLBACKS = 2;

        /// CAPTURE -> (0x00000004)
        public const int CAPTURE = 4;

        /// CREATETEMPLATE -> (0x00000008)
        public const int CREATETEMPLATE = 8;

        /// PROCESS -> (0x00000010)
        public const int PROCESS = 16;

        /// PROCESSWITHAUXBIR -> (0x00000020)
        public const int PROCESSWITHAUXBIR = 32;

        /// VERIFYMATCH -> (0x00000040)
        public const int VERIFYMATCH = 64;

        /// IDENTIFYMATCH -> (0x00000080)
        public const int IDENTIFYMATCH = 128;

        /// ENROLL -> (0x00000100)
        public const int ENROLL = 256;

        /// VERIFY -> (0x00000200)
        public const int VERIFY = 512;

        /// IDENTIFY -> (0x00000400)
        public const int IDENTIFY = 1024;

        /// IMPORT -> (0x00000800)
        public const int IMPORT = 2048;

        /// PRESETIDENTIFYPOPULATION -> (0x00001000)
        public const int PRESETIDENTIFYPOPULATION = 4096;

        /// DATABASEOPERATIONS -> (0x00002000)
        public const int DATABASEOPERATIONS = 8192;

        /// SETPOWERMODE -> (0x00004000)
        public const int SETPOWERMODE = 16384;

        /// SETINDICATORSTATUS -> (0x00008000)
        public const int SETINDICATORSTATUS = 32768;

        /// GETINDICATORSTATUS -> (0x00010000)
        public const int GETINDICATORSTATUS = 65536;

        /// CALIBRATESENSOR -> (0x00020000)
        public const int CALIBRATESENSOR = 131072;

        /// UTILITIES -> (0X00040000)
        public const int UTILITIES = 262144;

        /// QUERYUNITS -> (0x00100000)
        public const int QUERYUNITS = 1048576;

        /// QUERYBFPS -> (0x00200000)
        public const int QUERYBFPS = 2097152;

        /// CONTROLUNIT -> (0X00400000)
        public const int CONTROLUNIT = 4194304;

        /// RAW -> (0x00000001)
        public const int RAW = 1;

        /// QUALITY_RAW -> (0x00000002)
        public const int QUALITY_RAW = 2;

        /// QUALITY_INTERMEDIATE -> (0x00000004)
        public const int QUALITY_INTERMEDIATE = 4;

        /// QUALITY_PROCESSED -> (0x00000008)
        public const int QUALITY_PROCESSED = 8;

        /// APP_GUI -> (0x00000010)
        public const int APP_GUI = 16;

        /// STREAMINGDATA -> (0x00000020)
        public const int STREAMINGDATA = 32;

        /// SOURCEPRESENT -> (0x00000040)
        public const int SOURCEPRESENT = 64;

        /// PAYLOAD -> (0x00000080)
        public const int PAYLOAD = 128;

        /// BIR_SIGN -> (0x00000100)
        public const int BIR_SIGN = 256;

        /// BIR_ENCRYPT -> (0x00000200)
        public const int BIR_ENCRYPT = 512;

        /// TEMPLATEUPDATE -> (0x00000400)
        public const int TEMPLATEUPDATE = 1024;

        /// ADAPTATION -> (0x00000800)
        public const int ADAPTATION = 2048;

        /// BINNING -> (0x00001000)
        public const int BINNING = 4096;

        /// SELFCONTAINEDDEVICE -> (0x00002000)
        public const int SELFCONTAINEDDEVICE = 8192;

        /// MOC -> (0x00004000)
        public const int MOC = 16384;

        /// SUBTYPE_TO_CAPTURE -> (0x00008000)
        public const int SUBTYPE_TO_CAPTURE = 32768;

        /// SENSORBFP -> (0x00010000)
        public const int SENSORBFP = 65536;

        /// ARCHIVEBFP -> (0x00020000)
        public const int ARCHIVEBFP = 131072;

        /// MATCHINGBFP -> (0x00040000)
        public const int MATCHINGBFP = 262144;

        /// PROCESSINGBFP -> (0x00080000)
        public const int PROCESSINGBFP = 524288;

        /// COARSESCORES -> (0x00100000)
        public const int COARSESCORES = 1048576;

        /// POWER_NORMAL -> (1)
        public const int POWER_NORMAL = 1;

        /// POWER_DETECT -> (2)
        public const int POWER_DETECT = 2;

        /// POWER_SLEEP -> (3)
        public const int POWER_SLEEP = 3;

        /// NO_PRODUCT_OWNER_AVAILABLE -> (0x0000)
        public const int NO_PRODUCT_OWNER_AVAILABLE = 0;

        /// NO_PRODUCT_TYPE_AVAILABLE -> (0x0000)
        public const int NO_PRODUCT_TYPE_AVAILABLE = 0;

        /// NO_YEAR_AVAILABLE -> (0)
        public const int NO_YEAR_AVAILABLE = 0;

        /// NO_MONTH_AVAILABLE -> (0)
        public const int NO_MONTH_AVAILABLE = 0;

        /// NO_DAY_AVAILABLE -> (0)
        public const int NO_DAY_AVAILABLE = 0;

        /// NO_HOUR_AVAILABLE -> (99)
        public const int NO_HOUR_AVAILABLE = 99;

        /// NO_MINUTE_AVAILABLE -> (99)
        public const int NO_MINUTE_AVAILABLE = 99;

        /// NO_SECOND_AVAILABLE -> (99)
        public const int NO_SECOND_AVAILABLE = 99;

        /// NOT_SET -> (-1)
        public const int NOT_SET = -1;

        /// NULL -> ((void *)0)
        /// Error generating expression: Expression is not parsable.  Treating value as a raw string
        [UnmanagedFunctionPointerAttribute(CallingConvention.StdCall)]
        public delegate uint GUI_STATE_CALLBACK(System.IntPtr GuiStateCallbackCtx, uint GuiState, System.IntPtr Response, uint Message, byte Progress, IntPtr SampleBuffer);

        /// Return Type: BioAPI_RETURN->uint32_t->unsigned int
        ///GuiStreamingCallbackCtx: void*
        ///Bitmap: BioAPI_GUI_BITMAP*
        [UnmanagedFunctionPointerAttribute(CallingConvention.StdCall)]
        public delegate uint GUI_STREAMING_CALLBACK(System.IntPtr GuiStreamingCallbackCtx, IntPtr Bitmap);

        /// Return Type: BioAPI_RETURN->uint32_t->unsigned int
        ///BSPUuid: BioAPI_UUID*
        ///UnitID: BioAPI_UNIT_ID->uint32_t->unsigned int
        ///AppNotifyCallbackCtx: void*
        ///UnitSchema: BioAPI_UNIT_SCHEMA*
        ///EventType: BioAPI_EVENT->uint32_t->unsigned int
        [UnmanagedFunctionPointerAttribute(CallingConvention.StdCall)]
        public delegate uint EventHandler(System.IntPtr BSPUuid, uint UnitID, System.IntPtr AppNotifyCallbackCtx, ref bioapi_unit_schema UnitSchema, uint EventType);

    }
    
    [StructLayoutAttribute(LayoutKind.Sequential)]
    public struct bioapi_bir_biometric_data_format
    {

        /// uint16_t->unsigned short
        public ushort FormatOwner;

        /// uint16_t->unsigned short
        public ushort FormatType;
    }
    
    [StructLayoutAttribute(LayoutKind.Sequential)]
    public struct bioapi_bir_biometric_product_ID
    {

        /// uint16_t->unsigned short
        public ushort ProductOwner;

        /// uint16_t->unsigned short
        public ushort ProductType;
    }

    
    [StructLayoutAttribute(LayoutKind.Sequential)]
    public struct bioapi_date
    {

        /// uint16_t->unsigned short
        public ushort Year;

        /// uint8_t->unsigned char
        public byte Month;

        /// uint8_t->unsigned char
        public byte Day;
    }
    
    
    [StructLayoutAttribute(LayoutKind.Sequential)]
    public struct bioapi_time
    {

        /// uint8_t->unsigned char
        public byte Hour;

        /// uint8_t->unsigned char
        public byte Minute;

        /// uint8_t->unsigned char
        public byte Second;
    }
    
    
    [StructLayoutAttribute(LayoutKind.Sequential)]
    public struct bioapi_DTG
    {

        /// BioAPI_DATE->bioapi_date
        public bioapi_date Date;

        /// BioAPI_TIME->bioapi_time
        public bioapi_time Time;
    }

    [StructLayoutAttribute(LayoutKind.Sequential)]
    public struct bioapi_bir_security_block_format
    {

        /// uint16_t->unsigned short
        public ushort SecurityFormatOwner;

        /// uint16_t->unsigned short
        public ushort SecurityFormatType;
    }

    [StructLayoutAttribute(LayoutKind.Sequential, CharSet = CharSet.Ansi)]
    public struct bioapi_bir_header
    {

        /// BioAPI_VERSION->uint8_t->unsigned char
        public byte HeaderVersion;

        /// BioAPI_BIR_DATA_TYPE->uint8_t->unsigned char
        public byte Type;

        /// BioAPI_BIR_BIOMETRIC_DATA_FORMAT->bioapi_bir_biometric_data_format
        public bioapi_bir_biometric_data_format Format;

        /// BioAPI_QUALITY->int8_t->char
        public byte Quality;

        /// BioAPI_BIR_PURPOSE->uint8_t->unsigned char
        public byte Purpose;

        /// BioAPI_BIR_BIOMETRIC_TYPE->uint32_t->unsigned int
        public uint FactorsMask;

        /// BioAPI_BIR_BIOMETRIC_PRODUCT_ID->bioapi_bir_biometric_product_ID
        public bioapi_bir_biometric_product_ID ProductID;

        /// BioAPI_DTG->bioapi_DTG
        public bioapi_DTG CreationDTG;

        /// BioAPI_BIR_SUBTYPE->uint8_t->unsigned char
        public byte Subtype;

        /// BioAPI_DATE->bioapi_date
        public bioapi_date ExpirationDate;

        /// BioAPI_BIR_SECURITY_BLOCK_FORMAT->bioapi_bir_security_block_format
        public bioapi_bir_security_block_format SBFormat;

        /// BioAPI_UUID->uint8_t[16]
        [MarshalAsAttribute(UnmanagedType.ByValArray, SizeConst = 16)]
        public byte[] Index;
    }

    [StructLayoutAttribute(LayoutKind.Sequential)]
    public struct bioapi_data
    {

        /// uint32_t->unsigned int
        public uint Length;

        /// void*
        public System.IntPtr Data;
    }

    [StructLayoutAttribute(LayoutKind.Sequential)]
    public struct bioapi_bir
    {

        /// BioAPI_BIR_HEADER->bioapi_bir_header
        public bioapi_bir_header Header;

        /// BioAPI_DATA->bioapi_data
        public bioapi_data BiometricData;

        /// BioAPI_DATA->bioapi_data
        public bioapi_data SecurityBlock;
        
        ///
        public void FreeData()
        {
            BioAPI.Free(BiometricData.Data);
            BioAPI.Free(SecurityBlock.Data);
        }
    }

    [StructLayoutAttribute(LayoutKind.Sequential)]
    public struct bioapi_bir_array_population
    {

        /// uint32_t->unsigned int
        public uint NumberOfMembers;

        /// BioAPI_BIR*
        public System.IntPtr Members;
    }

    [StructLayoutAttribute(LayoutKind.Explicit)]
    public struct Anonymous_e86bb34b_0285_41a3_8599_bea9ac47a8c8
    {

        /// BioAPI_DB_HANDLE*
        [FieldOffsetAttribute(0)]
        public System.IntPtr BIRDataBase;

        /// BioAPI_BIR_ARRAY_POPULATION*
        [FieldOffsetAttribute(0)]
        public System.IntPtr BIRArray;
    }

    [StructLayoutAttribute(LayoutKind.Sequential)]
    public struct bioapi_identify_population
    {

        /// BioAPI_IDENTIFY_POPULATION_TYPE->uint8_t->unsigned char
        public byte Type;

        /// Anonymous_e86bb34b_0285_41a3_8599_bea9ac47a8c8
        public Anonymous_e86bb34b_0285_41a3_8599_bea9ac47a8c8 BIRs;
    }

    [StructLayoutAttribute(LayoutKind.Explicit)]
    public struct Anonymous_37142247_88d1_4a4b_ad9e_12f9b2a91032
    {

        /// BioAPI_UUID*
        [FieldOffsetAttribute(0)]
        public System.IntPtr BIRInDataBase;

        /// uint32_t*
        [FieldOffsetAttribute(0)]
        public System.IntPtr BIRInArray;
    }

    [StructLayoutAttribute(LayoutKind.Sequential)]
    public struct bioapi_candidate
    {

        /// BioAPI_IDENTIFY_POPULATION_TYPE->uint8_t->unsigned char
        public byte Type;

        /// Anonymous_37142247_88d1_4a4b_ad9e_12f9b2a91032
        public Anonymous_37142247_88d1_4a4b_ad9e_12f9b2a91032 BIR;

        /// BioAPI_FMR->int32_t->int
        public int FMRAchieved;
    }

    [StructLayoutAttribute(LayoutKind.Sequential, CharSet = CharSet.Ansi)]
    public struct bioapi_dbbir_id
    {

        /// BioAPI_DB_HANDLE->int32_t->int
        public int DbHandle;

        /// BioAPI_UUID->uint8_t[16]
        [MarshalAsAttribute(UnmanagedType.ByValTStr, SizeConst = 16)]
        public string KeyValue;
    }

    [StructLayoutAttribute(LayoutKind.Sequential)]
    public struct bioapi_gui_bitmap
    {

        /// uint32_t->unsigned int
        public uint Width;

        /// uint32_t->unsigned int
        public uint Height;

        /// BioAPI_DATA->bioapi_data
        public bioapi_data Bitmap;
    }

    /// Return Type: BioAPI_RETURN->uint32_t->unsigned int
    ///GuiStateCallbackCtx: void*
    ///GuiState: BioAPI_GUI_STATE->uint32_t->unsigned int
    ///Response: BioAPI_GUI_RESPONSE*
    ///Message: BioAPI_GUI_MESSAGE->uint32_t->unsigned int
    ///Progress: BioAPI_GUI_PROGRESS->uint8_t->unsigned char
    ///SampleBuffer: BioAPI_GUI_BITMAP*

    [StructLayoutAttribute(LayoutKind.Explicit)]
    public struct Anonymous_4db79c9b_656c_4f07_8580_34e1535e94d1
    {

        /// BioAPI_DBBIR_ID*
        [FieldOffsetAttribute(0)]
        public System.IntPtr BIRinDb;

        /// BioAPI_BIR_HANDLE*
        [FieldOffsetAttribute(0)]
        public System.IntPtr BIRinBSP;

        /// BioAPI_BIR*
        [FieldOffsetAttribute(0)]
        public System.IntPtr BIR;
    }

    [StructLayoutAttribute(LayoutKind.Sequential)]
    public struct bioapi_input_bir
    {

        /// BioAPI_INPUT_BIR_FORM->uint8_t->unsigned char
        public byte Form;

        /// Anonymous_4db79c9b_656c_4f07_8580_34e1535e94d1
        public Anonymous_4db79c9b_656c_4f07_8580_34e1535e94d1 InputBIR;
    }

    [StructLayoutAttribute(LayoutKind.Sequential, CharSet = CharSet.Ansi)]
    public struct install_error
    {

        /// BioAPI_RETURN->uint32_t->unsigned int
        public uint ErrorCode;

        /// BioAPI_STRING->uint8_t[269]
        [MarshalAsAttribute(UnmanagedType.ByValTStr, SizeConst = 269)]
        public string ErrorString;
    }

    [StructLayoutAttribute(LayoutKind.Sequential)]
    public struct bioapi_unit_list_element
    {

        /// BioAPI_CATEGORY->uint32_t->unsigned int
        public uint UnitCategory;

        /// BioAPI_UNIT_ID->uint32_t->unsigned int
        public uint UnitId;
    }


    [StructLayoutAttribute(LayoutKind.Sequential, CharSet = CharSet.Ansi)]
    public struct bioapi_framework_schema
    {

        /// BioAPI_UUID->uint8_t[16]
        [MarshalAsAttribute(UnmanagedType.ByValTStr, SizeConst = 16)]
        public string FrameworkUuid;

        /// BioAPI_STRING->uint8_t[269]
        [MarshalAsAttribute(UnmanagedType.ByValTStr, SizeConst = 269)]
        public string FwDescription;

        /// uint8_t*
        public System.IntPtr Path;

        /// BioAPI_VERSION->uint8_t->unsigned char
        public byte SpecVersion;

        /// BioAPI_STRING->uint8_t[269]
        [MarshalAsAttribute(UnmanagedType.ByValTStr, SizeConst = 269)]
        public string ProductVersion;

        /// BioAPI_STRING->uint8_t[269]
        [MarshalAsAttribute(UnmanagedType.ByValTStr, SizeConst = 269)]
        public string Vendor;

        /// BioAPI_UUID->uint8_t[16]
        [MarshalAsAttribute(UnmanagedType.ByValTStr, SizeConst = 16)]
        public string FwPropertyId;

        /// BioAPI_DATA->bioapi_data
        public bioapi_data FwProperty;
    }

    [StructLayoutAttribute(LayoutKind.Sequential, CharSet = CharSet.Ansi)]
    public struct bioapi_bsp_schema
    {

        /// BioAPI_UUID->uint8_t[16]
        [MarshalAsAttribute(UnmanagedType.ByValArray, SizeConst = 16)]
        public byte[] BSPUuid;

        /// BioAPI_STRING->uint8_t[269]
        [MarshalAsAttribute(UnmanagedType.ByValTStr, SizeConst = 269)]
        public string BSPDescription;

        /// uint8_t*
        public System.IntPtr Path;

        /// BioAPI_VERSION->uint8_t->unsigned char
        public byte SpecVersion;

        /// BioAPI_STRING->uint8_t[269]
        [MarshalAsAttribute(UnmanagedType.ByValTStr, SizeConst = 269)]
        public string ProductVersion;

        /// BioAPI_STRING->uint8_t[269]
        [MarshalAsAttribute(UnmanagedType.ByValTStr, SizeConst = 269)]
        public string Vendor;

        /// BioAPI_BIR_BIOMETRIC_DATA_FORMAT*
        public System.IntPtr BSPSupportedFormats;

        /// uint32_t->unsigned int
        public uint NumSupportedFormats;

        /// BioAPI_BIR_BIOMETRIC_TYPE->uint32_t->unsigned int
        public uint FactorsMask;

        /// BioAPI_OPERATIONS_MASK->uint32_t->unsigned int
        public uint Operations;

        /// BioAPI_OPTIONS_MASK->uint32_t->unsigned int
        public uint Options;

        /// BioAPI_FMR->int32_t->int
        public int PayloadPolicy;

        /// uint32_t->unsigned int
        public uint MaxPayloadSize;

        /// int32_t->int
        public int DefaultVerifyTimeout;

        /// int32_t->int
        public int DefaultIdentifyTimeout;

        /// int32_t->int
        public int DefaultCaptureTimeout;

        /// int32_t->int
        public int DefaultEnrollTimeout;

        /// int32_t->int
        public int DefaultCalibrateTimeout;

        /// uint32_t->unsigned int
        public uint MaxBSPDbSize;

        /// uint32_t->unsigned int
        public uint MaxIdentify;
    }

    [StructLayoutAttribute(LayoutKind.Sequential, CharSet = CharSet.Ansi)]
    public struct bioapi_unit_schema
    {

        /// BioAPI_UUID->uint8_t[16]
        [MarshalAsAttribute(UnmanagedType.ByValArray, SizeConst = 16)]
        public byte[] BSPUuid;

        /// BioAPI_UUID->uint8_t[16]
        [MarshalAsAttribute(UnmanagedType.ByValTStr, SizeConst = 16)]
        public string UnitManagerUuid;

        /// BioAPI_UNIT_ID->uint32_t->unsigned int
        public uint UnitId;

        /// BioAPI_CATEGORY->uint32_t->unsigned int
        public uint UnitCategory;

        /// BioAPI_UUID->uint8_t[16]
        [MarshalAsAttribute(UnmanagedType.ByValTStr, SizeConst = 16)]
        public string UnitProperties;

        /// BioAPI_STRING->uint8_t[269]
        [MarshalAsAttribute(UnmanagedType.ByValTStr, SizeConst = 269)]
        public string VendorInformation;

        /// uint32_t->unsigned int
        public uint SupportedEvents;

        /// BioAPI_UUID->uint8_t[16]
        [MarshalAsAttribute(UnmanagedType.ByValTStr, SizeConst = 16)]
        public string UnitPropertyID;

        /// BioAPI_DATA->bioapi_data
        public bioapi_data UnitProperty;

        /// BioAPI_STRING->uint8_t[269]
        [MarshalAsAttribute(UnmanagedType.ByValTStr, SizeConst = 269)]
        public string HardwareVersion;

        /// BioAPI_STRING->uint8_t[269]
        [MarshalAsAttribute(UnmanagedType.ByValTStr, SizeConst = 269)]
        public string FirmwareVersion;

        /// BioAPI_STRING->uint8_t[269]
        [MarshalAsAttribute(UnmanagedType.ByValTStr, SizeConst = 269)]
        public string SoftwareVersion;

        /// BioAPI_STRING->uint8_t[269]
        [MarshalAsAttribute(UnmanagedType.ByValTStr, SizeConst = 269)]
        public string HardwareSerialNumber;

        /// BioAPI_BOOL->uint8_t->unsigned char
        public byte AuthenticatedHardware;

        /// uint32_t->unsigned int
        public uint MaxBSPDbSize;

        /// uint32_t->unsigned int
        public uint MaxIdentify;
    }


}