using IdentaZone.CollectorServices;
using log4net;
using System;
using System.Collections.Generic;
using System.Linq;
using System.ServiceModel;
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

namespace IdentaZone.CollectorProgressBar
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class ProgressBar : Window, IProgressBarGUICallback
    {
        protected readonly ILog log = LogManager.GetLogger(typeof(ProgressBar));
        Timer Wdt = new System.Timers.Timer();

        //pipe to Server
        IProgressBarGUIService collectorServer = null;
        ChannelFactory<IProgressBarGUIService> pipeFactory;

        List<System.Windows.Controls.ProgressBar> ProgressBarList = new List<System.Windows.Controls.ProgressBar>();
        DispatcherTimer destroyer;
        bool _wdtFinished = false;

        public ProgressBar()
        {
            Auxiliary.Logger.Init();
            log.Info("Const");
            InitializeComponent();
            this.Hide();
            //MouseDown += delegate { DragMove(); };

            String pathToProgram = "";

            try
            {
                pipeFactory = new DuplexChannelFactory<IProgressBarGUIService>(
                    new InstanceContext(this), new NetNamedPipeBinding(),
                    new EndpointAddress("net.pipe://localhost/IdentaZone/Collector/ProgressBarGui"));

                collectorServer = pipeFactory.CreateChannel();

                if (Constants.IS_WDT_ENABLED)
                {
                    log.Info("Starting WDT");
                    Wdt.Elapsed += new ElapsedEventHandler(OnWdt);
                    Wdt.Interval = Constants.KEEP_ALIVE * 10;
                    Wdt.Enabled = true;
                }

                if (!collectorServer.Login())
                {
                    log.Error("Service is busy");
                    this.Close();
                    return;
                }
                else
                {
                    log.Info("Connected to service");
                }

                pathToProgram = collectorServer.GetApplicationDiretctory();
                log.InfoFormat("Acquired path \"{0}\"", pathToProgram);
            }
            catch (Exception ex)
            {
                log.Info(ex);
                this.Close();
                return;
            }

            log.Info("Ready to work");
            collectorServer.Ready();
        }

        private void OnWdt(object sender, ElapsedEventArgs e)
        {
            log.Error("WDT timer alarm!, lost connection to service, closing");
            Destroy();
            _wdtFinished = true;
        }

        public void ShowWindow()
        {
            log.Info("Showing window");


            this.Topmost = true;
            this.Show();
            /*
            var timer = new DispatcherTimer();
            timer.Interval = TimeSpan.FromSeconds(1);
            timer.Tick += TopMostOff;
            timer.Start();*/
        }
        /*
        private void TopMostOff(object sender, EventArgs e)
        {
            DispatcherTimer timer = (DispatcherTimer)sender;
            timer.Stop();
            timer.Tick -= TopMostOff;
            this.Topmost = false;
        }*/

        public void HideWindow()
        {
            log.Info("Hiding window");
            this.Hide();
        }

        bool toDestroy = false;

        public void Destroy()
        {
            if (destroyer == null)
            {
                toDestroy = true;
                log.Info("Destorying");
                if (Constants.IS_WDT_ENABLED)
                {
                    Wdt.Dispose();
                }
                this.Dispatcher.BeginInvoke(new Action(() => this.Close()));
            }
        }

        public void Heartbeat()
        {
            log.Info("Heartbeat");
            Wdt.Stop();
            Wdt.Start();
        }

        private void Window_Closing(object sender, System.ComponentModel.CancelEventArgs e)
        {
            if (!toDestroy && destroyer == null)
            {
                log.Info("Bad closing attempt, flag not set");
                e.Cancel = true;
            }
            else
            {
                log.Info("Closing window");
            }
        }

        private void Window_Closed(object sender, EventArgs e)
        {

            log.Info("Window closed");
            try
            {
                log.Info("Trying to logout");
                collectorServer.Logout();
                log.Info("Closing factory");
                pipeFactory.Close();
            }
            catch (Exception ex)
            {
                log.Error(ex);
            }
        }


        public void SetProgress(int pBarNumber, int newProgress)
        {
            (Panel.Children[pBarNumber] as ProgressBarExt).Value = newProgress;
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

            MessageText.Content = message;
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

        private void Window_MouseDown(object sender, MouseButtonEventArgs e)
        {
            if (e.ChangedButton == MouseButton.Left)
            {
                DragMove();
            }
        }
    }
}
