using BioNetACSLib;
using IdentaZone.IMPlugin;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace IB
{
    public class FingerImageIB : FingerImage
    {
        public byte[] rawData { get; set; }
        public int width { get; set; }
        public int height { get; set; }

        public FingerImageIB(byte[] rawData, int width, int height)
        {
            this.rawData = rawData;
            this.width = width;
            this.height = height;
        }

        override public FingerPicture MakePicture()
        {
            byte[] picture = BioNetACSDLL._RotateImage(rawData, false);
            return new FingerPicture(picture, height, width);
        }

         public override void Serialize(out string type, out byte[] data)
        {
            type = TemplateTypes.IBImage;
            data = rawData;
        }
    }
}
