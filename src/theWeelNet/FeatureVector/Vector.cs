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
    /// <summary>
    /// 
    /// </summary>
    public class Vector
    {
        /// <summary>
        /// 
        /// </summary>
        public Vector() { }

        /// <summary>
        /// 
        /// </summary>
        /// <returns></returns>
        public Vector Clone()
        {
            Vector vClone = new Vector();
            foreach (string axisName in _weights.Keys)
            {
                vClone._weights.Add(axisName, this[axisName]);
            }
            return vClone;
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="axisName"></param>
        /// <returns></returns>
        public float this[string axisName]
        {
            get
            {
                if (_weights.ContainsKey(axisName))
                    return _weights[axisName];
                return 0.0F;
            }
            set
            {
                if (_weights.ContainsKey(axisName))
                    _weights[axisName] = value;
                else
                    _weights.Add(axisName, value);
            }
        }

        /// <summary>
        /// 
        /// </summary>
        public float Length
        {
            get
            {
                float lengthSquared = 0.0F;
                foreach (float axisWeight in _weights.Values)
                {
                    lengthSquared += axisWeight * axisWeight;
                }
                return (float) Math.Sqrt(lengthSquared);
            }
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="vRight"></param>
        /// <param name="vLeft"></param>
        /// <returns></returns>
        public static Vector operator +(Vector vRight, Vector vLeft)
        {
            Vector sum = vRight.Clone();
            foreach (string axisKey in vLeft._weights.Keys)
            {
                sum[axisKey] += vLeft[axisKey];
            }
            return sum;
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="vRight"></param>
        /// <param name="vLeft"></param>
        /// <returns></returns>
        public static Vector operator -(Vector vRight, Vector vLeft)
        {
            Vector diff = vRight.Clone();
            foreach (string axisKey in vLeft._weights.Keys)
            {
                diff[axisKey] -= vLeft[axisKey];
            }
            return diff;
        }

        //
        Dictionary<string, float> _weights = new Dictionary<string, float>();
    }
}
