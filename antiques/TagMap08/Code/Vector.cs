using System;
using System.Data;
using System.Configuration;
using System.Web;
using System.Web.Security;
using System.Web.UI;
using System.Web.UI.WebControls;
using System.Web.UI.WebControls.WebParts;
using System.Web.UI.HtmlControls;

namespace TagMap08.Code
{
    public class Vector
    {
        public float[] Elements;

        public int Dim
        {
            get
            {
                return Elements.Length;
            }
            set
            {
                Elements = new float[value];
            }
        }

        public float Length
        {
            get
            {
                float sumSq = 0.0f;
                foreach (float f in Elements)
                {
                    sumSq += f*f;
                }
                return System.Math.Sqrt(sumSq);
            }
        }

        public void Scale(float f)
        {
            for (int n = 0; n < Dim; n++)
                this.Elements[n] *= f;
        }

        public void Add(Vector v)
        {
            for (int n = 0; n < Dim; n++)
                this.Elements[n] += v.Elements[n];
        }
    }
}
