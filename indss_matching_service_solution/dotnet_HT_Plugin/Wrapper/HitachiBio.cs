using log4net;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using System.Runtime.ExceptionServices;
namespace Hitachi.Wrapper
{
    public static class HitachiBio
    {
        private static readonly ILog _log = log4net.LogManager.GetLogger(typeof(HitachiBio));
        private static bioapi_framework_schema _frameworkSchema;
        private static bioapi_bsp_schema _bspinfo;

        private static IntPtr BSPArray = IntPtr.Zero;
        private static uint BSPCount;
        private static bool BSPLoaded = false;
        private static IntPtr DeviceArray;
        private static uint deviceCount;


        private static bool isInitialized = false;


        public static bool Initialize()
        {
            uint res = BioAPI.Init(0x20);

            if (res != BioAPI.OK)
            {
                _log.Error("Error while initializing");
                return false;
            }

            res = BioAPI.GetFrameworkInfo(ref _frameworkSchema);
            if (res != BioAPI.OK)
            {
                _log.Error("Error while getting framework info");
                return false;
            }
            BioAPI.Free(_frameworkSchema.Path);
            res = BioAPI.EnumBSPs(ref BSPArray, ref BSPCount);
            if (res != BioAPI.OK)
            {
                _log.Error("Error while enumerating BSPs " + res.ToString());
                return false;
            }
            if (BSPCount == 0)
            {
                _log.Error("No BSP detected");
                return false;
            }
            _bspinfo = BioAPI.BSPInfo(BSPArray, 0);
            BioAPI.Free(_bspinfo.Path);
            BioAPI.Free(_bspinfo.BSPSupportedFormats);


            isInitialized = true;
            return true;
        }

        /// <summary>
        /// 
        /// </summary>
        /// <returns></returns>
        /// 
        [HandleProcessCorruptedStateExceptions]
        public static List<bioapi_unit_schema> EnumerateDevices()
        {
            uint res;
            List<bioapi_unit_schema> result = new List<bioapi_unit_schema>();
            bioapi_unit_schema unitinfo;

            GCHandle gch = GCHandle.Alloc(_bspinfo.BSPUuid, GCHandleType.Pinned);
            try
            {
                res = BioAPI.OK;
                if (BSPLoaded)
                {
                    try
                    {
                        res = BioAPI.BSPUnload(gch.AddrOfPinnedObject(), (BioAPI.EventHandler)null, IntPtr.Zero);
                    }
                    catch (Exception ex)
                    {
                        _log.Error(ex);
                    }
                }
                if (res == BioAPI.OK)
                {
                    BSPLoaded = false;
                    res = BioAPI.BSPLoad(gch.AddrOfPinnedObject(), (BioAPI.EventHandler)null, IntPtr.Zero);
                    if (res == BioAPI.OK)
                    {
                        BSPLoaded = true;
                        res = BioAPI.QueryUnits(gch.AddrOfPinnedObject(), ref DeviceArray, ref deviceCount);
                        if (res == BioAPI.OK)
                        {
                            for (int i = 0; i < deviceCount; i++)
                            {
                                int size = Marshal.SizeOf(typeof(bioapi_unit_schema));
                                unitinfo = (bioapi_unit_schema)Marshal.PtrToStructure(DeviceArray + size * i, typeof(bioapi_unit_schema));
                                result.Add(unitinfo);
                            }
                        }
                        else
                        {
                            throw new Exception("QueryUnits " + res.ToString());
                        }
                    }
                    else
                    {
                        throw new Exception("BSPLoad " + res.ToString());
                    }
                }
                else
                {
                    throw new Exception("BSPUnload "+res.ToString());
                }
            }
            catch (Exception ex)
            {
                _log.Error(ex);
            }
            finally
            {
                gch.Free();
            }
            return result;
        }

        internal static void Dispose()
        {
            if (isInitialized)
            {
                if (BSPLoaded)
                {
                    if (BSPArray != IntPtr.Zero)
                    {
                        BioAPI.Free(BSPArray);
                    }
                    GCHandle gch = GCHandle.Alloc(_bspinfo.BSPUuid, GCHandleType.Pinned);
                    try
                    {
                        var res = BioAPI.BSPUnload(gch.AddrOfPinnedObject(), (BioAPI.EventHandler)null, IntPtr.Zero);
                        if (res != BioAPI.OK)
                        {
                            throw new Exception("BSPUnload " + res.ToString());
                        }
                    }
                    catch (Exception ex)
                    {
                        _log.Error(ex);
                    }
                    finally
                    {
                        gch.Free();
                    }
                    BSPLoaded = false;
                }
                BioAPI.Terminate();
                isInitialized = false;
            }
        }

