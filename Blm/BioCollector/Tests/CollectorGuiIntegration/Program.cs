using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CollectorGuiIntegration
{
    class Program
    {
        static void ShowMenu()
        {
            Console.WriteLine("1 Show new GUI");
            Console.WriteLine("5 Login user");
            Console.WriteLine("6 Identification fail");
            Console.WriteLine("7 Show Wrong user");
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
                        tester.ShowNewGUI();
                        break;
                    case "6":
                        tester.IdentificationFail();
                        break;
                    case "5":
                        tester.LoginUser();
                        break;
                    case "7":
                        tester.WrongUser();
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
