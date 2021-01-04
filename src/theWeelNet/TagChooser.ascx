<%@ Control Language="C#" AutoEventWireup="true" CodeBehind="TagChooser.ascx.cs" Inherits="theWeelNet.TagChooser" %>

<asp:ListBox ID="ListBox_AxisSelect" runat="server" DataSourceID="ObjectDataSource_AxisNames"
    DataTextField="AxisName" DataValueField="AxisName" AutoPostBack="True" Rows="25"
    OnSelectedIndexChanged="ListBox_AxisSelect_SelectedIndexChanged" Width="80%"></asp:ListBox>
<br />
<asp:RadioButtonList ID="RadioButtonList_MinTagObjects" runat="server" AutoPostBack="True"
    RepeatDirection="Horizontal">
    <asp:ListItem Selected="True">4</asp:ListItem>
    <asp:ListItem>3</asp:ListItem>
    <asp:ListItem>2</asp:ListItem>
</asp:RadioButtonList>
<asp:ObjectDataSource ID="ObjectDataSource_AxisNames" runat="server" SelectMethod="GetImportantAxes"
    TypeName="theWeelNet.theWeelDBTableAdapters.AxisTableAdapter" >
    <SelectParameters>
        <asp:ControlParameter ControlID="RadioButtonList_MinTagObjects" Name="min" DefaultValue="4" PropertyName="SelectedValue" Type="Int32" />
    </SelectParameters>
</asp:ObjectDataSource>