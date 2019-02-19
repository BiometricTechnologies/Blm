using DPUruNet;
using IdentaZone.IMPlugin;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace IdentaZone.Plugins.DP
{
    public class DeviceConrolDP : IDeviceControl
    {
        public List<IFingerDevice> ActiveDevices { get; set; }

        public List<int> SupportedBSP { get { return _supportedBSP; } }

        public Boolean IsMultitreaded { get { return true; } }

        private readonly List<int> _supportedBSP = new List<int> { TemplateTypes.DPTemplate };


        public override string ToString()
        {
            return "Digital Persona";
        }

        public void Dispose()
        {
            foreach (var device in ActiveDevices)
            {
                device.Dispose();
            }
        }

        public FingerTemplate Deserialize(int type, byte[] data)
        {
            FingerTemplate result = null;
            if (type == TemplateTypes.DPTemplate)
            {
                result = new TemplateDP(TemplateDP.Deserialize(data));
            }
            return result;
        }

        /// <summary>
        /// Initializes this instance.
        /// </summary>
        public void Initialize()
        {
            try
            {
                ReaderCollection readers = ReaderCollection.GetReaders();
                ActiveDevices = new List<IFingerDevice>();
            }
            catch (Exception ex)
            {
                throw ex;
            }
        }

        /// <summary>
        /// Enumerates the devices.
        /// </summary>
        public void EnumerateDevices()
        {
            ReaderCollection readers = ReaderCollection.GetReaders();
            foreach (var reader in readers)
            {
                if (ActiveDevices.OfType<DeviceDP>().Any(dev =>
                    dev.Id == reader.Description.Name))
                {
                    continue;
                }
                DeviceDP device = new DeviceDP(reader.Description.Name, "U.Are.U Fingerprint Reader", this);
                ActiveDevices.Add(device);
            }
            List<DeviceDP> deleteList = new List<DeviceDP>();

            foreach (var device in ActiveDevices.OfType<DeviceDP>())
            {
                if (!readers.Any(reader =>
                    reader.Description.Name == device.Id))
                {
                    device.Dispose();
                    deleteList.Add(device);
                }
            }
            foreach (var device in deleteList)
            {
                ActiveDevices.Remove(device);
            }
        }

        private const int DP_THRESHOLD = 1000;

        public int Match(FingerTemplate template, IEnumerable<FingerTemplate> candidates, out List<FingerTemplate> matches)
        {
            // extract FMD from FID
            TemplateDP templateDP = template as TemplateDP;

            matches = new List<FingerTemplate>();

            foreach (var candidate in candidates.OfType<TemplateDP>())
            {
                var identifyResult = Comparison.Compare(templateDP.fmd, 0, candidate.fmd, 0);
                if (identifyResult.ResultCode == Constants.ResultCode.DP_SUCCESS)
                {
                    if (identifyResult.Score < DP_THRESHOLD)
                    {
                        matches.Add(candidate);
                    }
                }
            }

            return matches.Count;
        }

        public FingerTemplate Extract(FingerImage image)
        {
            if (image is FingerImageDP)
            {
                var fingerImage = image as FingerImageDP;
                DataResult<Fmd> resultConversion = FeatureExtraction.CreateFmdFromFid(fingerImage.fid, Constants.Formats.Fmd.ANSI);
                if (resultConversion.ResultCode == Constants.ResultCode.DP_SUCCESS)
                {
                    return new TemplateDP(resultConversion.Data);
                }
            }
            return null;
        }
    }
}
