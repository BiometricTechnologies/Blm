using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Hitachi
{
    internal static partial class BioAPI
    {
        public enum Error:uint
        {
            // BioAPI Error Value Constants
            FRAMEWORK_ERROR = 0x00000000,
            BSP_ERROR = 0x01000000,
            UNIT_ERROR = 0x02000000,

            // General Error Codes
            INTERNAL_ERROR = 0x000101,
            MEMORY_ERROR = 0x000102,
            INVALID_POINTER = 0x000103,
            INVALID_INPUT_POINTER = 0x000104,
            INVALID_OUTPUT_POINTER = 0x000105,
            FUNCTION_NOT_SUPPORTED = 0x000106,
            OS_ACCESS_DENIED = 0x000107,
            FUNCTION_FAILED = 0x000108,
            INVALID_UUID = 0x000109,
            INCOMPATIBLE_VERSION = 0x00010a,
            INVALID_DATA = 0x00010b,
            UNABLE_TO_CAPTURE = 0x00010c,
            TOO_MANY_HANDLES = 0x00010d,
            TIMEOUT_EXPIRED = 0x00010e,
            INVALID_BIR = 0x00010f,
            BIR_SIGNATURE_FAILURE = 0x000110,
            UNABLE_TO_STORE_PAYLOAD = 0x000111,
            NO_INPUT_BIRS = 0x000112,
            UNSUPPORTED_FORMAT = 0x000113,
            UNABLE_TO_IMPORT = 0x000114,
            INCONSISTENT_PURPOSE = 0x000115,
            BIR_NOT_FULLY_PROCESSED = 0x000116,
            PURPOSE_NOT_SUPPORTED = 0x000117,
            USER_CANCELLED = 0x000118,
            UNIT_IN_USE = 0x000119,
            INVALID_BSP_HANDLE = 0x00011a,
            FRAMEWORK_NOT_INITIALIZED = 0x00011b,
            INVALID_BIR_HANDLE = 0x00011c,
            CALIBRATION_NOT_SUCCESSFUL = 0x00011d,
            PRESET_BIR_DOES_NOT_EXIST = 0x00011e,
            BIR_DECRYPTION_FAILURE = 0x00011f,

            // Component Management Error Codes
            COMPONENT_FILE_REF_NOT_FOUND = 0x000201,
            BSP_LOAD_FAIL = 0x000202,
            BSP_NOT_LOADED = 0x000203,
            UNIT_NOT_INSERTED = 0x000204,
            INVALID_UNIT_ID = 0x000205,
            INVALID_CATEGORY = 0x000206,

            // Database Error Values
            INVALID_DB_HANDLE = 0x000300,
            UNABLE_TO_OPEN_DATABASE = 0x000301,
            DATABASE_IS_LOCKED = 0x000302,
            DATABASE_DOES_NOT_EXIST = 0x000303,
            DATABASE_ALREADY_EXISTS = 0x000304,
            INVALID_DATABASE_NAME = 0x000305,
            RECORD_NOT_FOUND = 0x000306,
            MARKER_HANDLE_IS_INVALID = 0x000307,
            DATABASE_IS_OPEN = 0x000308,
            INVALID_ACCESS_REQUEST = 0x000309,
            END_OF_DATABASE = 0x00030a,
            UNABLE_TO_CREATE_DATABASE = 0x00030b,
            UNABLE_TO_CLOSE_DATABASE = 0x00030c,
            UNABLE_TO_DELETE_DATABASE = 0x00030d,
            DATABASE_IS_CORRUPT = 0x00030e,
            // Location Error Values

            // General location error codes
            LOCATION_ERROR = 0x000400,
            OUT_OF_FRAME = 0x000401,
            INVALID_CROSSWISE_POSITION = 0x000402,
            INVALID_LENGTHWISE_POSITION = 0x000403,
            INVALID_DISTANCE = 0x000404,
            // Specific location error codes
            LOCATION_TOO_RIGHT = 0x000405,
            LOCATION_TOO_LEFT = 0x000406,
            LOCATION_TOO_HIGH = 0x000407,
            LOCATION_TOO_LOW = 0x000408,
            LOCATION_TOO_FAR = 0x000409,
            LOCATION_TOO_NEAR = 0x00040a,
            LOCATION_TOO_FORWARD = 0x00040b,
            LOCATION_TOO_BACKWARD = 0x00040c,
            // Quality Error Codes
            QUALITY_ERROR = 0x000501
        }

    }
}