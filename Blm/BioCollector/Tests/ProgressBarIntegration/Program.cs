using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ProgressBarIntegration
{
    class Program
    {

        static void ShowMenu()
        {
            Console.WriteLine("1) Create new ProgressBarClient");
            Console.WriteLine("2) Init client");
            Console.WriteLine("3) Show progress bar");
            Console.WriteLine("4) Current progress ++");
            Console.WriteLine("5) Current progress --");
            Console.WriteLine("6) Update text");
            Console.WriteLine("7) Set caption");
            Console.WriteLine("9) Kill after 3 sec");
            Console.WriteLine("0) Kill client");
            Console.WriteLine("t) next bar");
            Console.WriteLine("c Clear Console");    
        }

        static void Main(string[] args)
        {
            Tester tester = new Tester();
            ConsoleKeyInfo cki;
            do
            {
                ShowMenu();
                cki = Console.ReadKey(false); // show the key as you read it
                switch (cki.KeyChar.ToString())
                {
                    case "1":
                        tester.AddClient();
                        break;
                    case "2":
                        tester.Init();
                        break;
                    case "3":
                        tester.ShowProgressBar();
                        break;
                    case "4":
                        tester.CurrentProgress++;
                        break;
                    case "5":
                        tester.CurrentProgress--;
                        break;
                    case "0":
                        tester.KillClient();
                        break;
                    case "6":
                        tester.UpdateText();
                        break;
                    case "7":
                        tester.SetCaption();
                        break;
                    case "9":
                        tester.KillClientAfter();
                        break;
                    case "t":
                    case "T":
                        tester.NextBar();
                        break;
                    case "c":
                    case "C":
                        Console.Clear();
                        break;
                    // etc..
                }
            } while (cki.Key != ConsoleKey.Escape);
        }
    }
}
