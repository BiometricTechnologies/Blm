using Hitachi.Wrapper;
using IdentaZone.IMPlugin;
using log4net;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace Hitachi
{
    public class DeviceControlHi : IDeviceControl
    {
        private readonly ILog _log = log4net.LogManager.GetLogger(typeof(DeviceControlHi));
        public List<IFingerDevice> ActiveDevices { get; set; }
        private DateTime LastTime = DateTime.Now - TimeSpan.FromSeconds(2.0);
        public override String ToString()
        {
            return "Hitachi";
        }

        public void Initialize()
        {

            ActiveDevices = new List<IFingerDevice>();
            HitachiBio.Initialize();
            EnumerateDevices();
        }

        public void EnumerateDevices()
        {
//            var newDeviceList = HitachiBio.EnumerateDevices();
            if (DateTime.Now - LastTime > TimeSpan.FromSeconds(1.0))
            {
                LastTime = DateTime.Now;
            }
            else
            {
                return;
            }
            var removeList = ActiveDevices.OfType<DeviceHi>().Where(dev => !dev.IsOnline).ToList();
            foreach (var device in removeList)
            {
                device.Dispose();
                ActiveDevices.Remove(device);
            }
            foreach (var device in ActiveDevices)
            {
                device.Dispatch(COMMAND.SINGLECAPTURE_STOP);
            }

            var newDevicesInfo = HitachiBio.EnumerateDevices();
            foreach (var deviceInfo in newDevicesInfo)
            {
                if (!ActiveDevices.OfType<DeviceHi>().Any(dev => dev.Uid == deviceInfo.UnitId.ToString()))
                {
                    ActiveDevices.Add(new DeviceHi(deviceInfo));
                }
            }
        }

        public void Dispose()
        {
            foreach (var device in ActiveDevices)
            {
                device.Dispose();
            }

            HitachiBio.Dispose();
        }

        public bool IsMultitreaded
        {
            get { return true; }
        }

        private readonly List<int> _supportedBSP = new List<int> { TemplateTypes.HiTemplate };

        public List<int> SupportedBSP
        {
            get { return _supportedBSP; }
        }


        public FingerTemplate Deserialize(int type, byte[] data)
        {
            TemplateHi template = null;
            if (type == TemplateTypes.HiTemplate)
            {
                template = new TemplateHi(data);
            }
            return template;
        }

        /// <summary>
        /// Matches the specified template.
        /// </summary>
        /// <param name="template">The template.</param>
        /// <param name="candidates">The candidates.</param>
        /// <param name="matches">The matches.</param>
        /// <returns></returns>
        /// <exception cref="System.NotImplementedException"></exception>
        public int Match(FingerTemplate template, IEnumerable<FingerTemplate> candidates, out List<FingerTemplate> matches)
        {
            var devices = ActiveDevices.OfType<DeviceHi>().Where(dev => dev.IsOnline).ToList();
            if (devices != null)
            {
                if (devices.Count > 0)
                {
                    return devices[0].Match(template, candidates.ToList(), out matches);
                }
            }
            matches = null;
            return 0;
        }
    }
}
