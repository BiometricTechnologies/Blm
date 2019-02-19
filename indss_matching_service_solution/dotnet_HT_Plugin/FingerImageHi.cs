using IdentaZone.IMPlugin;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using log4net;

namespace Hitachi
{
    public class FingerImageHi : FingerImage
    {
        public bioapi_bir bir_;
        public byte[] BiometricData { get; set; }
        public byte[] SecurityBlock { get; set; }

        private readonly ILog _log = log4net.LogManager.GetLogger(typeof(DeviceHi));

        public FingerImageHi(bioapi_bir bir)
        {
            bir_ = bir;
            if (bir.BiometricData.Length > 0)
            {
                BiometricData = new byte[bir.BiometricData.Length];
                Marshal.Copy(bir.BiometricData.Data, BiometricData, 0, (int)bir.BiometricData.Length);
            }
            if (bir.SecurityBlock.Length > 0)
            {
                SecurityBlock = new byte[bir.SecurityBlock.Length];
                Marshal.Copy(bir.SecurityBlock.Data, BiometricData, 0, (int)bir.SecurityBlock.Length);
            }
        }
        public FingerImageHi(byte[] rawdata)
        {
            int birsize = Marshal.SizeOf(typeof(bioapi_bir));
            int dataLength = rawdata.Length - birsize;
            ////
            GCHandle gcDataHandle = GCHandle.Alloc(rawdata, GCHandleType.Pinned);
            bir_ = (bioapi_bir)Marshal.PtrToStructure(gcDataHandle.AddrOfPinnedObject(),
                typeof(bioapi_bir));
            gcDataHandle.Free();
            if (bir_.BiometricData.Length > 0)
            {
                BiometricData = new byte[bir_.BiometricData.Length];
                _log.Info(rawdata.Count()+" "+BiometricData.Length+" "+birsize);
                Array.Copy(rawdata, birsize, BiometricData, 0, bir_.BiometricData.Length);
                
            }
            if (bir_.SecurityBlock.Length > 0)
            {
                SecurityBlock = new byte[bir_.SecurityBlock.Length];
                Array.Copy(rawdata, birsize + bir_.BiometricData.Length, SecurityBlock, 0, bir_.SecurityBlock.Length);
            }
        }
        override public FingerPicture MakePicture()
        {
            int w = (int)Math.Sqrt((double)BiometricData.Length);
            return new FingerPicture(BiometricData, w, w);
        }
        internal byte[] Serialize()
        {
            int birsize = Marshal.SizeOf(typeof(bioapi_bir));
            byte[] birBytes = new byte[birsize + BiometricData.Length];
            GCHandle gcheader = GCHandle.Alloc(birBytes, GCHandleType.Pinned);
            Marshal.StructureToPtr(bir_, gcheader.AddrOfPinnedObject(), false);
            gcheader.Free();
            BiometricData.CopyTo(birBytes, birsize);
            return birBytes;
        }


        override public void Serialize(out string type, out byte[] data)
        {
            type = "IMG_HI_DEFAULT";
            data = Serialize();
        }

    }
}
