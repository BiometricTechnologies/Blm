using IdentaZone.IMPlugin;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace IdentaZone.Plugins.FAKE
{
    public class TemplateFake : FingerTemplate
    {
        public override int BSPCode
        {
            get { return TemplateTypes.FakeTemplate; }
        }

        public override byte[] Serialize()
        {
            return new byte[0];
        }

        private byte[] data;

        public TemplateFake(byte[] data)
        {
            // TODO: Complete member initialization
            this.data = data;
        }
    }
}
