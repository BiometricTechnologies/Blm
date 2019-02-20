using System;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Media;
using System.Windows.Threading;

namespace UIControlsINDSS
{
    /// <summary>
    /// Interaction logic for FingerDisplayINDSS.xaml
    /// </summary>
    public partial class FingerDisplayINDSS : UserControl
    {
        private DispatcherTimer _cleanImageTmr=new DispatcherTimer();




        public bool EnableTimeoutView
        {
            get { return (bool)GetValue(EnableTimeoutViewProperty); }
            set { SetValue(EnableTimeoutViewProperty, value); }
        }

        // Using a DependencyProperty as the backing store for EnableTimeoutView.  This enables animation, styling, binding, etc...
        public static readonly DependencyProperty EnableTimeoutViewProperty =
            DependencyProperty.Register("EnableTimeoutView", typeof(bool), typeof(FingerDisplayINDSS), new PropertyMetadata(true));

        

        public static ImageSource GetFingerPrintImageSource(DependencyObject obj)
        {
            return (ImageSource)obj.GetValue(FingerPrintImageSourceProperty);
        }

        public static void SetFingerPrintImageSource(DependencyObject obj, ImageSource value)
        {
            obj.SetValue(FingerPrintImageSourceProperty, value);
        }

        // Using a DependencyProperty as the backing store for FingerPrintImageSource.  This enables animation, styling, binding, etc...
        public static readonly DependencyProperty FingerPrintImageSourceProperty =
            DependencyProperty.Register("FingerPrintImageSource", typeof(ImageSource), typeof(FingerDisplayINDSS), new FrameworkPropertyMetadata(
                null,
                FrameworkPropertyMetadataOptions.AffectsRender,
                OnCallbackFingerPrintImageSource));

        private static void OnCallbackFingerPrintImageSource(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
             ((FingerDisplayINDSS)d).Mode3();

        }


        public static bool GetIsNoDevice(DependencyObject obj)
        {
            return (bool)obj.GetValue(IsNoDeviceProperty);
        }

        public static void SetIsNoDevice(DependencyObject obj, bool value)
        {
            obj.SetValue(IsNoDeviceProperty, value);
        }

        // Using a DependencyProperty as the backing store for IsNoDevice.  This enables animation, styling, binding, etc...
        public static readonly DependencyProperty IsNoDeviceProperty =
            DependencyProperty.RegisterAttached("IsNoDevice", typeof(bool), typeof(FingerDisplayINDSS), new PropertyMetadata(false, OnCallbackIsNoDevice));



        private static void OnCallbackIsNoDevice(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            if ((bool) e.NewValue)
            {
                ((FingerDisplayINDSS) d).Mode2();
            }
            else
            {
                ((FingerDisplayINDSS)d).Mode1();
            }
        }


        public FingerDisplayINDSS()
        {

            InitializeComponent();
            Mode1();
            _cleanImageTmr.Tick += cleanImageTick;
            _cleanImageTmr.Interval = new TimeSpan(0, 0, 2);
        }


        private void cleanImageTick(object sender, EventArgs e)
        {
            //_fingerImage.Source = null;
            //_cleanImageTmr.Stop();
            Mode1();
        }


        private void Mode1()
        {
            _cleanImageTmr.Stop();
            _arc.Visibility = Visibility.Visible;
            _arcFull.Visibility = Visibility.Visible;
            _imageFingerPrint.Visibility = Visibility.Visible;
            _redArc.Visibility = Visibility.Hidden;
            _redLine.Visibility = Visibility.Hidden;
            _imageDevice.Visibility = Visibility.Hidden;
            _fingerImage.Visibility = Visibility.Hidden;
        }

        private void Mode2()
        {
            _cleanImageTmr.Stop();
            _arc.Visibility = Visibility.Hidden;
            _arcFull.Visibility = Visibility.Visible;
            _imageFingerPrint.Visibility = Visibility.Hidden;
            _redArc.Visibility = Visibility.Visible;
            _redLine.Visibility = Visibility.Visible;
            _imageDevice.Visibility = Visibility.Visible;
            _fingerImage.Visibility = Visibility.Hidden;
        }

        private void Mode3()
        {
            
            _fingerImage.Visibility = Visibility.Visible;
            _arc.Visibility = Visibility.Hidden;
            _arcFull.Visibility = Visibility.Hidden;
            _imageFingerPrint.Visibility = Visibility.Hidden;
            _redArc.Visibility = Visibility.Hidden;
            _redLine.Visibility = Visibility.Hidden;
            _imageDevice.Visibility = Visibility.Hidden;
            if (EnableTimeoutView) _cleanImageTmr.Start();
            
        }
    }
}
