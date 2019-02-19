using IdentaZone.IMPlugin;
using SecuGen.FDxSDKPro.Windows;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SG
{
    public class TemplateSG : FingerTemplate
    {
        public SGFPMFingerInfo info;
        public byte[] Bytes { get; set; }
        public int size;
        private int _BSPCode;

        override public int BSPCode
        {
            get { return _BSPCode; }
        }

        override public byte[] Serialize()
        {
            return Bytes;
        }

        public TemplateSG(int bspcode)
        {
            _BSPCode = bspcode;
        }

        public TemplateSG(int bspcode, byte[] data)
        {
            _BSPCode = bspcode;
            Bytes = data;
        }
    }
}
