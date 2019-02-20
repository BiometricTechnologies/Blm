using log4net;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Xml;
using System.Xml.Serialization;
using csharp_aes_encryptor;
namespace IdentaZone.IdentaMaster
{
    public class XmlDB : IUserDB
    {
        private readonly ILog Log = LogManager.GetLogger(typeof(XmlDB));

        public enum LOGIN_TYPE { PASS = 0, BIO = 1, MIXED = 2 };
        public static string[] LOGIN_STRING = new string[3] { "Password", "Biometrics", "Password OR Biometrics" };
        private XmlData _data;
        private String _path;
        private byte[] entropy = { 0x00, 0x01, 0x02, 0x01, 0x00, 0x04, 0xBB, 0x17, 0x8b, 0xf6, 0xa2, 0x15, 0xe2, 0x64, 0x11, 0x9a };

        public void Init(String dbPath)
        {
            if (!File.Exists(dbPath))
            {
                using (FileStream fs = new FileStream(dbPath, FileMode.Create))
                {
                    try
                    {
                        MemoryStream unprotectedFs = new MemoryStream();
                        XmlSerializer xs = new XmlSerializer(typeof(XmlData));
                        XmlData data = new XmlData();
                        xs.Serialize(unprotectedFs, data);
                        Encryptor en = new AesEncryptor();
                        byte[] input = unprotectedFs.ToArray();
                        byte[] newArray = new byte[(16 - input.Length % 16) + input.Length];
                        input.CopyTo(newArray, 0);
                        input = newArray;
                        byte[] protectedFs = en.Encrypt(input, entropy);
                        fs.Write(protectedFs, 0, protectedFs.Length);
                        fs.Flush();
                        unprotectedFs.Dispose();
                        _data = data;
                        _path = dbPath;
                    }
                    catch (Exception e)
                    {
                        Log.Error(e);
                    }
                }
            }
            else
            {
                using (FileStream fs = new FileStream(dbPath, FileMode.Open))
                {
                    try
                    {
                        Encryptor en = new AesEncryptor();
                        Stream unprotectedFs = StreamCryptor.Decrypt(en, fs, entropy);
                        XmlSerializer xs = new XmlSerializer(typeof(XmlData));
                        XmlData data = (XmlData)xs.Deserialize(unprotectedFs);
                        _data = data;
                        _path = dbPath;
                    }
                    catch (Exception e)
                    {
                        Log.Error(e);
                    }
                }
            }
        }

        private void DeployFile(String fileName)
        {
            // deploy dll
            try
            {
                String destination = Environment.SystemDirectory + "\\" + fileName;
                if (File.Exists(destination))
                {
                    File.SetAttributes(destination, FileAttributes.Normal);
                }
                File.Copy("login\\" + fileName, destination, true);
            }
            catch (Exception e)
            {
                Log.Error("Can't copy file to system folder", e);
            }
        }

        private void DeployFileInDir(String fileName, String dirName)
        {
            // deploy dll
            try
            {
                String destination = Environment.SystemDirectory + "\\" + dirName + "\\" + fileName;
                if (File.Exists(destination))
                {
                    File.SetAttributes(destination, FileAttributes.Normal);
                }
                File.Copy("login\\" + fileName, destination, true);
            }
            catch (Exception e)
            {
                Log.Error("Can't copy file to system folder", e);
            }
        }

        public void Deploy(Dictionary<string,string> deploymentList)
        {
            // directories in System32
            Directory.CreateDirectory(Environment.SystemDirectory + "\\IdentaZone");
            Directory.CreateDirectory(Environment.SystemDirectory + "\\IdentaZone" + "\\IdentaMaster");
            // empty lock file
            try
            {
                using (File.Create(Environment.SystemDirectory + "\\IdentaZone\\lock.txt")) { };
            }
            catch (Exception e)
            {
                Log.Error("Can't create system directory", e);
            }
            
            DeployFile("DPlogin.dll");
            DeployFile("SingleLogin.dll");
            DeployFile("libglog.dll");
            DeployFile("libapr-1.dll");
            DeployFile("libapriconv-1.dll");
            foreach (var pair in deploymentList)
            {
                if (pair.Value != "")
                {
                    DeployFileInDir(pair.Value, "IdentaZone\\IdentaMaster");
                }
            }

            // add registry key
            Microsoft.Win32.RegistryKey key;
            // DPLogin.dll
            key = Microsoft.Win32.Registry.LocalMachine.CreateSubKey("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Authentication\\Credential Providers\\{46409ad9-942c-4fba-a11f-673bb6e31cf4}");
            key.SetValue("", "DPLogin");
            key.Close();
            key = Microsoft.Win32.Registry.LocalMachine.CreateSubKey("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Authentication\\Credential Provider Filters\\{46409ad9-942c-4fba-a11f-673bb6e31cf4}");
            key.SetValue("", "DPLogin");
            key.Close();
            key = Microsoft.Win32.Registry.ClassesRoot.CreateSubKey("CLSID\\{46409ad9-942c-4fba-a11f-673bb6e31cf4}");
            key.SetValue("", "DPLogin");
            key.Close();
            key = Microsoft.Win32.Registry.ClassesRoot.CreateSubKey("CLSID\\{46409ad9-942c-4fba-a11f-673bb6e31cf4}\\InprocServer32");
            key.SetValue("", "DPLogin.dll");
            key.SetValue("ThreadingModel", "Apartment");
            key.Close();
            // SingleLogin.dll
            key = Microsoft.Win32.Registry.LocalMachine.CreateSubKey("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Authentication\\Credential Providers\\{C0D9D8DE-C8A2-4533-8146-172136A20F2E}");
            key.SetValue("", "SingleLogin");
            key.Close();
            key = Microsoft.Win32.Registry.ClassesRoot.CreateSubKey("CLSID\\{C0D9D8DE-C8A2-4533-8146-172136A20F2E}");
            key.SetValue("", "SingleLogin");
            key.Close();
            key = Microsoft.Win32.Registry.ClassesRoot.CreateSubKey("CLSID\\{C0D9D8DE-C8A2-4533-8146-172136A20F2E}\\InprocServer32");
            key.SetValue("", "SingleLogin.dll");
            key.SetValue("ThreadingModel", "Apartment");
            key.Close();
            // add DB
            saveData(Environment.SystemDirectory + "\\IdentaZone\\usrdb");
        }



