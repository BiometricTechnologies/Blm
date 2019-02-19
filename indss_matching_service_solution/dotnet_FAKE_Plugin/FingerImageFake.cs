using IdentaZone.IMPlugin;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace IdentaZone.Plugins.FAKE
{
    public class FingerImageFake : FingerImage
    {
        public override FingerPicture MakePicture()
        {
            int height = 300;
            int width = 200;
            byte[] picture = new byte[height * width];
            return new FingerPicture(picture, height, width);
        }

        public override void Serialize(out string type, out byte[] data)
        {
            type = TemplateTypes.FakeImage;
            data = new byte[0];
        }


    }
}
