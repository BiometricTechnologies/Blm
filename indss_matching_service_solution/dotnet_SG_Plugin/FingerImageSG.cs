using IdentaZone.IMPlugin;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SG
{
    public class FingerImageSG : FingerImage
    {
        public byte[] RawData { get; set; }
        public int Width { get; set; }
        public int Height { get; set; }
        private int _bspcode;
        public FingerImageSG(int bspcode, byte[] rawData, int width, int height)
        {
            this._bspcode = bspcode;
            this.RawData = new byte[rawData.Length];
            Array.Copy(rawData, this.RawData, rawData.Length);
            this.Width = width;
            this.Height = height;
        }

        override public FingerPicture MakePicture()
        {
            return new FingerPicture(RawData, Width, Height);
        }

        public override void Serialize(out string type, out byte[] data)
        {
            switch (_bspcode)
            {
                case 22:
                    type = TemplateTypes.SG03Image;
                    break;
                case 23:
                    type = TemplateTypes.SG20Image;
                    break;
                default:
                    type = TemplateTypes.SG04Image;
                    break;
            }
            data = RawData;
        }
    }
}
