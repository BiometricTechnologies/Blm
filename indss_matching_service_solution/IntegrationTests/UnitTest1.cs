using System;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using IdentaZone.Indss.MatchingService;
using System.ServiceModel;
using System.ServiceModel.Description;
using System.ServiceModel.Web;

namespace IntegrationTests
{
    [TestClass]
    public class UnitTest1
    {/*
        static MatchingServiceHost matchingService;

        [ClassInitialize]
        public static void Init(TestContext context)
        {
            matchingService = new MatchingServiceHost();
            
        }

        [TestMethod]
        public void CheckEcho()
        {
            using (ChannelFactory<MatchingServiceInterfaceChannel> scf = new ChannelFactory<MatchingServiceInterfaceChannel>(new BasicHttpBinding(), "http://localhost:8001/MatchingService/Soap"))
            {
                try
                {
                    MatchingServiceInterfaceChannel channel = scf.CreateChannel();

                    Console.WriteLine("Calling EchoWithGet on SOAP endpoint: ");
                    var respone = channel.Echo(new EchoRequest("Hello, world"));
                    Assert.AreEqual("Hello, world", respone.output);
                }
                catch (Exception ex)
                {
                    System.Console.WriteLine(ex);
                    Assert.Fail(ex.ToString());
                }
            }
        }*/
    }
}
