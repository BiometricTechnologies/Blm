using DPUruNet;
using IdentaZone.IMPlugin;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Xml;

namespace IdentaZone.Plugins.DP
{
    public class TemplateDP : FingerTemplate
    {
        override public int BSPCode
        {
            get { return TemplateTypes.DPTemplate; }
        }

        public Fmd fmd { get; set; }

        public TemplateDP(Fmd fmd)
        {
            this.fmd = fmd;
        }


        /// <summary>
        /// Serializes this instance.
        /// </summary>
        /// <returns></returns>
        override public byte[] Serialize()
        {
            if (fmd.Bytes != null)
            {
                return fmd.Bytes;
            }
            else
            {
                return new byte[0];
            }
        }

        static public Fmd Deserialize(byte[] template)
        {
            String xml = "<?xml version=\"1.0\" encoding=\"UTF-8\"?><Fid><Bytes>" + Convert.ToBase64String(template) + "</Bytes><Format>1769473</Format><Version>1.0.0</Version></Fid>";
            return Fmd.DeserializeXml(xml);
        }

    }
}
