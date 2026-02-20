using System;
using System.Data;
using System.Web;
using System.Xml;
using System.Collections;
using System.Configuration;
using System.Web.Services;
using System.Web.Services.Protocols;
using System.ComponentModel;

namespace theWeelNet
{
    /// <summary>
    /// Summary description for ObjectAccess
    /// </summary>
    [WebService(Namespace = "http://tempuri.org/")]
    [WebServiceBinding(ConformsTo = WsiProfiles.BasicProfile1_1)]
    [ToolboxItem(false)]
    public class ObjectAccess : System.Web.Services.WebService
    {
        private theWeelNet.theWeelDBTableAdapters.ObjectTableAdapter objectTableAdapter1;
        private theWeelNet.theWeelDBTableAdapters.AxisWeightTableAdapter axisWeightTableAdapter1;

        public ObjectAccess()
        {
            // is InitializeComponent() called yet?
            string strConnection = ConfigurationManager.ConnectionStrings["theWeelConnectionString"].ConnectionString;
            this.objectTableAdapter1.Connection = new System.Data.SqlClient.SqlConnection(strConnection);
        }

        [WebMethod]
        public theWeelDB GetObjectById(int nObjectId)
        {
            theWeelDB db = new theWeelDB();
            db.Tables.Add(objectTableAdapter1.GetObjectById(nObjectId));
            return db;
        }

        [WebMethod]
        public theWeelDB GetLinksById(int nObjectId)
        {
            theWeelDB.AxisWeightDataTable result = axisWeightTableAdapter1.GetLinksByObjectId(nObjectId);
            return result.DataSet as theWeelDB;
        }

        private void InitializeComponent()
        {
            this.objectTableAdapter1 = new theWeelNet.theWeelDBTableAdapters.ObjectTableAdapter();
            this.axisWeightTableAdapter1 = new theWeelNet.theWeelDBTableAdapters.AxisWeightTableAdapter();
            // 
            // objectTableAdapter1
            // 
            this.objectTableAdapter1.ClearBeforeFill = true;
            // 
            // axisWeightTableAdapter1
            // 
            this.axisWeightTableAdapter1.ClearBeforeFill = true;

        }
    }
}
