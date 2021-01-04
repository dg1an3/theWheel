<%@ Page Language="C#" AutoEventWireup="true" CodeBehind="Default.aspx.cs" Inherits="TagMap08._Default" %>

<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">

<html xmlns="http://www.w3.org/1999/xhtml" >
<head runat="server">
    <title>Untitled Page</title>
</head>
<body>
    <form id="form1" runat="server">
    <div>
        <asp:DropDownList ID="DropDownList2" runat="server" AutoPostBack="True" DataSourceID="ObjectDataSource1"
            DataTextField="Name" DataValueField="Name">
        </asp:DropDownList>
        <br />
        <asp:ObjectDataSource ID="ObjectDataSource1" runat="server" SelectMethod="GetAllByPrevalance"
            TypeName="TagMap08.Code.Tag"></asp:ObjectDataSource>
        <br />
        <asp:GridView ID="GridView1" runat="server" AutoGenerateColumns="False" DataSourceID="ObjectDataSource2">
            <Columns>
                <asp:BoundField DataField="Name" HeaderText="Name" SortExpression="Name" />
                <asp:BoundField DataField="Prevalance" HeaderText="Prevalance" ReadOnly="True" SortExpression="Prevalance" DataFormatString="{0:f2}" />
                <asp:BoundField DataField="CondProbSingle" HeaderText="CondProbSingle" ReadOnly="True"
                    SortExpression="CondProbSingle" DataFormatString="{0:f2}" />
            </Columns>
        </asp:GridView>
        <asp:ObjectDataSource ID="ObjectDataSource2" runat="server" SelectMethod="GetAllCondProbString"
            TypeName="TagMap08.Code.Tag">
            <SelectParameters>
                <asp:ControlParameter ControlID="DropDownList2" Name="withTagName" PropertyName="SelectedValue"
                    Type="String" />
            </SelectParameters>
        </asp:ObjectDataSource>
    
    </div>
    </form>
</body>
</html>
