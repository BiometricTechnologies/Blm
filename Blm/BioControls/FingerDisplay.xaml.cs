using System;
using System.Drawing;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Media;
using System.Windows.Media.Animation;
using System.Windows.Media.Imaging;

namespace IdentaZone.BioControls
{


    /// <summary>
    /// Interaction logic for CheckFinger.xaml
    /// </summary>
    public partial class FingerDisplay : UserControl
    {
        AnimatedGIFControl gifImage;

        public FingerDisplay()
        {
            InitializeComponent();

            IsDeviceConnected = true;
        }

        public FingerDisplay(Bitmap image)
        {
            InitializeComponent();

            gifImage = new AnimatedGIFControl(image);
            gifImage.Visibility = Visibility.Visible;

            GifPanel.Children.Add(gifImage);
        }

        public void ClearImage()
        {
            FingerImage.Source = null;
            gifImage.Visibility = Visibility.Visible;
        }

        public void ShowImage(byte[] image, int width, int height)
        {
            FingerImage.Source = CreateBitmap(image, width, height);
            gifImage.Visibility = Visibility.Hidden;
        }

        /// <summary>
        /// Create a bitmap from raw data in row/column format.
        /// </summary>
        /// <param name="Bytes"></param>
        /// <param name="Width"></param>
        /// <param name="Height"></param>
        /// <returns></returns>
        private BitmapSource CreateBitmap(byte[] bytes, int width, int height)
        {
            byte[] rgbBytes = new byte[bytes.Length * 3];

            for (int i = 0; i <= bytes.Length - 1; i++)
            {
                rgbBytes[(i * 3)] = bytes[i];
                rgbBytes[(i * 3) + 1] = bytes[i];
                rgbBytes[(i * 3) + 2] = bytes[i];
            }
            System.Windows.Media.PixelFormat pf = PixelFormats.Bgr24;
            int rawStride = (width * pf.BitsPerPixel + 7) / 8;

            BitmapSource bmp = BitmapSource.Create(width, height, 96, 96, pf, null, rgbBytes, rawStride);

            return bmp;
        }

        public void SetText(String p)
        {
            if (!Object.Equals(p, Message.Text))
            {
                Message.Text = p;
            }
        }

        Storyboard storyboard = new Storyboard();
        bool isAnimated = false;

        public void ShowPopUp(String Message)
        {
            if (isAnimated)
            {
                if (PopUp.Content.ToString() == Message)
                {
                    storyboard.Begin();
                    return;
                }
            }


            PopUp.Content = Message;
            PopUp.Visibility = System.Windows.Visibility.Visible;
            storyboard.Stop();

            var a = new DoubleAnimation
            {
                From = 1.0,
                To = 0.0,
                FillBehavior = FillBehavior.Stop,
                BeginTime = TimeSpan.FromSeconds(1.5),
                Duration = new Duration(TimeSpan.FromSeconds(0.5))
            };
            isAnimated = true;
            storyboard = new Storyboard();

            storyboard.Children.Add(a);
            Storyboard.SetTarget(a, PopUp);
            Storyboard.SetTargetProperty(a, new PropertyPath(OpacityProperty));
            storyboard.Completed += delegate { PopUp.Visibility = System.Windows.Visibility.Hidden; };
            storyboard.Completed += delegate { isAnimated = false; };
            storyboard.Begin();
        }

        private bool IsDeviceConnectedValue;
        public bool IsDeviceConnected
        {
            get
            {
                return IsDeviceConnectedValue;
            }
            set
            {
                if (value != IsDeviceConnectedValue)
                {
                    IsDeviceConnectedValue = value;
                    if (IsDeviceConnectedValue)
                    {
                        GifPanel.Children.Clear();
                        gifImage = new AnimatedGIFControl(IdentaZone.BioControls.Properties.Resources.cursor);
                        gifImage.Visibility = Visibility.Visible;

                        GifPanel.Children.Add(gifImage);
                    }
                    else
                    {
                        GifPanel.Children.Clear();
                        gifImage = new AnimatedGIFControl(IdentaZone.BioControls.Properties.Resources.no_device);
                        gifImage.Visibility = Visibility.Visible;

                        GifPanel.Children.Add(gifImage);
                    }
                }
            }
        }
    }
}
