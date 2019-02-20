using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;
using System.Runtime.InteropServices;

namespace IdentaZone.BioSecure
{
    class BiosecLibClient
    {

        #region public interface

        public enum Ret
        {
            NoError,
            UserNotAnArchiveOwner
        }

        public enum Mode
        {
            Encrypt,
            Decrypt,
        }

        public enum EntityAtPathType
        {
            Folder,
            Regular
        }


        [StructLayout(LayoutKind.Sequential, Pack = 8)]
        public struct SourcePathInfo
        {
            [MarshalAs(UnmanagedType.LPWStr)]
            public string pathName;
        }

        [StructLayout(LayoutKind.Sequential, Pack = 8)]
        public struct ArchivePathInfo
        {
            [MarshalAs(UnmanagedType.LPWStr)]
            public String pathName;
            public UInt64 size;
            public Int64 mTime;
            public UInt64 index;
            public EntityAtPathType type;
        }


        [UnmanagedFunctionPointer(CallingConvention.StdCall)]
        public delegate void UpdateDelegate([MarshalAs(UnmanagedType.LPWStr)]string total, [MarshalAs(UnmanagedType.LPWStr)]string current, Int32 doneTotal, Int32 fromTotal, Int32 doneCurrent, Int32 fromCurrent);

        [UnmanagedFunctionPointer(CallingConvention.StdCall)]
        public delegate void ErrorDelegate(ErrorType errType, [MarshalAs(UnmanagedType.LPWStr)]string file, [MarshalAs(UnmanagedType.LPWStr)]string errorMessage);

        [UnmanagedFunctionPointer(CallingConvention.StdCall)]
        public delegate Byte FinishDelegate(CryptoOperationRes result, [MarshalAs(UnmanagedType.LPWStr)]string fileName);

        [UnmanagedFunctionPointer(CallingConvention.StdCall)]
        public delegate void BiometricProcessedDelegate(BiometricProcessingRes result);


        public void initialize(Mode mode, Action UserNotRegisteredCbc)
        {
            RetCode ret = init((UInt32)mode);
            
            if (ret == RetCode.UserNotEnrolled)
            {
                Auxiliary.Logger._log.Error("User hasn't got any registered biometric factors");
                UserNotRegisteredCbc();
                return;
            }

            if (RetCode.NoError != ret)
            {
                Auxiliary.Logger._log.Error("Error : " + ret);
                throw new System.Exception("Error: init biosecure library");
            }

        } 


        public void dispose()
        {
            RetCode ret = release();
            if (RetCode.NoError != ret)
            {
                Auxiliary.Logger._log.Error("Can't release biosecure library :" + ret);
                //throw new System.Exception("Error: release biosecure library");
            }

        }


        public Ret setSourcePathList(SourcePathInfo[] sourcePathList)
        {
            RetCode ret = setSourcePathList(sourcePathList, Convert.ToUInt64(sourcePathList.Length));
            if (RetCode.UserNotAnArchiveOwner == ret)
            {
                Auxiliary.Logger._log.Error("Current user isn't an archive owner");
                return Ret.UserNotAnArchiveOwner;
            }
            if (RetCode.NoError != ret)
            {
                Auxiliary.Logger._log.Error("Can't set source path :" + ret);
                throw new System.Exception("Error: set source path list");
            }

            return Ret.NoError;
        }


        public void setTargetDirectory(string targetDir)
        {
            RetCode ret = setTargetDir(targetDir);
            if (RetCode.NoError != ret)
            {
                Auxiliary.Logger._log.Error("Can't set target directory :" + ret);
                throw new System.Exception("Error: set target directory");
            }
        }


        public List<StringBuilder> getCryptoProviders()
        {
            List<StringBuilder> providers = new List<StringBuilder>();
            UInt32 index = 0;
            RetCode ret;

            bool needPollMore = true;
            while (needPollMore)
            {
                StringBuilder provider = new StringBuilder(Convert.ToInt32(maxCryptoProvderLength));
                ret = getCryptoProviders(index, provider, maxCryptoProvderLength);
                switch (ret)
                {
                    case RetCode.NoError:
                        providers.Add(provider);
                        break;
                    case RetCode.IndexOutOfRange:
                        needPollMore = false;
                        break;
                    case RetCode.Busy:
                        Thread.Sleep(200);
                        break;
                    case RetCode.LibraryNotInited:
                        Auxiliary.Logger._log.Error("Biosec library not inited");
                        throw new System.Exception("Error: biosec library not inited");
                        break;
                    case RetCode.InternalError:
                        Auxiliary.Logger._log.Error("Biosec library internal error");
                        throw new System.Exception("Error: biosec library internal error");
                        break;
                    default:
                        Auxiliary.Logger._log.Error("Unhandled error " + ret);
                        throw new System.Exception("Error: polling crypto providers");
                        break;
                }
                index++;
            }
            return providers;
        }


        public void setSelectedCryptoProvider(string provider)
        {
            RetCode ret = setCryptoProvider(provider);
            if (RetCode.NoError != ret)
            {
                Auxiliary.Logger._log.Error("Can't set (select) crypto provider" + ret);
                throw new System.Exception("Error: set crypto provider");
            }
        }