        private bool saveData(String path)
        {
            //  if (!File.Exists(path)) return false; // don't do anything if file doesn't exist        
            using (FileStream fs = new FileStream(path, FileMode.Create))
            {
                XmlSerializer xs = new XmlSerializer(typeof(XmlData));
                MemoryStream unprotectedFs = new MemoryStream();
                try
                {

#if DEBUG ///temporary check the xml output
                    FileStream tmpfs = new FileStream("testdb.xml", FileMode.Create);

                    TextWriter writer = new StreamWriter(tmpfs, new UTF8Encoding());
                    xs.Serialize(writer, _data);
                    writer.Close();
                    ///
#endif
                    xs.Serialize(unprotectedFs, _data);
                    Encryptor en = new AesEncryptor();
                    byte[] input = unprotectedFs.ToArray();
                    byte[] newArray = new byte[(16 - input.Length % 16) + input.Length];
                    input.CopyTo(newArray, 0);
                    input = newArray;
                    byte[] protectedFs = en.Encrypt(input, entropy);
                    fs.Write(protectedFs, 0, protectedFs.Length);
                    fs.Flush();
                    unprotectedFs.Dispose();
                }
                catch (Exception e)
                {
                    Log.Error("Can't save DB", e);
                }
                return true;
            }
        }

        public WinUser GetUser(string sid)
        {
            if (_data != null)
            {
                foreach (WinUser user in _data.users)
                {
                    if (user.SID == sid)
                    {
                        WinUser retUser = (WinUser)user.Clone();
                        return retUser;
                    }
                }
            }
            return null;
        }

        public List<WinUser> GetAllUsers()
        {
            return _data.users;
        }

        public bool AddUser(WinUser newUser)
        {
            foreach (WinUser user in _data.users)
            {
                if (user.SID == newUser.SID)
                {
                    return false;
                }
            }
            _data.users.Add(newUser);
            saveData(_path);
            return true;
        }

        public void RestoreUser(String sid)
        {
            foreach (WinUser user in _data.users.Where(user => user.SID == sid && user.isDeactivated))
            {
                user.Name =user.Name.Substring(3);
                user.isDeactivated = false;
            }
            saveData(_path);
        }


        public void DeactivateUser(String sid)
        {
            foreach (WinUser user in _data.users.Where(user => user.SID == sid))
            {
                user.Name = "@@@" + user.Name;
                user.isDeactivated = true;
            }
            saveData(_path);
        }

        public void RemoveUser(string sid)
        {
            WinUser userToRemove = null;
            foreach (WinUser user in _data.users)
            {
                if (user.SID == sid)
                {
                    userToRemove = user;
                    break;
                }
            }
            if (userToRemove != null)
            {
                _data.users.Remove(userToRemove);
                saveData(_path);
            }
        }

        public bool UpdateUser(WinUser user, bool passwordChanged)
        {
            for (int i = 0; i < _data.users.Count; i++)
            {
                if (_data.users[i].SID == user.SID)
                {
                    if (passwordChanged)
                    {
                        _data.users[i] = user;
                    }
                    else
                    {
                        String pass = _data.users[i].Password;
                        _data.users[i] = user;
                        _data.users[i].Password = pass;
                    }
                    saveData(_path);
                    return true;
                }
            }
            return false;
        }
                
        public List<Credential> GetCredentials(String sid)
        {
            foreach (WinUser user in _data.users)
            {
                if (user.SID == sid)
                {
                    return user.credentials_List;
                }
            }
            return null;
        }

        public bool SetCredentials(String sid, List<Credential> cred_list)
        {
            foreach (WinUser user in _data.users)
            {
                if (user.SID == sid)
                {
                    user.credentials_List = cred_list;
                    saveData(_path);
                    return true;
                }
            }
            return false;
        }

    }

    [XmlRoot("IdentaZoneConfig")]
    public class XmlData
    {
        [XmlArray("WinUsers")]
        [XmlArrayItem("User")]
        public List<WinUser> users;
        public XmlData()
        {
            users = new List<WinUser>();
        }
    }

}
