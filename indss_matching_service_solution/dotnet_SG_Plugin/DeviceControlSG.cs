using IdentaZone.IMPlugin;
using SecuGen.FDxSDKPro.Windows;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SG
{
    public class DeviceControlSG : IDeviceControl
    {
        private SGFingerPrintManager m_FPM;
        private readonly List<int> _supportedBSP = new List<int>() { 18, 22, 23 };

        public int BSPCode
        {
            get { return TemplateTypes.SGTemplate; }
        }

        public List<IFingerDevice> ActiveDevices { get; set; }

        public DeviceControlSG()
        {
            ActiveDevices = new List<IFingerDevice>();
        }

        public void Initialize()
        {
            m_FPM = new SGFingerPrintManager();
            var err = m_FPM.InitEx(260,300,500);
        }

        public override string ToString()
        {
            return "SecuGen";
        }

        public void Dispose()
        {
            foreach (var device in ActiveDevices)
            {
                device.Dispose();
            }
        }

        public void EnumerateDevices()
        {
            Int32 iError;

            SGFPMDeviceList[] m_DevList = null; // Used for EnumerateDevice

            // Enumerate Device


            //            return;
            // Get enumeration info into SGFPMDeviceList
            //m_FPM = new SGFingerPrintManager();
            iError = m_FPM.EnumerateDevice();
            m_DevList = new SGFPMDeviceList[m_FPM.NumberOfDevice];


            for (int i = 0; i < m_FPM.NumberOfDevice; i++)
            {
                m_DevList[i] = new SGFPMDeviceList();
                m_FPM.GetEnumDeviceInfo(i, m_DevList[i]);
                if (ActiveDevices.OfType<DeviceSG>().Any(item =>
                    item.devId == m_DevList[i].DevID &&
                    item.devName == m_DevList[i].DevName))
                {
                    continue;
                }

                DeviceSG device = new DeviceSG(m_DevList[i].DevID, m_DevList[i].DevName, "SecuGen " + m_DevList[i].DevName.ToString().Remove(0, 4));

                ActiveDevices.Add(device);
            }

            List<DeviceSG> deleteList = new List<DeviceSG>();
            Dictionary<SGFPMDeviceName, int> devCount = new Dictionary<SGFPMDeviceName, int>();
            foreach (var device in ActiveDevices.OfType<DeviceSG>())
            {
                bool toDelete = true;
                for (int i = 0; i < m_FPM.NumberOfDevice; i++)
                {
                    if (m_DevList[i].DevID == device.devId &&
                        m_DevList[i].DevName == device.devName)
                    {
                        toDelete = false;
                    }
                }
                if (toDelete)
                {
                    device.Dispose();
                    deleteList.Add(device);
                }
            }
            foreach (var device in deleteList)
            {
                ActiveDevices.Remove(device);
            }
            foreach (var device in ActiveDevices.OfType<DeviceSG>())
            {
                var count = 0;
                if (ActiveDevices.OfType<DeviceSG>().Count(item => item.devName == device.devName) > 1)
                {
                    if (!devCount.Keys.Contains(device.devName))
                    {
                        count = 1;
                        devCount.Add(device.devName, 1);
                    }
                    else
                    {
                        count = ++devCount[device.devName];
                    }
                }
                device.count = count;
            }
        }

        public List<int> SupportedBSP
        {
            get { return _supportedBSP; }
        }

        public FingerTemplate Deserialize(int type, byte[] data)
        {
            FingerTemplate result = null;
            if (SupportedBSP.Contains(type))
            {
                result = new TemplateSG(type, data);
            }
            return result;
        }

        public int Match(FingerTemplate template, IEnumerable<FingerTemplate> candidates,out List<FingerTemplate> matches)
        {
            TemplateSG templateSG = template as TemplateSG;
            int result = m_FPM.SetTemplateFormat(SGFPMTemplateFormat.ANSI378);
            matches = new List<FingerTemplate>();
            foreach (var canditate in candidates.OfType<TemplateSG>())
            {
                if (canditate.BSPCode != template.BSPCode)
                {
                    continue;
                }
                bool matched = false;
                result = m_FPM.MatchTemplate(templateSG.Bytes, canditate.Bytes, SGFPMSecurityLevel.HIGH, ref matched);
                if (matched)
                {
                    matches.Add(canditate);
                }
            }

            return matches.Count;
        }

        public bool IsMultitreaded
        {
            get { return true; }
        }
    }
}