        public void putBiometric(string biometricType, Byte[] biometric, BiometricProcessedDelegate fpBiometricProcessed)
        {
            int size = Marshal.SizeOf(biometric[0]) * biometric.Length;
            IntPtr pnt = Marshal.AllocHGlobal(size);
            try
            {
                // Copy the array to unmanaged memory.
                Marshal.Copy(biometric, 0, pnt, biometric.Length);
                RetCode ret = putBiometric(biometricType, pnt, Convert.ToUInt64(biometric.Length), fpBiometricProcessed);
                if (RetCode.NoError != ret)
                {
                    Auxiliary.Logger._log.Error("Can't put biometric with :" + ret);
                    throw new System.Exception("Error: put biometric data");
                }
            }
            finally
            {
                Marshal.FreeHGlobal(pnt);
            }
        }


        public void setOverwriteFlag(Boolean needOverwrite)
        {
            RetCode ret = setOverwriteFlagValue(needOverwrite);
            if (RetCode.NoError != ret)
            {
                Auxiliary.Logger._log.Error("Can't set overwrite flag :" + ret);
                throw new System.Exception("Error: set overwrite flag");
            }
        }


        public StringBuilder getUserName()
        {
            StringBuilder userName = new StringBuilder(Convert.ToInt32(maxUserNameLength));
            RetCode ret = getUserName(userName, maxUserNameLength);
            if (RetCode.NoError != ret)
            {
                Auxiliary.Logger._log.Error("Can't get username :" + ret);
                throw new System.Exception("Error: get user name");
            }
            return userName;
        }


        public void startEncryptionProcess(UpdateDelegate pUpdate, FinishDelegate pFinish, ErrorDelegate pError)
        {
            StringBuilder userName = new StringBuilder(Convert.ToInt32(maxUserNameLength));
            RetCode ret = startEncryption(pUpdate, pFinish, pError);
            if (RetCode.NoError != ret)
            {
                Auxiliary.Logger._log.Error("Can't start encryption process with :" + ret);
                throw new System.Exception("Error: start encryption");
            }
        }


        public void startDecryptionProcess(UpdateDelegate pUpdate, FinishDelegate pFinish, ErrorDelegate pError)
        {
            StringBuilder userName = new StringBuilder(Convert.ToInt32(maxUserNameLength));
            RetCode ret = startDecryption(pUpdate, pFinish, pError);
            if (RetCode.NoError != ret)
            {
                Auxiliary.Logger._log.Error("Can't start decryption process with :" + ret);
                throw new System.Exception("Error: start encryption");
            }
        }


        // deprecated
        public List<ArchivePathInfo> getArchivePaths()
        {
            List<ArchivePathInfo> pathInfoList = new List<ArchivePathInfo>();
            UInt32 index = 0;
            RetCode ret;

            bool needPollMore = true;
            while (needPollMore)
            {
                ArchivePathInfo pathInfo = new ArchivePathInfo();
                pathInfo.pathName = new String(' ', Convert.ToInt32(maxArchivepathLen));
                ret = getArchivePath(index, ref pathInfo, maxCryptoProvderLength);

                switch (ret)
                {
                    case RetCode.NoError:
                        pathInfoList.Add(pathInfo);
                        break;
                    case RetCode.IndexOutOfRange:
                        needPollMore = false;
                        break;
                    
                    default:
                        Auxiliary.Logger._log.Error("Can't get archive paths :" + ret);
                        throw new System.Exception("Error: polling archive paths");
                        break;
                }
                index++;
            }
            return pathInfoList;
        }



        public Nullable<ArchivePathInfo> getArchivePathsAt(UInt32 index)
        {
            RetCode ret;

            ArchivePathInfo pathInfo = new ArchivePathInfo();
            pathInfo.pathName = new String(' ', Convert.ToInt32(maxArchivepathLen));
            ret = getArchivePath(index, ref pathInfo, maxArchivepathLen);

                switch (ret)
                {
                    case RetCode.NoError:
                        return pathInfo;
                        break;
                    case RetCode.IndexOutOfRange:
                        return null;
                        break;

                    default:
                        Auxiliary.Logger._log.Error("Can't get archive paths :" + ret);
                        throw new System.Exception("Error: polling archive paths");
                        break;
                }
        }


        public void setSelectionPaths(UInt64[] selectedPathsIndices)
        {

            if (0 == selectedPathsIndices.Length)
            {
                return;
            }

            int  size = Marshal.SizeOf(selectedPathsIndices[0]) * selectedPathsIndices.Length;
            IntPtr pnt = Marshal.AllocHGlobal(size);
            try
            {
                Int64[] temp = new Int64[selectedPathsIndices.Length];
                System.Buffer.BlockCopy(selectedPathsIndices, 0, temp, 0, temp.Length * 8);
                // Copy the array to unmanaged memory.
                Marshal.Copy(temp, 0, pnt, temp.Length);
                RetCode ret = setSelectionPaths(pnt, Convert.ToUInt64(temp.Length));
                if (RetCode.NoError != ret)
                {
                    throw new System.Exception("Error: set selection");
                }
            }
            finally
            {
                Marshal.FreeHGlobal(pnt);

            }
        }


