using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Runtime.InteropServices;
using System.Windows.Forms;
namespace IdentaZone.IdentaMaster.Logic
{
    class SessionChangeHandler:Control
    {
        [DllImport("WtsApi32.dll")]
        private static extern bool WTSRegisterSessionNotification(IntPtr hWnd, [MarshalAs(UnmanagedType.U4)]int dwFlags);
        [DllImport("WtsApi32.dll")]
        private static extern bool WTSUnRegisterSessionNotification(IntPtr hWnd);

        private const int NOTIFY_FOR_THIS_SESSION = 0;
        private const int WM_WTSSESSION_CHANGE = 0x2b1;
        private const int WTS_CONSOLE_CONNECT = 0x1; // A session was connected to the console terminal.
        private const int WTS_CONSOLE_DISCONNECT = 0x2; // A session was disconnected from the console terminal.
        private const int WTS_REMOTE_CONNECT = 0x3; // A session was connected to the remote terminal.
        private const int WTS_REMOTE_DISCONNECT = 0x4; // A session was disconnected from the remote terminal.
        private const int WTS_SESSION_LOGON = 0x5; // A user has logged on to the session.
        private const int WTS_SESSION_LOGOFF = 0x6; // A user has logged off the session.
        private const int WTS_SESSION_LOCK = 0x7; // A session has been locked.
        private const int WTS_SESSION_UNLOCK = 0x8; // A session has been unlocked.
        private const int WTS_SESSION_REMOTE_CONTROL = 0x9; // A session has changed its remote controlled status.
        public event EventHandler MachineLocked;
        public event EventHandler MachineUnlocked;

        public SessionChangeHandler()
        {
            if (!WTSRegisterSessionNotification(this.Handle, NOTIFY_FOR_THIS_SESSION))
            {
                Marshal.ThrowExceptionForHR(Marshal.GetLastWin32Error());
            }
        }

        protected override void OnHandleDestroyed(EventArgs e)
        {
            // unregister the handle before it gets destroyed
            WTSUnRegisterSessionNotification(this.Handle);
            base.OnHandleDestroyed(e);
        }

        protected override void WndProc(ref Message m)
        {
            if (m.Msg == WM_WTSSESSION_CHANGE)
            {
                int value = m.WParam.ToInt32();
                MessageBox.Show(value.ToString());
                if (value == WTS_SESSION_LOCK)
                {
                    OnMachineLocked(EventArgs.Empty);
                }
                else if (value == WTS_SESSION_UNLOCK)
                {
                    OnMachineUnlocked(EventArgs.Empty);
                }
            }
            base.WndProc(ref m);
        }

        protected virtual void OnMachineLocked(EventArgs e)
        {
            MessageBox.Show("locked");
            //EventHandler temp = myMachineLockedHandler;
            //if (temp != null)
            //{
            //    temp(this, e);
            //}
        }

        protected virtual void OnMachineUnlocked(EventArgs e)
        {
            MessageBox.Show("unlocked");
            //EventHandler temp = myMachineUnlockedHandler;
            //if (temp != null)
            //{
            //    temp(this, e);
            //}
        }
    }
}
