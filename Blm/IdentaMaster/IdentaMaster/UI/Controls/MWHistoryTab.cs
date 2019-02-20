using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Documents;

namespace IdentaZone.IdentaMaster
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        private static int LogRotateCount = 2;

        /// <summary>
        /// Updates the event list.
        /// </summary>
        private void updateEventList()
        {
            List<LogRecord> sortingQueue = new List<LogRecord>();
            try
            {
                LogReader reader = new LogReader(System.IO.Path.Combine(Environment.SystemDirectory, "IdentaZone\\singlelogin.log"), LogRotateCount);
                foreach (LogRecord record in reader.records)
                {
                    sortingQueue.Add(record);
                }
                reader = new LogReader(System.IO.Path.Combine(Environment.SystemDirectory, "IdentaZone\\multilogin.log"), LogRotateCount);
                foreach (LogRecord record in reader.records)
                {
                    sortingQueue.Add(record);
                }

                reader = new LogReader(System.IO.Path.Combine(Environment.SystemDirectory, "IdentaZone\\BiosecureHistory.log"), LogRotateCount);
                //reader = new LogReader(System.IO.Path.Combine("C:\\Logs", "IdentaZone\\BiosecureHistory.log"), LogRotateCount);
                foreach (LogRecord record in reader.records)
                {
                    sortingQueue.Add(record);
                }
            }
            catch (Exception ex)
            {
                Log.Error("can't load logs " + ex);
            }
            sortingQueue.Sort();
            sortingQueue.Reverse();
            foreach (LogRecord record in sortingQueue)
            {
                LogView.Items.Add(new { Date = GetDate(record.GetTime()), Message = record.GetMessage() });
            }
        }

        public static String GetDate(DateTime date)
        {
            return date.ToShortDateString() + " " + date.ToLongTimeString();
        }

        private void LogView_SelectionChanged(object sender, System.Windows.Controls.SelectionChangedEventArgs e)
        {
            if (null != sender && sender is ListBox)
            {
                var lv = sender as ListBox;
                lv.SelectedIndex = -1;
            }
        }
    }
}