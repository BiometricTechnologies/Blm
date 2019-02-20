using IdentaZone.Collector;
using IdentaZone.IZService;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

namespace ProgressBarIntegration
{
    class Tester
    {
        ProgressBarClient CurrentClient = null;
        List<ProgressBarClient> ClientList = new List<ProgressBarClient>();

        int CurrentBar = 0;
        internal static int BARS_COUNT = 2;

        public Tester()
        {
            Task.Factory.StartNew(() =>
           {
               var izService = new IZService();
           });
            
        }

        internal void AddClient()
        {
            try
            {
                progress = 0;
                CurrentClient = new ProgressBarClient();
                ClientList.Add(CurrentClient);
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex);
            }
        }

        internal void Init()
        {
            try
            {
                var time = System.Environment.TickCount;
                CurrentClient.Init();
                while (CurrentClient.IsReady() != IdentaZone.ReturnTypes.rtOK)
                {
                    Thread.Sleep(1);
                }
                time = System.Environment.TickCount - time;
                Console.WriteLine("Init has taken {0} ticks", time);
                CurrentClient.SetBarsCount(BARS_COUNT);
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex);
            }
        }

        internal void ShowClientCount()
        {
            Console.WriteLine("Current client count: " + ClientList.Count);
        }

        internal void SelectClient(int number)
        {
            try
            {
                CurrentClient = ClientList[number];
            }
            catch (Exception ex)
            {
                Console.WriteLine("Error " + ex);
            }
        }

        internal void ShowProgressBar()
        {
            try
            {
                CurrentClient.ShowDialog();
            }
            catch (Exception ex)
            {
                Console.WriteLine("Error " + ex);
            }
        }

        internal void KillClientAfter()
        {
            try
            {
                ClientList.Remove(CurrentClient);
                CurrentClient.CloseDialogAfter(3000,"BAZINGA");
                CurrentClient.Dispose();
                CurrentClient = null;
                Console.WriteLine("Killed current client");
                if (ClientList.Count > 0)
                {
                    CurrentClient = ClientList[ClientList.Count - 1];
                }
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex);
            }

        }

        internal void KillClient()
        {
            try
            {
                ClientList.Remove(CurrentClient);
                CurrentClient.CloseDialog();
                CurrentClient.Dispose();
                CurrentClient = null;
                Console.WriteLine("Killed current client");
                if (ClientList.Count > 0)
                {
                    CurrentClient = ClientList[ClientList.Count - 1];
                }
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex);
            }
        }

        int progress = 0;
        internal void UpdateProgress()
        {
            try
            {
                CurrentClient.SetProgress(CurrentBar,progress);
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex);
            }
        }

        public int CurrentProgress
        {
            get
            {
                return progress;
            }
            set
            {
                progress = value;
                UpdateProgress();
            }
        }

        internal void UpdateText()
        {
            try
            {
                CurrentClient.SetText(CurrentBar, progress.ToString());
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex);
            }
        }

        internal void NextBar()
        {
            CurrentBar = (CurrentBar + 1) % BARS_COUNT;
        }

        internal void SetCaption()
        {
            try
            {
                CurrentClient.SetCaption(String.Format("Current bar is {0} current progress is {1}", CurrentBar, progress));
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex);
            }
        }

       
    }
}
