using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace FileChooserIntegration
{
    class Program
    {
        static void ShowMenu()
        {
            Console.WriteLine("1) Create new GUI");
            Console.WriteLine("2) Switch to filemode");
            Console.WriteLine("3) Add Random file");
            Console.WriteLine("4) Add Randon sub-directory file");
            Console.WriteLine("5) Populate");
            Console.WriteLine("9) Check FileStatus");
            Console.WriteLine("0) Get file list");

            Console.WriteLine("c Clear console");
        }

        static void Main(string[] args)
        {
            Tester tester = new Tester();
            ConsoleKeyInfo cki;
            do
            {
                ShowMenu();
                cki = Console.ReadKey(false);
                switch (cki.KeyChar.ToString())
                {
                    case "1":
                        tester.AddClient();
                        break;
                    case "2":
                        tester.SwitchToFileMode();
                        break;
                    case "3":
                        tester.AddRandomFile();
                        break;
                    case "4":
                        tester.AddRandomFolder();
                        break;
                    case "5":
                        tester.Populate();
                        break;
                    case "9":
                        tester.CheckStatus();
                        break;
                    case "0":
                        tester.GetList();
                        break;
                    case "c":
                    case "C":
                        Console.Clear();
                        break;
                }
            } while (cki.Key != ConsoleKey.Escape);
        }
    }
}
