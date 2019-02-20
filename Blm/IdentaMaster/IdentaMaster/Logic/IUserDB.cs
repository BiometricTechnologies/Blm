using IdentaZone.IMPlugin;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Security.Cryptography;
using System.Text;
using System.Threading.Tasks;
using System.Xml;
using System.Xml.Serialization;

namespace IdentaZone.IdentaMaster
{

    public class WinUser : ICloneable
    {
        public const int _KEY_SIZE = 32;

        public byte[] BioSecureEntropy;
        public String Name;
        public String SID;
        public String Identification;
        public String Password;
        public String FullName;
        public bool isDeactivated;
        public List<Credential> credentials_List { get; set; }
        public override String ToString()
        {
            if (String.IsNullOrEmpty(FullName))
            {
                return Name;
            }
            else
            {
                return FullName;
            }

        }

        public Object Clone()
        {
            return this.MemberwiseClone();
        }
        public WinUser()
        {
            credentials_List = new List<Credential>();
            Identification = "Password";
            Password = "";
            var rng = RandomNumberGenerator.Create();
            BioSecureEntropy = new byte[_KEY_SIZE];
            rng.GetBytes(BioSecureEntropy); 
        }
    }


    /// <summary>
    /// Interface for UserDB
    /// </summary>
    interface IUserDB
    {

        void Init(String dbPath);
        WinUser GetUser(String sid);
        bool AddUser(WinUser newUser);
        bool UpdateUser(WinUser user, bool passwordChanged);
        bool SetCredentials(String sid, List<Credential> credList);
        List<Credential> GetCredentials(String sid);
        List<WinUser> GetAllUsers();
        void Deploy(Dictionary<string,string> deploymentList);

        void RemoveUser(string sid);

        void DeactivateUser(string sid);

        void RestoreUser(string sid);
    }

    [XmlType("credential")]
    [XmlInclude(typeof(PWDCredential)), XmlInclude(typeof(FingerCredential))]
    public abstract class Credential
    {
    }

    public class PWDCredential : Credential
    {
        public String password { get; set; }
    }
    public class PalmCredential : Credential
    {
        private static String[] palmNames = new String[2] { "LEFT PALM", "RIGHT PALM"};
        [XmlAttribute]
        public String device;
        [XmlAttribute]
        public String deviceName;
        [XmlArray("Fingers")]
        [XmlArrayItem("Finger")]
        public List<Palm> Palms { get; set; }

        public PalmCredential()
        {
            Palms = new List<Palm>();
        }
        public PalmCredential(PalmCredential source)
        {
            device = source.device;
            deviceName = source.deviceName;
            Palms = new List<Palm>(source.Palms);
        }
        //public FingerCredential()
        //{
        //    fingers = new List<Finger>();
        //}

    }

    public class FingerCredential : Credential
    {
        private static String[] fingersNames = new String[14] { "LEFT THUMB", "LEFT INDEX", "LEFT MIDDLE", "LEFT RING", "LEFT LITTLE", "RIGHT THUMB", "RIGHT INDEX", "RIGHT MIDDLE", "RIGHT RING", "RIGHT LITTLE", "LEFT PALM", "RIGHT PALM", "LEFT EYE", "RIGHT EYE" };
        private static int[] fingerPos = new int[14] { 4, 3, 2, 1, 0, 5, 6, 7, 8, 9, 10, 11, 12, 13 };
        [XmlAttribute]
        public String device;
        [XmlAttribute]
        public String deviceName;
        [XmlArray("Fingers")]
        [XmlArrayItem("Finger")]
        public List<Finger> fingers { get; set; }

        public FingerCredential()
        {
            fingers = new List<Finger>();
        }
        

        public FingerCredential(FingerCredential source)
        {            
            device = source.device;
            deviceName = source.deviceName;
            fingers = new List<Finger>(source.fingers);
        }


        public String GetFingersList()
        {
            String res = "";
            foreach (var findex in getFingerList())
            {
                if (!res.Contains(fingersNames[findex]))
                {
                    res += fingersNames[findex];
                    res += ", ";
                }
            }

            // Cut last ', '
            if (res != "")
            {
                res = res.Substring(0, res.Length - 2);
            }
            return res;
        }



        public List<int> getFingerList()
        {
            List<int> result = new List<int>();
            foreach (var finger in fingers)
            {
                if (finger.FingerNum >= 10)
                {
                    result.Add(finger.FingerNum);
                }
                else
                {
                    int prevIndex = result.FindIndex(
                        delegate(int findex)
                        {
                            return fingerPos[findex] > fingerPos[finger.FingerNum];
                        });
                    if (prevIndex == -1)
                    {
                        result.Add(finger.FingerNum);
                    }
                    else
                    {
                        result.Insert(prevIndex, finger.FingerNum);
                    }
                }
            }
            return result;
        }

        public void AddFinger(Finger newFinger)
        {
            fingers.Add(newFinger);
        }

        internal void RemoveFinger(int fingerNum)
        {
            fingers.RemoveAll(finger => finger.FingerNum == fingerNum);
        }

        public static String GetFingerName(int index)
        {
            
            if (index > -1 && index < 10)
            {
                return fingersNames[index];
            }
            else if (index >= 10)
            {
                return fingersNames[index];
            }
            else
            {
                return "NONE";
            }
        }
    }

    public class Palm
    {
        [XmlIgnore]
        public FingerTemplate Template { get; set; }

        public Palm()
        {
            PalmNum = 0;
            Bytes = "";
        }
        public int PalmNum { get; set; }
        public String Bytes { get; set; }
        public int Type { get; set; }
        public Palm(int PalmNum, String bytes, FingerTemplate tmpl)
        {
            this.PalmNum = PalmNum;
            this.Bytes = bytes;
            this.Type = tmpl.BSPCode;
            this.Template = tmpl;
        }
    }

    public class Finger
    {
        [XmlIgnore]
        public FingerTemplate Template { get; set; }

        public Finger()
        {
            FingerNum = 0;
            Bytes = "";
        }
        public int FingerNum { get; set; }
        public String Bytes { get; set; }
        public int Type { get; set; }
        public Finger(int fingerNum, String bytes, FingerTemplate tmpl)
        {
            this.FingerNum = fingerNum;
            this.Bytes = bytes;
            this.Type = tmpl.BSPCode;
            this.Template = tmpl;
        }
    }
   
}
