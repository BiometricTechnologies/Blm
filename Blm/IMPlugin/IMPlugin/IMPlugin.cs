using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace IdentaZone.IMPlugin
{
    public enum ControlTypeEnum
    {
        FINGERPRINT_CONTROL_TYPE = 1,
        PALM_CONTROL_TYPE = 2,
        IRIS_CONTROL_TYPE = 3,
        FACE_CONTROL_TYPE = 4,
        VOICE_CONTROL_TYPE = 5,
        SIGNATURE_CONTROL_TYPE = 6,
        KESTROKE_DYNAMICS_CONTROL_TYPE = 7,
        BRAINWAVE_CONTROL_TYPE = 8
    }
    public abstract class Biometrics
    {
    }

    public abstract class FingerTemplate : Biometrics
    {
        abstract public int BSPCode { get; }
        abstract public byte[] Serialize();
    }

    public abstract class FingerImage : Biometrics
    {
        public abstract FingerPicture MakePicture();
        public abstract void Serialize(out String type, out byte[] data);
    }

    public sealed class FingerPicture
    {
        public byte[] Image { get { return _image; } }
        public int Width { get { return _width; } }
        public int Height { get { return _height; } }

        private byte[] _image;
        private int _width;
        private int _height;

        public FingerPicture(byte[] image, int width, int height)
        {
            _image = image;
            _width = width;
            _height = height;
        }
    }

    public interface IDevicePlugin
    {
        String Name { get; }
        Version Version { get; }
        String Description { get; }
        String DeploymentList { get; }

        void Initialize(object MainContainer);
        void Dispose();
    }

    public interface IDeviceControl
    {
        Boolean IsMultitreaded { get; }

        List<IFingerDevice> ActiveDevices { get; set; }
        List<int> SupportedBSP { get; }
        /// <summary>
        /// If template "bspcode" is known by plugin, function should
        /// Deserialize it and return Template. Otherwise null should be returned.
        /// </summary>
        /// <param name="bspcode">Template BSPCode</param>
        /// <param name="data">Data</param>
        /// <returns></returns>
        FingerTemplate Deserialize(int bspcode, byte[] data);
        int Match(FingerTemplate template, IEnumerable<FingerTemplate> candidates, out List<FingerTemplate> matches);

        void Initialize();
        void EnumerateDevices();
        void Dispose();
    }

    public interface IFingerDevice
    {
        int BSPCode { get; }
        ControlTypeEnum ControlType { get; }
        void Dispatch(COMMAND com);
        FingerTemplate Extract(FingerImage image);
        void Dispose();
        string Description { get; }
    }

    public enum COMMAND
    {
        NONE,
        LIVECAPTURE_START,
        LIVECAPTURE_STOP,
        ENROLLMENT_START,
        ENROLLMENT_STOP,
        SINGLECAPTURE_START,
        SINGLECAPTURE_STOP
    }

}
