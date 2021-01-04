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

namespace TagMap08.Code
{
    /// <summary>
    /// 
    /// </summary>
    public class Tag
    {
        /// <summary>
        /// 
        /// </summary>
        /// <param name="initName"></param>
        public Tag(string initName)
        {
            System.Diagnostics.Trace.Assert(initName != "");
            Name = initName;
            All.Add(this);
        }


        public Vector Position;

        /// <summary>
        /// 
        /// </summary>
        /// <param name="Name"></param>
        /// <returns></returns>
        public static Tag Find(string Name)
        {
            if (Name == "")
                return null;

            foreach (Tag tag in All)
            {
                if (tag.Name == Name)
                    return tag;
            }
            return new Tag(Name);
        }

        /// <summary>
        /// 
        /// </summary>
        public string Name
        {
            get
            {
                return _Name;
            }
            set
            {
                _Name = value;
            }
        }
        string _Name;

        /// <summary>
        /// 
        /// </summary>
        public static List<Tag> GetAll() { return All; }
        private static List<Tag> All = new List<Tag>();

        public static int CompareTagsByPrevalance(Tag t1, Tag t2)
        {
            if (t1.Prevalance > t2.Prevalance)
                return -1;
            return 1;
        }

        public static List<Tag> GetAllByPrevalance()
        {
            if (AllByPrevalance == null)
            {
                AllByPrevalance = new List<Tag>();
                AllByPrevalance.InsertRange(0, GetAll());
                AllByPrevalance.Sort(CompareTagsByPrevalance);
            }
            return AllByPrevalance;
        }
        private static List<Tag> AllByPrevalance;

        /// <summary>
        /// 
        /// </summary>
        public float Prevalance
        {
            get
            {
                int nCount = 0;
                foreach (Bookmark bm in Bookmark.GetAll())
                {
                    if (bm.Tags.Contains(this))
                    {
                        nCount++;
                    }
                }
                return (float)nCount / (float)Bookmark.GetAll().Count;
            }
        }

        public float JointProb(Tag withTag)
        {
            int Count = 0;
            foreach (Bookmark bm1 in Bookmark.GetAll())
            {
                if (bm1.Tags.Contains(this) && bm1.Tags.Contains(withTag))
                    Count++;
            }

            return (float) Count / (float) Bookmark.GetAll().Count;
        }

        public float CondProb(Tag withTag)
        {
            int Count = 0;
            int CondCount = 0;
            foreach (Bookmark bm1 in Bookmark.GetAll())
            {
                if (bm1.Tags.Contains(withTag))
                {
                    CondCount++;
                    if (bm1.Tags.Contains(this))
                        Count++;
                }
            }

            return (float)Count / (float)CondCount;
        }

        public float AvgCondProb(Tag withTag)
        {
            float thisGivenThat = CondProb(withTag);
            float thatGivenThis = withTag.CondProb(this);
            float thisGivenThat_BF = thatGivenThis * this.Prevalance / withTag.Prevalance;
            if (!System.Double.IsNaN(thisGivenThat_BF))
                System.Diagnostics.Trace.Assert((thisGivenThat_BF - thisGivenThat) < 1e-6);
            return (thisGivenThat + thisGivenThat_BF) / 2.0f;
        }

        static Tag condProbTag;
        public float CondProbSingle
        {
            get
            {
                return AvgCondProb(condProbTag);
            }
        }

        public static List<Tag> GetAllCondProbString(string withTagName)
        {
            GetAll();
            condProbTag = Tag.Find(withTagName);
            All.Sort(CompareTagsByCondProb);
            return All;
        }

        public static int CompareTagsByCondProb(Tag t1, Tag t2)
        {
            if (t1.CondProbSingle > t2.CondProbSingle)
                return -1;
            return 1;
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="withTag"></param>
        /// <returns></returns>
        //public float Similarity(Tag withTag)
        //{
        //    float Prev1 = this.Prevalance;
        //    float Prev2 = withTag.Prevalance;

        //    float TotalSimilarity = 0.0f;
        //    float TotalRelevance = 0.0f;
        //    foreach (Bookmark bm1 in Bookmark.GetAll())
        //    {
        //        foreach (Bookmark bm2 in Bookmark.GetAll())
        //        {
        //            if (bm1 != bm2)
        //            {
        //                float Relevance = bm1.CommonTagFraction(bm2);
        //                if (bm1.Tags.Contains(this))
        //                {
        //                    if (bm2.Tags.Contains(withTag))
        //                        TotalSimilarity += Prev1 * Relevance * 1.0f;
        //                    TotalRelevance += Prev1 * Relevance;
        //                }
        //                if (bm1.Tags.Contains(withTag))
        //                {
        //                    if (bm2.Tags.Contains(this))
        //                        TotalSimilarity += Prev2 * Relevance * 1.0f;
        //                    TotalRelevance += Prev2 * Relevance;
        //                }
        //            }
        //        }
        //    }

        //    return TotalSimilarity / TotalRelevance;
        //}
    }
}
