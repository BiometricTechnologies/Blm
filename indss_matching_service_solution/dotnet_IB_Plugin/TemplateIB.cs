using IdentaZone.IMPlugin;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace IB
{
    public class TemplateIB : FingerTemplate
    {
        override public int BSPCode
        {
            get { return TemplateTypes.IBTemplate; }
        }
        public byte[] enrollment { get; set; }
        public TemplateIB(byte[] enrollment)
        {
            this.enrollment = enrollment;
        }
        override public byte[] Serialize()
        {
            return enrollment;
        }
    }
}
