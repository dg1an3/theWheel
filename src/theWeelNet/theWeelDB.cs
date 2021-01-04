namespace theWeelNet {

    /// <summary>
    /// 
    /// </summary>
    public partial class theWeelDB 
    {

        /// <summary>
        /// 
        /// </summary>
        public partial class ObjectRow : IFeatureVector
        {
            /// <summary>
            /// 
            /// </summary>
            public Vector FeatureVector
            {
                get
                {
                    if (_featureVector == null)
                    {
                        _featureVector = new Vector();

                        theWeelDB db = this.Table.DataSet as theWeelDB;
                        theWeelDBTableAdapters.AxisWeightTableAdapter awTableAdapter =
                            new theWeelDBTableAdapters.AxisWeightTableAdapter();
                        awTableAdapter.Fill(db.AxisWeight);
                        AxisWeightDataTable awTable = awTableAdapter.GetLinksByObjectId(this.ObjectId);
                        foreach (System.Data.DataRow row in awTable.Rows)
                        {
                            AxisWeightRow axisWeightRow = row as AxisWeightRow;
                            _featureVector[axisWeightRow.AxisName] = axisWeightRow.Weight;
                        }
                    }
                    return _featureVector;
                }
            }

            Vector _featureVector;
        }
    }


}
