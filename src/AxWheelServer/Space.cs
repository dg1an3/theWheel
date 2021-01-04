using System;
using System.Web;
using System.Web.Services;
using System.Web.Services.Protocols;
using System.ComponentModel;
using System.Collections.Generic;

namespace AxWheelServer
{
    public class Space
    {
        /// <summary>
        /// RootNode is always node with Id 0
        /// </summary>
        public Node RootNode
        {
            get
            {
                return GetNodeById(0);
            }
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="newNode"></param>
        public void AddNode(Node newNode)
        {
            newNode.Id = myNextId;
            myNextId++;

            Node prevNode;
            if (nodeById.TryGetValue(newNode.Id, out prevNode))
            {
                myNodes.Remove(prevNode);
                nodeById.Remove(prevNode.Id);
            }

            myNodes.Add(newNode);
            nodeById.Add(newNode.Id, newNode);
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="Id"></param>
        /// <returns></returns>
        public Node GetNodeById(int Id)
        {
            return nodeById[Id];
        }

        /// <summary>
        /// 
        /// </summary>
        public Node[] Nodes
        {
            get
            {
                return myNodes.ToArray();
            }
            set
            {
                myNodes.Clear();
                myNodes.AddRange(value);

                nodeById.Clear();
                foreach (Node atNode in value)
                {
                    nodeById.Add(atNode.Id, atNode);
                }
            }
        }
        private List<Node> myNodes = new List<Node>();
        private Dictionary<int, Node> nodeById = new Dictionary<int, Node>();
        private int myNextId = 0;   // assigns RootNode to first node created
    }
}
