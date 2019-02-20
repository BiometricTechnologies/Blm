using log4net;
using System;
using System.Collections;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace IdentaZone.IdentaMaster
{
    class LogRecord : IComparable
    {

        private String _message;
        private DateTime _time;

        private LogRecord(String message, DateTime time)
        {
            _message = message;
            _time = time;
        }

        public String GetMessage()
        {
            return _message;
        }

        public DateTime GetTime()
        {
            return _time;
        }
        static public LogRecord Parse(String line)
        {
            try
            {
                string[] split = line.Split(';');
                if (split != null && split.Length == 2)
                {
                    System.DateTime time = new DateTime(1970, 1, 1, 0, 0, 0, 0, DateTimeKind.Utc);
                    time = time.AddSeconds(Convert.ToDouble(split[1]));
                    time = time.ToLocalTime();
                    return new LogRecord(split[0], time);
                }
            }
            finally
            {
            }
            return null;
        }


        public int CompareTo(Object obj)
        {

            return _time.CompareTo((obj as LogRecord)._time);
        }
    }
    class LogReader
    {

        private readonly ILog Log = LogManager.GetLogger(typeof(LogReader));

        public List<LogRecord> records = new List<LogRecord>();
        private String _fileName;
        public LogReader(String fileName, int rotateCount)
        {
       //     Log.InfoFormat("Looking up for {0} log family", fileName);
            _fileName = fileName;
            String iterFilename = _fileName;
            try
            {
                for (int i = 1; i <= rotateCount; i++)
                {
                    if (File.Exists(iterFilename))
                    {
                        using (FileStream fs = new FileStream(iterFilename, FileMode.Open, FileAccess.Read, FileShare.ReadWrite))
                        {
                            using (StreamReader reader = new StreamReader(fs))
                            {
                                String line;
                                while (!reader.EndOfStream)
                                {
                                    line = reader.ReadLine();
                                    LogRecord newRecord = LogRecord.Parse(line);
                                    if (newRecord != null)
                                    {
                                        records.Add(newRecord);
                                    }
                                }
                            }
                        }
                        iterFilename = _fileName + i.ToString();
                    }
                    else
                    {
                     //   Log.Info("File not exists " + iterFilename);
                    }
                }
            }
            catch (Exception ex)
            {
                Log.ErrorFormat("Can't get file {0} with error {1}", iterFilename, ex);
            }
        }


        public String GetLog()
        {
            return null;
        }
    }
}
