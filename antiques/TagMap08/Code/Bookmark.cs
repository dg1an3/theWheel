using System;
using System.Data;
using System.Configuration;
using System.Net;
using System.Web;
using System.Web.Security;
using System.Web.UI;
using System.Web.UI.WebControls;
using System.Web.UI.WebControls.WebParts;
using System.Web.UI.HtmlControls;
using System.Collections.Generic;
using System.Xml;
using System.Text.RegularExpressions;

namespace TagMap08.Code
{
    /// <summary>
    /// 
    /// </summary>
    public class Bookmark
    {
        /// <summary>
        /// 
        /// </summary>
        /// <param name="Name"></param>
        /// <param name="Url"></param>
        public Bookmark(string Name, string Url)
        {
            Tags = new List<Tag>();
            All.Add(this);
        }

        /// <summary>
        /// 
        /// </summary>
        public List<Tag> Tags;

        /// <summary>
        /// 
        /// </summary>
        /// <param name="withbm"></param>
        /// <returns></returns>
        public float CommonTagFraction(Bookmark withbm)
        {
            int CommonCount = 0;

            foreach (Tag tag in Tags)
            {
                if (withbm.Tags.Contains(tag))
                {
                    CommonCount++;
                }
            }

            return (float)(2*CommonCount) / (float)(Tags.Count + withbm.Tags.Count);
        }

        /// <summary>
        /// 
        /// </summary>
        /// <returns></returns>
        public static List<Bookmark> GetAll()
        {
            if (All == null)
                LoadAll("");
            return All;
        }
        private static List<Bookmark> All;

        /// <summary>
        /// 
        /// </summary>
        /// <param name="url"></param>
        public static void LoadAll(string url)
        {
            All = new List<Bookmark>();

            //NetworkCredential myCred = new NetworkCredential(
            //    SecurelyStoredUserName,SecurelyStoredPassword,SecurelyStoredDomain);
 
            //CredentialCache myCache = new CredentialCache();
            

            //HttpWebRequest req = (HttpWebRequest)WebRequest.Create(url);
            //// req.Credentials = 
            //WebResponse resp = req.GetResponse();
            //XmlTextReader reader = new XmlTextReader(resp.GetResponseStream());                      
            XmlReader reader = XmlReader.Create("file://C:\\Projects\\TagMap08\\all-posts.xml");
            while (reader.Read())
            {
                if (reader.Name == "post")
                {
                    Bookmark bm = new Bookmark(reader.GetAttribute("name"), reader.GetAttribute("url"));

                    string strTags = reader.GetAttribute("tag");

                    // parse tags
                    Regex regex = new Regex("(?<tag>[A-Za-z0-9_]+)\\s*");
                    MatchCollection matches = regex.Matches(strTags);
                    foreach (Match match in matches)
                    {
                        Group tagGroup = match.Groups["tag"];
                        Tag tag = TagMap08.Code.Tag.Find(tagGroup.Value);
                        bm.Tags.Add(tag);
                    }
                }
            }
        }
    }
}
