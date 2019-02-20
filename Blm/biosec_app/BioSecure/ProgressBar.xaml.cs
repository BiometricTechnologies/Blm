using log4net;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Timers;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Animation;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using System.Windows.Threading;
using System.Windows.Shell;
using System.IO;
using System.Drawing;
using System.Drawing.Imaging;
using System.Timers;
using System.ComponentModel;



namespace IdentaZone.BioSecure
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class ProgressBar : Window, INotifyPropertyChanged
    {
        DispatcherTimer destroyer;

        bool toDestroy = false;
        private Timer animationTiemr;
        private System.Drawing.Image animatedGifImage;
        private int framesCount;
        private int currentFrame;
        private FrameDimension dimension;
        private BitmapImage[] animationFrames;

        private ImageSource _overlayAnimation;
        public ImageSource OverlayAnimation
        {
            get
            {
                return _overlayAnimation;
            }
            set
            {
                _overlayAnimation = value;
                OnPropertyChanged("OverlayAnimation");
            }
        }

        public ProgressBar()
        {
            InitializeComponent();
            try
            {
                taskBarItemInfo.Description = "Biosecure progress";
                taskBarItemInfo.ProgressState = TaskbarItemProgressState.Indeterminate;
                // var bitmapImage = new BitmapImage(new Uri("pack://application:,,,/testOverlayIcon;component/Images/red_circle_preloader.gif"));
                initAnimation();
                
                
                //log.Info("Progress bar inited ");
            }
            catch (Exception ex)
            {
                Auxiliary.Logger._log.Error("Progress bar initialization error: " + ex.Message);
            } 
            
        }


        private void initAnimation()
        {
            var assembly = System.Reflection.Assembly.GetExecutingAssembly().Location;
            var path = System.IO.Path.GetDirectoryName(assembly);

            
            path = path + "\\Images\\progress_animation.gif";
            //log.Info("animation image path: " + path);
            animatedGifImage = System.Drawing.Image.FromFile(path);
            dimension = new FrameDimension(animatedGifImage.FrameDimensionsList[0]);
            framesCount = animatedGifImage.GetFrameCount(FrameDimension.Time);
            animationFrames = new BitmapImage[framesCount];
            for (int frame = 0; frame < framesCount; ++frame)
            {
                animatedGifImage.SelectActiveFrame(dimension, frame);
                animationFrames[frame] = getBitmapImage(animatedGifImage);
            }

            currentFrame = 0;
            animatedGifImage.SelectActiveFrame(dimension, currentFrame);
            initTimer();
        }


        private void initTimer()
        {
            animationTiemr = new System.Timers.Timer(90);
            animationTiemr.Elapsed += new ElapsedEventHandler(OnTimedEvent);
            animationTiemr.AutoReset = true;
            animationTiemr.Enabled = true;
        }



        private BitmapImage getBitmapImage(System.Drawing.Image image)
        {
            Bitmap img = (Bitmap)image;
            BitmapImage bmImg = new BitmapImage();

            using (MemoryStream ms = new MemoryStream())
            {
                img.Save(ms, System.Drawing.Imaging.ImageFormat.Png);
                ms.Position = 0;

                bmImg.BeginInit();
                bmImg.CacheOption = BitmapCacheOption.OnLoad;
                bmImg.UriSource = null;
                bmImg.StreamSource = ms;
                bmImg.EndInit();
            }

            return bmImg;
        }



        private void OnTimedEvent(object source, ElapsedEventArgs e)
        {
            try
            {
                currentFrame++;
                if (currentFrame >= framesCount)
                {
                    currentFrame = 0;
                }

                Dispatcher.Invoke(new Action(() =>
                {
                    OverlayAnimation = animationFrames[currentFrame];

                }));
            }
            catch (Exception ex)
            {

            }
        }

        
        // boiler-plate
        public event PropertyChangedEventHandler PropertyChanged;
        protected virtual void OnPropertyChanged(String propertyName)
        {
            PropertyChangedEventHandler handler = PropertyChanged;
            if (handler != null) handler(this, new PropertyChangedEventArgs(propertyName));
        }


        public void ShowWindow()
        {
            //log.Info("Showing window");
            this.Show();
        }

        public void HideWindow()
        {
            //log.Info("Hiding window");
            this.Hide();
        }

        public void Destroy()
        {
            if (destroyer == null)
            {
                toDestroy = true;
                //log.Info("Destorying");
             
                this.Dispatcher.BeginInvoke(new Action(() => this.Close()));
            }
        }

        private void Window_Closing(object sender, System.ComponentModel.CancelEventArgs e)
        {
            if (!toDestroy && destroyer == null)
            {
                Auxiliary.Logger._log.Warn("Bad closing attempt, flag not set");
                e.Cancel = true;
            }
            else
            {
                //log.Info("Closing window");
            }
        }

        private void Window_Closed(object sender, EventArgs e)
        {

            //log.Info("Window closed");
            try
            {
                //log.Info("Trying to logout");
                //log.Info("Closing factory");
            }
            catch (Exception ex)
            {
                Auxiliary.Logger._log.Error(ex);
            }
        }


        public void SetProgress(int pBarNumber, int newProgress)
        {
            (Panel.Children[pBarNumber] as ProgressBarExt).Value = newProgress;
            if (0 == pBarNumber)
            {
                TaskbarItemInfo.ProgressValue = (double)newProgress / 100;
            }

        }

        public void SetBarsCount(int count)
        {
            Panel.Children.Clear();
            for (int i = 0; i < count; i++)
            {
                var pb = new ProgressBarExt();
                Panel.Children.Add(pb);
            }
        }

        public void SetCaption(String caption)
        {
            this.Title = caption;
        }

        public void SetText(int pBarNumber, String text)
        {
            (Panel.Children[pBarNumber] as ProgressBarExt).Text = text;
        }


        public void HideWindowAfter(int timeout, String message)
        {
            destroyer = new System.Windows.Threading.DispatcherTimer();
            destroyer.Tick += new EventHandler(DestroyTimerEvent);
            destroyer.Interval = new TimeSpan(0, 0, 0, 0, timeout);
            destroyer.Start();

            MessageText.Text = message;
            Message.Visibility = System.Windows.Visibility.Visible;
            // Animate
            DoubleAnimation opacityAnimation = new DoubleAnimation();

            opacityAnimation.Duration = new Duration(TimeSpan.FromSeconds(1.25));
            opacityAnimation.To = 1;

            Storyboard sb = new Storyboard();
            sb.Duration = opacityAnimation.Duration;

            sb.Children.Add(opacityAnimation);

            Storyboard.SetTarget(opacityAnimation, Message);
            Storyboard.SetTargetProperty(opacityAnimation, new PropertyPath("(Border.Opacity)"));

            sb.Begin();
        }

        private void DestroyTimerEvent(object sender, EventArgs e)
        {
            destroyer.Stop();
            this.Close();
        }
    }
}
