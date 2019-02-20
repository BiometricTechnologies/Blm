using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace IdentaZone.IZService
{
    public interface EventLogger
    {
        void Log(String message);
    }

    class SimpleEventLogger : EventLogger
    {
        const String BaseFilename = "BiosecureHistory.log";
        String PathToLog;
        String Filename;
        int MAX_FILE_LENGTH = 1024 * 1024;


        protected void RotateLogs()
        {
            if (File.Exists(Filename))
            {
                var len = new FileInfo(Filename).Length;
                if (len > MAX_FILE_LENGTH)
                {
                    File.Copy(Filename, Path.Combine(Filename, "1"), true);
                }
            }
        }

        public SimpleEventLogger()
        {
            PathToLog = Path.Combine(Environment.SystemDirectory, "IdentaZone");
            Filename = BaseFilename;
            RotateLogs();
        }

        public void Log(String message)
        {
            int unixTime = (int)(DateTime.UtcNow - new DateTime(1970, 1, 1)).TotalSeconds;
            String result = String.Format("{0};{1}{2}", message, unixTime, Environment.NewLine);
            File.AppendAllText(Path.Combine(PathToLog, Filename), result);            
        }
    }
}
