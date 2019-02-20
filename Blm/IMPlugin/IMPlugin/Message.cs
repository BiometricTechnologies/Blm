using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

namespace IdentaZone.IMPlugin
{
    public static class Ambassador
    {
        private static BlockingCollection<Message> queue = new BlockingCollection<Message>();
        private static CancellationTokenSource cancelToken = new CancellationTokenSource();

        private static Action<Message> callback_;

        private static readonly Object cbcLock = new object();
        private static Task poller;

        static public void SetCallback(Action<Message> callback)
        {
            lock (cbcLock)
            {
                callback_ = callback;
            }
        }

        static public void ClearCallback()
        {
            lock (cbcLock)
            {
                callback_ = null;
            }
        }

        static Ambassador()
        {
            poller = Task.Factory.StartNew(() =>
            {
                PollerTask();
            });
        }

        public static void Dispose()
        {
            cancelToken.Cancel();
        }

        /// <summary>
        /// Put Message into queue
        /// </summary>
        /// <param name="msg">The Message.</param>
        public static void AddMessage(Message msg)
        {
            queue.Add(msg);
        }

        private static void PollerTask()
        {
            try
            {
                while (true)
                {
                    Message msg = queue.Take(cancelToken.Token);
                    Action<Message> callback = null;
                    lock (cbcLock)
                    {
                        callback = callback_;
                    }
                    if (callback != null)
                    {
                        try
                        {
                            callback(msg);
                        }
                        catch (Exception ex)
                        {
                            System.Console.WriteLine(ex);
                        }
                    }
                }
            }
            finally
            {
            }
        }
    }

    /// <summary>
    /// Message allows plugin interact with MainApp
    /// </summary>
    abstract public class Message
    {
        /// <summary>
        /// Gets or sets the Sender.
        /// </summary>
        /// <value>
        /// The Sender.
        /// </value>
        public object Sender { get; set; }
    }


    /// <summary>
    /// This Message contains device failure Message
    /// </summary>
    public class MSignal : Message
    {
        /// <summary>
        /// DEVICE_FAILURE - device cannot be futher normally operated
        /// </summary>
        public enum SIGNAL { DEVICE_FAILURE };

        /// <summary>
        /// Gets or sets the signal.
        /// </summary>
        /// <value>
        /// The signal.
        /// </value>
        public SIGNAL Signal { get; set; }

        /// <summary>
        /// Initializes a new instance of the <see cref="MSignal"/> class.
        /// </summary>
        /// <param name="Sender">The Sender.</param>
        /// <param name="Signal">The signal.</param>
        public MSignal(Object sender, SIGNAL signal)
        {
            this.Sender = sender;
            this.Signal = signal;
        }
    }

    /// <summary>
    /// This Message is generated while enrollment
    /// And in Identify phase on enrollment panel
    /// </summary>
    public class MBiometricsSingleCaptured : Message
    {
        /// <summary>
        /// Image (RAW data)
        /// </summary>
        public FingerImage Image { get; set; }

        /// <summary>
        /// Initializes a new instance of the <see cref="MBiometricsSingleCaptured"/> class.
        /// </summary>
        /// <param name="Sender">The Sender.</param>
        /// <param name="Image">The Image.</param>
        public MBiometricsSingleCaptured(Object sender, FingerImage image)
        {
            this.Sender = sender;
            this.Image = image;
        }
    }


    /// <summary>
    /// Message with List of enrolled teplates
    /// Is generate as result of Enrollment
    /// If enrollment fails - Biometrics should be set to "null"
    /// </summary>
    public class MBiometricsEnrolled : Message
    {
        /// <summary>
        /// Gets or sets the Biometrics.
        /// </summary>
        /// <value>
        /// The Biometrics. If enrollment failed - send "null"
        /// </value>
        public List<FingerTemplate> Biometrics { get; set; }


