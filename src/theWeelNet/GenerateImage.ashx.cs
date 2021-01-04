using System;
using System.Data;
using System.Drawing;
using System.Drawing.Imaging;
using System.Web;
using System.Collections;
using System.Web.Services;
using System.Web.Services.Protocols;

namespace TestImageUpdateWeb
{
    /// <summary>
    /// Summary description for $codebehindclassname$
    /// </summary>
    [WebService(Namespace = "http://tempuri.org/")]
    [WebServiceBinding(ConformsTo = WsiProfiles.BasicProfile1_1)]
    public class GenerateImage : IHttpHandler
    {
        Bitmap bitmap = new Bitmap(200, 200);
        Graphics graphics;

        public void ProcessRequest(HttpContext context)
        {
            int size = System.Convert.ToInt32(context.Request.QueryString["size"]);            

            if (graphics == null)
                graphics = Graphics.FromImage(bitmap);

            graphics.DrawEllipse(new Pen(Color.Azure), 
                new Rectangle(new Point(10, 10), new Size(size, size)));

            context.Response.ContentType = "image/jpeg";
            bitmap.Save(context.Response.OutputStream, ImageFormat.Jpeg);
        }

        public bool IsReusable
        {
            get
            {
                return true;
            }
        }
    }
}