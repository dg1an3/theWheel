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
    public partial class TagFilter : System.Web.UI.Page
    {
        protected void Page_Load(object sender, EventArgs e)
        {
            // TagChooser1.SelectedTagChanged += new EventHandler(TagChooser1_SelectedTagChanged);
        }

        protected void ObjectNameLabelLeft_Click(object sender, EventArgs e)
        {
            ObjectDataSource_NewSortEvent.InsertParameters["ResponseAt"].DefaultValue = System.DateTime.Now.ToString();
            ObjectDataSource_NewSortEvent.InsertParameters["AxisName"].DefaultValue = TagChooser1.SelectedTag;
            ObjectDataSource_NewSortEvent.InsertParameters["MatchObjectId"].DefaultValue = FormViewLeft.DataKey.Value.ToString();
            ObjectDataSource_NewSortEvent.InsertParameters["RejectObjectId"].DefaultValue = FormViewRight.DataKey.Value.ToString();
            ObjectDataSource_NewSortEvent.Insert();

            ObjectDataSource_LeftObject.Select();
        }

        protected void ObjectNameLabelRight_Click(object sender, EventArgs e)
        {
            ObjectDataSource_NewSortEvent.InsertParameters["ResponseAt"].DefaultValue = System.DateTime.Now.ToString();
            ObjectDataSource_NewSortEvent.InsertParameters["AxisName"].DefaultValue = TagChooser1.SelectedTag;
            ObjectDataSource_NewSortEvent.InsertParameters["MatchObjectId"].DefaultValue = FormViewRight.DataKey.Value.ToString();
            ObjectDataSource_NewSortEvent.InsertParameters["RejectObjectId"].DefaultValue = FormViewLeft.DataKey.Value.ToString(); 
            ObjectDataSource_NewSortEvent.Insert();

            ObjectDataSource_LeftObject.Select();
        }

        protected void ObjectDataSource_LeftObject_Selected(object sender, ObjectDataSourceStatusEventArgs e)
        {
            if (FormViewLeft.PageCount < 2)
                return;

            System.Random random = new System.Random();
            FormViewLeft.PageIndex = random.Next(FormViewLeft.PageCount);
            FormViewRight.PageIndex = random.Next(FormViewRight.PageCount);
            if (FormViewLeft.PageIndex == FormViewRight.PageIndex)
            {
                FormViewRight.PageIndex = (FormViewLeft.PageIndex + 1) % FormViewLeft.PageCount;
            }
        }

        protected void TagChooser1_SelectedTagChanged(object sender, EventArgs e)
        {
            Label_PageTitle.Text = TagChooser1.SelectedTag;
        }
    }
}
