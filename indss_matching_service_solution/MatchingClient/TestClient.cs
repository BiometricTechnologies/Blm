using IdentaZone.Indss.MatchingService.MatchingServiceReference;
using System;
using System.Collections.Generic;
using System.Linq;
using System.ServiceModel;
using System.Text;
using System.Threading.Tasks;

namespace IdentaZone.Indss.MatchingService
{
    public class TestClient
    {
        MatchingServiceClient _proxy;

        public TestClient()
        {
            var binding = new BasicHttpBinding();
            binding.Name = "BasicHttpBinding_IMatchingService";
            _proxy = new MatchingServiceClient(binding, new EndpointAddress("http://localhost:8001/MatchingService/soap/"));
            
        }

        public String Echo(String input)
        {
            return _proxy.Echo(input);
        }

        public String Match(String pin, String login, int bsp, byte[] template)
        {
            return _proxy.Match(new matchRequest()
            {
                Pin = pin,
                Bsp_Code = bsp,
                Template = template,
                LoginName = login
            });
        }

        public String GetProvidersList()
        {
            return _proxy.GetBspProvidersList(null);
        }


        //ECHO-PC
        //public string ConnectDB()
        //{
        //    _proxy.ConnectDB(new connectDBRequest()
        //    {
        //        Database = "indss",
        //        Password = "root",
        //        Port = "3306",
        //        Server = "localhost",
        //        Uid = "root"
        //    });
        //    return "Connected DB";
        //}


        //ILIYA-PC
        public string ConnectDB()
        {
            _proxy.ConnectDB(new connectDBRequest()
            {
                Database = "indss",
                Password = "root",
                Port = "3306",
                Server = "localhost",
                Uid = "root"
            });
            return "Connected DB";
        }

    }
}
