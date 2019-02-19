using BioNetACSLib;
using IdentaZone.IMPlugin;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
namespace IB
{
    public class DeviceControlIB : IDeviceControl
    {

        public List<IFingerDevice> ActiveDevices { get; set; }

        public override string ToString()
        {
            return "Integrated Biometrics";
        }

        public void Initialize()
        {
            ActiveDevices = new List<IFingerDevice>();
            BioNetACSDLL._AlgoInit();
        }

        public void Dispose()
        {
            foreach (var reader in ActiveDevices)
            {
                reader.Dispose();
            }
        }


        public FingerTemplate Deserialize(String type, String data)
        {
            FingerTemplate result = null;
            if (type == TemplateTypes.IBTemplate.ToString())
            {
                result = new TemplateIB(Convert.FromBase64String(data));
            }
            return result;
        }

        public void EnumerateDevices()
        {
            String deviceList = BioNetACSDLL._GetUSNList();

            foreach (var deviceName in deviceList.Split(','))
            {
                if (ActiveDevices.OfType<DeviceIB>().Any(dev => dev.name == deviceName)) continue;
                var error = BioNetACSDLL._OpenNetAccessDeviceByUSN(deviceName);
                if (error != 1)
                {
                    break;
                }
                else
                {
                    BioNetACSDLL._CloseNetAccessDevice();
                }

                DeviceIB device = new DeviceIB(deviceName);
                ActiveDevices.Add(device);
            }

            List<DeviceIB> toDeleteList = new List<DeviceIB>();
            foreach (var device in ActiveDevices.OfType<DeviceIB>())
            {
                bool toDelete = true;
                foreach (var deviceName in deviceList.Split(','))
                {
                    if (deviceName == device.name)
                    {
                        toDelete = false;
                    }
                }
                if (toDelete)
                {
                    toDeleteList.Add(device);
                    device.Dispose();
                }
            }
            foreach (var device in toDeleteList)
            {
                ActiveDevices.Remove(device);
            }
        }

        public bool IsMultitreaded
        {
            get { return true; }
        }

        private readonly List<int> _supportedBSP = new List<int> { TemplateTypes.IBTemplate};
        public List<int> SupportedBSP
        {
            get { return _supportedBSP; }
        }

        public FingerTemplate Deserialize(int type, byte[] data)
        {
            FingerTemplate result = null;
            if (type == TemplateTypes.IBTemplate)
            {
                result = new TemplateIB(data);
            }
            return result;
        }

        public int Match(FingerTemplate template, IEnumerable<FingerTemplate> candidates, out List<FingerTemplate> matches)
        {
            byte[] pFp = new byte[BioNetACSDLL._GetFeatSize()];

            var resultList = new List<FingerTemplate>();
            TemplateIB templateIB = template as TemplateIB;

            matches = new List<FingerTemplate>();

            foreach (var candidate in candidates.OfType<TemplateIB>())
            {
                int compareResult = BioNetACSDLL._CompareFt9052vs9052(candidate.enrollment, templateIB.enrollment);
                if (compareResult > 0)
                {
                    matches.Add(candidate);
                }
            }

            return matches.Count;
        }
    }
}
