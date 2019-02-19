using DPUruNet;
using IdentaZone.IMPlugin;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace IdentaZone.Plugins.DP
{
    public class FingerImageDP : FingerImage
    {
        public Fid fid { get; set; }

        public FingerImageDP(Fid fid)
        {
            this.fid = fid;
        }

        override public FingerPicture MakePicture()
        {
            return new FingerPicture(fid.Views[0].RawImage, fid.Views[0].Width, fid.Views[0].Height);
        }

        public override void Serialize(out string type, out byte[] data)
        {
            type = TemplateTypes.DPImageAnsi;
            data = fid.Bytes;
        }
    }
}
