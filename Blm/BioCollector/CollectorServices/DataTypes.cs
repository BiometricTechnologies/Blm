using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.Serialization;
using System.Text;
using System.Threading.Tasks;

namespace IdentaZone.IdentaMasterServices
{


    public enum CollectorState
    {
        UNITIALIZED,
        OK,
        NO_DATA
    }
   

    public enum LogOperation
    {
        ENCRYPT,
        DECRYPT
    }

  
    [DataContract]
    public struct LogRecord
    {
        [DataMember]
        public LogOperation operation;
        [DataMember]
        public String provider;
        [DataMember]
        public String filename;
        [DataMember]
        public String username;
    }

}