        internal static bool Attach(bioapi_unit_schema unitinfo, ref uint handle, BioAPI.GUI_STREAMING_CALLBACK streamingCallBack, BioAPI.GUI_STATE_CALLBACK stateCallBack)
        {
            bool result = false;
            GCHandle uuidHandle = GCHandle.Alloc(unitinfo.BSPUuid, GCHandleType.Pinned);
            try
            {
                IntPtr uuidPtr = uuidHandle.AddrOfPinnedObject();
                bioapi_unit_list_element unitlist;
                unitlist.UnitCategory = unitinfo.UnitCategory;
                unitlist.UnitId = unitinfo.UnitId;
                uint res = BioAPI.BSPAttach(uuidPtr, 0x20, ref unitlist, 1, ref handle);
                if (res == 0)
                {
                    res = BioAPI.SetGUICallbacks(handle,
                        streamingCallBack,
                        IntPtr.Zero,
                        stateCallBack,
                        IntPtr.Zero);
                    result = true;
                }
                else
                {
                    throw new Exception("BSPAttach " + res.ToString());
                }
            }
            catch (Exception ex)
            {
                _log.Error(ex);
            }
            finally
            {
                uuidHandle.Free();
            }

            return result;
        }

        internal static void Detach(uint handle)
        {
            uint res = BioAPI.BSPDetach(handle);
            if (res != BioAPI.OK)
            {
                _log.Error("BSPDetach " + res.ToString());
            }
        }

        internal static int Match(uint handle, IdentaZone.IMPlugin.FingerTemplate template, 
            IEnumerable<IdentaZone.IMPlugin.FingerTemplate> candidates, out List<IdentaZone.IMPlugin.FingerTemplate> matches)
        {
            matches = new List<IdentaZone.IMPlugin.FingerTemplate>();

            uint res = 0;
            byte result = 0;
            int FMRAchieved = 0;
            bioapi_input_bir proc_input_bir = new bioapi_input_bir();
            bioapi_input_bir db_input_bir = new bioapi_input_bir();
            int birsize = Marshal.SizeOf(typeof(bioapi_bir));
            IntPtr dbBirPtr = Marshal.AllocHGlobal(birsize);
            IntPtr procBirPtr = Marshal.AllocHGlobal(birsize);
            bioapi_bir dbBir, procBir;
            IntPtr procBirDataPtr;
            procBir = (template as TemplateHi).BirImage.bir_;
            procBirDataPtr = Marshal.AllocHGlobal((int)procBir.BiometricData.Length);
            try
            {
                procBir.BiometricData.Data = procBirDataPtr;
                procBir.SecurityBlock.Data = IntPtr.Zero;
                procBir.SecurityBlock.Length = 0;
                procBir.Header.Purpose = BioAPI.PURPOSE_VERIFY;
                Marshal.Copy((template as TemplateHi).BirImage.BiometricData, 0, procBirDataPtr, (int)procBir.BiometricData.Length);
                Marshal.StructureToPtr(procBir, procBirPtr, false);

                proc_input_bir.Form = BioAPI.FULLBIR_INPUT;
                proc_input_bir.InputBIR.BIR = procBirPtr;


                foreach (TemplateHi finger in candidates)
                {
                    IntPtr dbBirDataPtr;
                    
                    dbBir = finger.BirImage.bir_;
                    dbBirDataPtr = Marshal.AllocHGlobal((int)dbBir.BiometricData.Length);
                    try
                    {
                        Marshal.Copy(finger.BirImage.BiometricData, 0, dbBirDataPtr, (int)dbBir.BiometricData.Length);
                        dbBir.BiometricData.Data = dbBirDataPtr;
                        dbBir.BiometricData.Length = (uint)finger.BirImage.BiometricData.Length;
                        dbBir.SecurityBlock.Data = IntPtr.Zero;
                        dbBir.SecurityBlock.Length = 0;

                        Marshal.StructureToPtr(dbBir, dbBirPtr, false);
                        db_input_bir.Form = BioAPI.FULLBIR_INPUT;
                        db_input_bir.InputBIR.BIR = dbBirPtr;
                        res = BioAPI.VerifyMatch(handle, 2072, ref proc_input_bir, ref db_input_bir, ref result, ref FMRAchieved);
                        if (res != BioAPI.OK)
                        {
                            throw new Exception("VerifyMatch " + res.ToString());
                        }
                        FMRAchieved = (int)res;
                    }
                    catch (System.ComponentModel.Win32Exception ex)
                    {
                        _log.Error("VerifyMatch " + ex.Message);
                    }
                    catch (Exception ex)
                    {
                        _log.Error(ex);
                    }
                    finally
                    {
                        Marshal.FreeHGlobal(dbBirDataPtr);
                    }

                    if (result != 0)
                    {
                        matches.Add(finger);
                    }
                }
            }
            catch (Exception ex)
            {
                _log.Error(ex);
            }
            finally
            {
                Marshal.FreeHGlobal(dbBirPtr);
                Marshal.FreeHGlobal(procBirPtr);
            }
            Marshal.FreeHGlobal(procBirDataPtr);
            return matches.Count();
        }
    }
}
