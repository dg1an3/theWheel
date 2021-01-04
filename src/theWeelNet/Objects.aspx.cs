using System;
using System.Data;
using System.Configuration;
using System.Collections;
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
    public partial class _Objects : System.Web.UI.Page
    {
        /// <summary>
        /// 
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        protected void Page_Load(object sender, EventArgs e)
        {
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        protected void TagChooser1_SelectedTagChanged(object sender, EventArgs e)
        {
            Label_PageTitle.Text = TagChooser1.GetSelectedTagString(", ");
            ObjectDataSource_GridView_Objects.Select();
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        protected void TextBox_Search_TextChanged(object sender, EventArgs e)
        {
            ObjectDataSource_GridView_Objects.Select();
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        protected void ObjectDataSource_GridView_Objects_Selecting(object sender, ObjectDataSourceMethodEventArgs e)
        {
            ObjectDataSource_GridView_Objects.SelectParameters["tags"].DefaultValue = TagChooser1.GetSelectedTagString(" OR ");
            ObjectDataSource_GridView_Objects.SelectParameters["searchText"].DefaultValue = TextBox_Search.Text;
        }
    }
}
