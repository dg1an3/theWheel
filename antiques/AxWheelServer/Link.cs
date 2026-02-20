using System;
using System.Web;
using System.Web.Services;
using System.Web.Services.Protocols;
using System.ComponentModel;
using System.Xml.Serialization;

namespace AxWheelServer
{
    public class Link
    {
        /// <summary>
        /// 
        /// </summary>
        public Link()
        {
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="initToNode"></param>
        /// <param name="initWeight"></param>
        public Link(Node initToNode, float initWeight)
        {
            myToNode = initToNode;
            /// ToNodeId = initToNode.Id;
            Weight = initWeight;
        }

        /// <summary>
        /// 
        /// </summary>
        public int ToNodeId
        {
            get
            {
                if (myToNode != null)
                    return myToNode.Id;
                return myToNodeId;
            }
            set
            {
                myToNodeId = value;
            }
        }
        private int myToNodeId;

        /// <summary>
        /// 
        /// </summary>
        //[XmlIgnore]
        //public Node ToNode
        //{
        //    get
        //    {
        //        return Node.GetNodeById(ToNodeId);
        //    }
        //    set
        //    {
        //        ToNodeId = value.Id;
        //    }
        //}
        private Node myToNode;

        /// <summary>
        /// 
        /// </summary>
        public float Weight
        {
            get
            {
                return myWeight;
            }
            set
            {
                myWeight = value;
            }
        }
        private float myWeight;
    }
}
