using System;
using System.Data;
using System.Configuration;
using System.Web;
using System.Web.Security;
using System.Web.UI;
using System.Web.UI.WebControls;
using System.Web.UI.WebControls.WebParts;
using System.Web.UI.HtmlControls;
using System.Collections.Generic;
using System.Xml.Serialization;

namespace AxWheelServer
{
    public class Node
    {
        public const int INVALID_ID = -1;

        /// <summary>
        /// 
        /// </summary>
        public Node()
        {
            Id = INVALID_ID;
            myImageBuffer = new System.Byte[] { 0, 1, 2, 3, 4 };
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="Name"></param>
        public Node(string initName)
        {
            Id = INVALID_ID;
            Name = initName;
            myImageBuffer = new System.Byte[] { 0, 1, 2, 3, 4 };
        }

        /// <summary>
        /// 
        /// </summary>
        public int Id;

        /// <summary>
        /// 
        /// </summary>
        public string Name;

        /// <summary>
        /// 
        /// </summary>
        public string Body;

        /// <summary>
        /// 
        /// </summary>
        [SoapIgnore]
        public System.Byte[] ImageBuffer
        {
            get
            {
                return myImageBuffer;
            }
        }
        private System.Byte[] myImageBuffer;

        /// <summary>
        /// 
        /// </summary>
        public Link[] Links;
    }
}
