using IdentaZone.IMPlugin;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace IdentaZone.Plugins.FAKE
{
    public class DeviceControlFake : IDeviceControl
    {
        /// <summary>
        /// Can this device perform multithreaded matching or
        /// Device API is limited to singlethreded matching
        /// </summary>
        public bool IsMultitreaded
        {
            get { return true; }
        }

        /// <summary>
        /// Provider Name, usually simular to DeviceName
        /// </summary>
        /// <returns></returns>
        public override string ToString()
        {
            return "Fake Device";
        }

        /// <summary>
        /// List of currently active devices
        /// </summary>
        public List<IFingerDevice> ActiveDevices { get; set; }

        /// <summary>
        /// Supported BSP providers list
        /// </summary>
        private readonly List<int> _supportedBSP = new List<int> { TemplateTypes.FakeTemplate};
        public List<int> SupportedBSP
        {
            get { return _supportedBSP; }
        }

        public int BSPCode
        {
            get { return 0; }
        }

        /// <summary>
        /// Convert byte array into fingertemplate, if possible
        /// </summary>
        /// <param name="type"></param>
        /// <param name="data"></param>
        /// <returns></returns>
        public FingerTemplate Deserialize(int bspcode, byte[] data)
        {
            FingerTemplate result = null;
            if (bspcode == BSPCode)
            {
                //TODO: decode byte[] date into FingerTemplate object
                result = new TemplateFake(data);
            }
            return result;
        }



        /// <summary>
        /// Match templates
        /// </summary>
        /// <param name="template"></param>
        /// <param name="candidates"></param>
        /// <param name="matches"></param>
        /// <returns></returns>
        public int Match(FingerTemplate template, IEnumerable<FingerTemplate> candidates, out List<FingerTemplate> matches)
        {
            matches = new List<FingerTemplate>();
            foreach (var candidate in candidates)
            {
                matches.Add(candidate);
            }
            return matches.Count();
        }

        public void Initialize()
        {
            ActiveDevices = new List<IFingerDevice>();
        }

        private DeviceFake _device = new DeviceFake();
        public void EnumerateDevices()
        {
            ActiveDevices.Clear();
            ActiveDevices.Add(_device);
        }

        public void Dispose()
        {
        }
    }
}
