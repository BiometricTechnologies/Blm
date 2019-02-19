using IdentaZone.IMPlugin;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Hitachi
{
    public class TemplateHi : FingerTemplate
    {
        public FingerImageHi BirImage;
        public TemplateHi(bioapi_bir bir)
        {
            BirImage = new FingerImageHi(bir);
        }
        public TemplateHi(byte[] rawdata)
        {
            BirImage = new FingerImageHi(rawdata);
        }
        override public int BSPCode
        {
            get { return TemplateTypes.HiTemplate; }
        }

        override public byte[] Serialize()
        {
            return BirImage.Serialize();
        }
    }
}
