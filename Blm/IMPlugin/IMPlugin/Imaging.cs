using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Media;
using System.Windows.Media.Imaging;

namespace IdentaZone.IMPlugin
{

    public static class Imaging
    {
        /// <summary>
        /// Create a bitmap from raw data in row/column format.
        /// </summary>
        /// <param name="Bytes"></param>
        /// <param name="Width"></param>
        /// <param name="Height"></param>
        /// <returns></returns>
        public static BitmapSource CreateBitmap(FingerPicture pic)
        {
            byte[] rgbBytes = new byte[pic.Image.Length * 3];

            for (int i = 0; i <= pic.Image.Length - 1; i++)
            {
                rgbBytes[(i * 3)] = pic.Image[i];
                rgbBytes[(i * 3) + 1] = pic.Image[i];
                rgbBytes[(i * 3) + 2] = pic.Image[i];
            }
            PixelFormat pf = PixelFormats.Bgr24;
            int rawStride = (pic.Width * pf.BitsPerPixel + 7) / 8;
            BitmapSource bmp = BitmapSource.Create(pic.Width, pic.Height, 96, 96, pf, null, rgbBytes, rawStride);
            return bmp;
        }

        public static byte[] CreatePNGImage(FingerPicture pic)
        {
            var bmp = CreateBitmap(pic);

            //PngBitmapEncoder encoder = new PngBitmapEncoder();
            var encoder = new JpegBitmapEncoder();
            MemoryStream memoryStream = new MemoryStream();

            encoder.Frames.Add(BitmapFrame.Create(bmp));
            encoder.Save(memoryStream);

            memoryStream.Seek(0, SeekOrigin.Begin);
            var res = memoryStream.ToArray();

            memoryStream.Close();

            return res;
        }
    }
}
