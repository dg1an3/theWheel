using System;
using System.Data;
using System.Configuration;
using System.Collections.Generic;
using System.Web;
using System.Web.Security;
using System.Web.UI;
using System.Web.UI.WebControls;
using System.Web.UI.WebControls.WebParts;
using System.Web.UI.HtmlControls;

namespace theWeelNet
{
    public class Cluster : IFeatureVector
    {
        /// <summary>
        /// 
        /// </summary>
        public IFeatureVector LeftChild
        {
            get
            {
                return _leftChild;
            }
            set
            {
                _featureVector = null;
                _leftChild = value;
            }
        }

        //
        IFeatureVector _leftChild;

        /// <summary>
        /// 
        /// </summary>
        public IFeatureVector RightChild
        {
            get
            {
                return _rightChild;
            }
            set
            {
                _featureVector = null;
                _rightChild = value;
            }
        }

        //
        IFeatureVector _rightChild;

        /// <summary>
        /// 
        /// </summary>
        public Vector FeatureVector
        {
            get
            {
                if (_featureVector == null)
                {
                    _featureVector = LeftChild.FeatureVector + RightChild.FeatureVector;
                }
                return _featureVector;
            }
        }

        //
        Vector _featureVector;

        /// <summary>
        /// 
        /// </summary>
        public float Distance = System.Single.MaxValue;

        /// <summary>
        /// 
        /// </summary>
        /// <param name="forList"></param>
        /// <returns></returns>
        public static Cluster Analyze(IList<IFeatureVector> forList)
        {
            while (forList.Count > 1)
            {
                Cluster currentCluster = new Cluster();
                foreach (IFeatureVector leftItem in forList)
                {
                    foreach (IFeatureVector rightItem in forList)
                    {
                        if (rightItem == leftItem)
                            continue;

                        float currentDistance = (leftItem.FeatureVector - rightItem.FeatureVector).Length;
                        if (currentDistance < currentCluster.Distance)
                        {
                            currentCluster.LeftChild = leftItem;
                            currentCluster.RightChild = rightItem;
                            currentCluster.Distance = currentDistance;
                        }
                    }
                }

                forList.Remove(currentCluster.LeftChild);
                forList.Remove(currentCluster.RightChild);
                forList.Add(currentCluster);
            }

            return forList[0] as Cluster;
        }
    }
}