        public UInt64 getArchiveEntriesCount()
        {
            UInt64 count = 0;
            RetCode ret = getEntriesInArchiveCount(ref count);
            if (RetCode.NoError != ret)
            {
                throw new System.Exception("Error: getArchiveEntriesCount");
            }

            return count;
        }

        #endregion

        private const string biosecLibraryname = "biosec_lib.dll";
        private const UInt32 maxCryptoProvderLength = 200;
        private const UInt32 maxArchivepathLen = 500;
        private const UInt32 maxUserNameLength = 256;


        #region Data types

        internal enum BiometricProcessingRes : byte
        {
            BiometricAccepted,
            BiometricMatchNotFound,
            BiometricNotBelongsToUser,
            BiometricInvalidEncryptionKey,
            BiometricInternalError,
            BiometricUnknowEnrror
        }

        internal enum CryptoOperationRes : byte
        {
            BIOSEC_NO_ERROR,
            BIOSEC_INDEX_OUT_OF_RANGE,
            BIOSEC_INTERNAL_ERROR,
            BIOSEC_LIB_NOT_INITED,
            BIOSEC_INVALID_ARG,
            BIOSEC_BUSY,
            BIOSEC_USER_NOT_ENROLLED,
            BIOSEC_USER_NOT_AN_ARCHIVE_OWNER,
            BIOSEC_UNKNOWN_ERROR            
        }


        internal enum ErrorType : byte
        {
            Critical,
            Error
        }


        internal enum RetCode : byte
        {
            NoError,
            IndexOutOfRange,
            InternalError,
            LibraryNotInited,
            InvalidArgument,
            Busy,
            UserNotEnrolled,
            UserNotAnArchiveOwner,
            UnknownError
        }


        #endregion




        [DllImport(biosecLibraryname, CallingConvention = CallingConvention.StdCall)]
        static extern RetCode init(UInt32 mode);

        [DllImport(biosecLibraryname, CallingConvention = CallingConvention.StdCall)]
        static extern RetCode release();

        [DllImport(biosecLibraryname, CallingConvention = CallingConvention.StdCall)]
        static extern RetCode setSourcePathList([MarshalAs(UnmanagedType.LPArray, SizeParamIndex = 1)] SourcePathInfo[] pathArray, UInt64 size);

        [DllImport(biosecLibraryname, CallingConvention = CallingConvention.StdCall)]
        static extern RetCode setTargetDir([MarshalAs(UnmanagedType.LPWStr)] string targetDir);

        [DllImport(biosecLibraryname, CallingConvention = CallingConvention.StdCall)]
        static extern RetCode getCryptoProviders(UInt32 index, [MarshalAs(UnmanagedType.LPWStr)] StringBuilder buffer, UInt32 bufferSize);

        [DllImport(biosecLibraryname, CallingConvention = CallingConvention.StdCall)]
        static extern RetCode setCryptoProvider([MarshalAs(UnmanagedType.LPWStr)] string provider);

        [DllImport(biosecLibraryname, CallingConvention = CallingConvention.StdCall)]
        static extern RetCode putBiometric([MarshalAs(UnmanagedType.LPWStr)]string biometricType, IntPtr data, UInt64 size, [MarshalAs(UnmanagedType.FunctionPtr)] BiometricProcessedDelegate fpBiometricProcessed);

        [DllImport(biosecLibraryname, CallingConvention = CallingConvention.StdCall)]
        static extern RetCode setOverwriteFlagValue(Boolean needOverwrite);

        [DllImport(biosecLibraryname, CallingConvention = CallingConvention.StdCall)]
        static extern RetCode getUserName([MarshalAs(UnmanagedType.LPWStr)] StringBuilder buffer, UInt64 bufferSize);

        [DllImport(biosecLibraryname, CallingConvention = CallingConvention.StdCall)]
        static extern RetCode startEncryption([MarshalAs(UnmanagedType.FunctionPtr)] UpdateDelegate pUpdate, [MarshalAs(UnmanagedType.FunctionPtr)] FinishDelegate pFinish, ErrorDelegate pError);

        [DllImport(biosecLibraryname, CallingConvention = CallingConvention.StdCall)]
        static extern RetCode startDecryption([MarshalAs(UnmanagedType.FunctionPtr)] UpdateDelegate pUpdate, [MarshalAs(UnmanagedType.FunctionPtr)] FinishDelegate pFinish, ErrorDelegate pError);

        [DllImport(biosecLibraryname, CallingConvention = CallingConvention.StdCall)]
        static extern RetCode getArchivePath(UInt32 index, ref ArchivePathInfo pathInFo, UInt64 pathnameBufferSize);

        [DllImport(biosecLibraryname, CallingConvention = CallingConvention.StdCall)]
        static extern RetCode setSelectionPaths(IntPtr indices, UInt64 size);

        [DllImport(biosecLibraryname, CallingConvention = CallingConvention.StdCall)]
        static extern RetCode getEntriesInArchiveCount(ref UInt64 count);

    }
}
