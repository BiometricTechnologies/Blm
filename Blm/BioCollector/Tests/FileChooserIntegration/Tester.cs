using IdentaZone.Collector;
using IdentaZone.IZService;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

namespace FileChooserIntegration
{
    class Tester
    {

        public Tester()
        {
            Task.Factory.StartNew(() =>
                {
                    var izService = new IZService();
                }
            );
        }

        CollectorClient client = null;

        internal void AddClient()
        {
            try
            {
                client = new CollectorClient();
                var time = System.Environment.TickCount;
                client.Init();
                while (client.IsReady() != IdentaZone.ReturnTypes.rtOK)
                {
                    Thread.Sleep(100);
                }
                time = System.Environment.TickCount - time;
                Console.WriteLine("Init has taken {0} ticks", time);

                client.AddCryptoProvider("MS CRYPTO");
                client.AddCryptoProvider("SuperBlowFish");
                //client.ShowDialog();
                client.ShowDialogEx(new IdentaZone.CollectorDialogData() { Filename = "test.iz", Username = "NOTORIOUS BIOSEC" });
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex);
            }
        }

        internal void SwitchToFileMode()
        {
            client.UpdateState(IdentaZone.StateTypes.stShowFileList);
        }


        int fileNum;
        internal void AddRandomFile()
        {
            IdentaZone.FileInfoStruct fileInfo = new IdentaZone.FileInfoStruct();
            fileInfo.FileNumber = fileNum++;
            fileInfo.Filename = System.IO.Path.GetRandomFileName();
            fileInfo.FileSize = 555;
            fileInfo.ModificationDate = (long)(DateTime.UtcNow.Subtract(new DateTime(1970, 1, 1))).TotalSeconds;
            client.AddFile(fileInfo);
        }

        internal void AddRandomFolder()
        {
            var folderName = Path.GetFileNameWithoutExtension(Path.GetRandomFileName());
            var fileInfo = new IdentaZone.FileInfoStruct() { FileNumber = fileNum++, Filename = Path.Combine(folderName, Path.GetRandomFileName()) };
            client.AddFile(fileInfo);
        }

        internal IdentaZone.FileInfoStruct NewFile(String path)
        {
            return new IdentaZone.FileInfoStruct()
            {
                FileNumber = fileNum++,
                Filename = path,
                FileSize = 666
            };
        }

        internal void Populate()
        {   
            client.AddFile(NewFile("Readme.txt"));
            client.AddFile(NewFile(Path.Combine("Documents","review.xls")));
            client.AddFile(NewFile(Path.Combine("Documents", "datasheet.doc")));
            client.AddFile(NewFile(Path.Combine("Music", "New","shopen.mp3")));
            client.AddFile(NewFile(Path.Combine("Music", "ringtone.mp3")));
        }

        internal void CheckStatus()
        {
            var status = client.PullFileAction();
            Console.WriteLine("Status: " + status);
        }

        internal void GetList()
        {
            int elemCount = 1;
            IntPtr fileNumArray = Marshal.AllocCoTaskMem(elemCount);
            var res = client.GetSelectedFiles(ref elemCount,fileNumArray);
            Console.WriteLine("Status {0} elemCount {1} Prt {2}", res, elemCount, fileNumArray);
            var elem = Marshal.ReadInt64(fileNumArray);
            Console.WriteLine("First element is {0}", elem);
            Marshal.FreeCoTaskMem(fileNumArray);
        }

    }
}