        /// <summary>
        /// Initializes a new instance of the <see cref="MBiometricsEnrolled"/> class.
        /// </summary>
        /// <param name="Sender">The Sender.</param>
        /// <param name="Biometrics">The Biometrics. If enrollmed failed - send "null"</param>
        public MBiometricsEnrolled(Object sender, List<FingerTemplate> bios)
        {
            this.Sender = sender;
            this.Biometrics = bios;
        }

    }

    /// <summary>
    /// Message with live-captured Image
    /// This Type of images are generated in TEST mode
    /// </summary>
    public class MBiometricsLiveCaptured : Message
    {
        /// <summary>
        /// Gets or sets the Image.
        /// </summary>
        /// <value>
        /// The Image.
        /// </value>
        public FingerImage Image { get; set; }


        /// <summary>
        /// Initializes a new instance of the <see cref="MBiometricsLiveCaptured"/> class.
        /// </summary>
        /// <param name="Sender">The Sender.</param>
        /// <param name="Image">The Image.</param>
        public MBiometricsLiveCaptured(Object sender, FingerImage image)
        {
            this.Sender = sender;
            this.Image = image;
        }
    }

    /// <summary>
    /// This Message updates progress bar on enrollment pane
    /// </summary>
    public class MUpdateProgress : Message
    {
        /// <summary>
        /// RESET - return progress bar to zero
        /// SET - set current progress bar value 0..100
        /// </summary>
        public enum ACTION { RESET, SET };
        private ACTION ActionValue;

        /// <summary>
        /// Gets the action.
        /// </summary>
        /// <value>
        /// The action.
        /// </value>
        public ACTION Action { get { return ActionValue; } }
        private int ValueValue;

        /// <summary>
        /// Gets the value.
        /// </summary>
        /// <value>
        /// The value.
        /// </value>
        public int Value { get { return ValueValue; } }

        /// <summary>
        /// Create new instanse of Message, of Type : SET
        /// </summary>
        /// <param name="value">Value to set 0..100</param>
        /// <returns></returns>
        public static MUpdateProgress Set(object sender, int value)
        {
            MUpdateProgress result = new MUpdateProgress();
            result.Sender = sender;
            result.ActionValue = ACTION.SET;
            result.ValueValue = value;
            return result;
        }

        /// <summary>
        /// Create new instanse of Message, of Type RESET
        /// </summary>
        /// <returns></returns>
        public static MUpdateProgress Reset()
        {
            MUpdateProgress result = new MUpdateProgress();
            result.ActionValue = ACTION.RESET;
            result.ValueValue = 0;
            return result;
        }


    }

    /// <summary>
    /// This class represents Message with text to display
    /// </summary>
    public class MShowText : Message
    {
        /// <summary>
        /// BASE - Message is shown to user until next BASE Message arrives
        /// POPUP - Message is shown to user for limited time, after it prev. BASE Message is shown
        /// </summary>
        public enum TYPE { BASE, POPUP };
        /// <summary>
        /// Type of this Message
        /// </summary>
        public TYPE Type { get; set; }
        /// <summary>
        /// Text of this Message
        /// </summary>
        public String Message { get; set; }

        /// <summary>
        /// Create new Text Message, which would be displayed to user
        /// </summary>
        /// <param name="Sender">Who is sending this Message?</param>
        /// <param name="Message">Text to display</param>
        /// <param name="Type">Type of Message</param>
        public MShowText(Object sender, String message, TYPE type)
        {
            this.Sender = sender;
            this.Message = message;
            this.Type = type;
        }

        /// <summary>
        /// Create new Text Message, which would be displayed to user.
        /// TYPE would be set to TYPE.BASE
        /// </summary>
        /// <param name="Sender"></param>
        /// <param name="Message"></param>
        public MShowText(Object sender, String message)
        {
            this.Sender = sender;
            this.Message = message;
            this.Type = TYPE.BASE;
        }

    }
}
