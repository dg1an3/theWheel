using System;
using System.Data;
using System.Web;
using System.Collections;
using System.Web.Services;
using System.Web.Services.Protocols;
using System.ComponentModel;
using System.Xml.Serialization;
using System.IO;
using System.Collections.Generic;
using System.Diagnostics;

namespace AxWheelServer
{
    /// <summary>
    /// Summary description for Service1
    /// </summary>
    [WebService(Namespace = "http://tempuri.org/")]
    [WebServiceBinding(ConformsTo = WsiProfiles.BasicProfile1_1)]
    [ToolboxItem(false)]
    public class NodeService : System.Web.Services.WebService
    {
        /// <summary>
        /// 
        /// </summary>
        public NodeService()
        {
            Trace.WriteLine("NodeService.NodeService");
            LoadSpace("space.xml");
        }

        /// <summary>
        /// 
        /// </summary>
        private Space mySpace;
        private static Dictionary<string, Space> mySpaceMap
            = new Dictionary<string, Space>();

        /// <summary>
        /// 
        /// </summary>
        /// <param name="filename"></param>
        public void LoadSpace(string filename)
        {
            if (mySpaceMap.TryGetValue(filename, out mySpace))
            {
                Trace.WriteLine("NodeService.LoadSpace: Located cached space for file " + filename);
                return;
            }

            XmlSerializer xmlser =
                new XmlSerializer(typeof(Space));

            try
            {
                Trace.WriteLine("NodeService.LoadSpace: Loading space from file " + filename);

                TextReader reader = new StreamReader(filename);
                mySpace = (Space) xmlser.Deserialize(reader);
                reader.Close();
            }
            catch (FileNotFoundException)
            {
                GenSpace();

                // Writing the document requires a TextWriter.
                TextWriter writer = new StreamWriter(filename);
                xmlser.Serialize(writer, mySpace);
                writer.Close();
            }

            mySpaceMap.Add(filename, mySpace);
        }

        /// <summary>
        /// 
        /// </summary>
        public void GenSpace()
        {
            Trace.WriteLine("NodeService.GenSpace");

            Node rootNode = new Node("ROOT");
            Node nodeMoe = new Node("Moe");
            Node nodeCurly = new Node("Curly");
            Node nodeJoe = new Node("Joe");

            rootNode.Links = new Link[] { new Link(nodeMoe, 0.5f), new Link(nodeCurly, 0.5f), new Link(nodeJoe, 0.5f) };
            nodeMoe.Links = new Link[] { new Link(rootNode, 0.5f), new Link(nodeCurly, 0.25f), new Link(nodeJoe, 0.15f) };

            mySpace = new Space();
            mySpace.AddNode(rootNode);
            mySpace.AddNode(nodeMoe);
            mySpace.AddNode(nodeCurly);
            mySpace.AddNode(nodeJoe);
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="Id"></param>
        /// <returns></returns>
        [WebMethod]
        public Node GetNodeById(int Id)
        {
            return mySpace.GetNodeById(Id);
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="Id"></param>
        /// <returns></returns>
        [WebMethod]
        public System.Byte[] GetImageBufferForNodeId(int Id)
        {
            Node node = GetNodeById(Id);
            return node.ImageBuffer;
        }
    }
}
