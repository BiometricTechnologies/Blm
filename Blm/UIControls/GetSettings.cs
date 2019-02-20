namespace UIControlsINDSS
{
    public class GetSettings
    {
        public static string Server
        {
            get { return Properties.Settings.Default.ServerName; }
        }
        public static string Port
        {
            get { return Properties.Settings.Default.Port; }
        }

    }
}
