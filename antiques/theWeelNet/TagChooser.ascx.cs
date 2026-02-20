using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;
using System.Web.UI;
using System.Web.UI.WebControls;

namespace theWeelNet
{
    public partial class TagChooser : System.Web.UI.UserControl
    {
        protected void Page_Load(object sender, EventArgs e)
        {

        }

        /// <summary>
        /// sets the control to multiple-selection mode
        /// </summary>
        public bool MultipleSelection
        {
            get
            {
                return ListBox_AxisSelect.SelectionMode == ListSelectionMode.Multiple;
            }
            set
            {
                ListBox_AxisSelect.SelectionMode = value ? ListSelectionMode.Multiple : ListSelectionMode.Single;
            }
        }

        /// <summary>
        /// returns a single selected tag
        /// </summary>
        public string SelectedTag
        {
            get
            {
                return ListBox_AxisSelect.SelectedValue;
            }
        }

        /// <summary>
        /// returns all selected tags as a delimited string
        /// </summary>
        /// <param name="separator">separator to use between tags</param>
        /// <returns>the concatenated string</returns>
        public string GetSelectedTagString(string separator)
        {
            string strTags = "";
            int[] arrSelectedIndices = ListBox_AxisSelect.GetSelectedIndices();
            bool bFirst = true;
            for (int nAt = 0; nAt < arrSelectedIndices.Length; nAt++)
            {
                if (!bFirst)
                    strTags += separator;

                strTags += ListBox_AxisSelect.Items[arrSelectedIndices[nAt]].Text;
                bFirst = false;
            }

            return strTags;
        }

        /// <summary>
        /// 
        /// </summary>
        public event EventHandler SelectedTagChanged;

        protected void ListBox_AxisSelect_SelectedIndexChanged(object sender, EventArgs e)
        {
            SelectedTagChanged(sender, e);
        }
    }
}