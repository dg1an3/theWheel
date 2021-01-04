<%@ Page Language="C#" AutoEventWireup="true" CodeBehind="Objects.aspx.cs" Inherits="theWeelNet._Objects" %>

<%@ Register src="TagChooser.ascx" tagname="TagChooser" tagprefix="uc1" %>

<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<head runat="server">
    <title>tW Objects</title>
    <link rel="Stylesheet" href="./stylesheet.css" type="text/css" />
</head>
<body>
    <form id="form1" runat="server">    
    <div class="sidebar">
        <asp:HyperLink ID="HyperLink_Browse" runat="server" NavigateUrl="~/Objects.aspx">Browse Objects</asp:HyperLink>
        <br />
        <asp:HyperLink ID="HyperLink_Sort" runat="server" NavigateUrl="~/Sort.aspx">Sort Objects</asp:HyperLink>
        <br />
        <uc1:TagChooser ID="TagChooser1" runat="server" MultipleSelection="true" OnSelectedTagChanged="TagChooser1_SelectedTagChanged" />
        <br />
    </div>
    <div class="main">
        <table width="90%">
            <tr> 
                <td width="50%"><h3><asp:Label ID="Label_PageTitle" runat="server"></asp:Label></h3> </td>
                <td><h3>Search: <asp:TextBox Width="60%" ID="TextBox_Search" runat="server" OnTextChanged="TextBox_Search_TextChanged"
                            AutoCompleteType="Search" AutoPostBack="True"></asp:TextBox></h3></td>
            </tr>
        </table>
        <asp:GridView ID="GridView_Objects" runat="server" DataKeyNames="ObjectId" DataSourceID="ObjectDataSource_GridView_Objects"
            EmptyDataText="There are no data records to display." AllowPaging="True" AutoGenerateColumns="False"
            AllowSorting="True" Width="90%" CellPadding="4" ForeColor="#333333" GridLines="Horizontal">
            <Columns>
                <asp:CommandField ShowSelectButton="True" />
                <asp:BoundField DataField="ObjectId" HeaderText="ObjectId" InsertVisible="False"
                    ReadOnly="True" SortExpression="ObjectId" />
                <asp:BoundField DataField="ObjectName" HeaderText="ObjectName" SortExpression="ObjectName" />
                <asp:BoundField DataField="Description" HeaderText="Description" SortExpression="Description" />
                <asp:BoundField DataField="Tags" HeaderText="Tags" SortExpression="Tags" />
                <asp:BoundField DataField="When" HeaderText="When" SortExpression="When" DataFormatString="{0:d}" />
            </Columns>
            <SelectedRowStyle BackColor="#E2DED6" Font-Bold="True" ForeColor="#333333" />
            <PagerSettings Position="Top" />
            <RowStyle BackColor="#F7F6F3" ForeColor="#333333" />
            <FooterStyle BackColor="#5D7B9D" Font-Bold="True" ForeColor="White" />
            <PagerStyle BackColor="#284775" ForeColor="White" HorizontalAlign="Center" />
            <HeaderStyle BackColor="#5D7B9D" Font-Bold="True" ForeColor="White" />
            <EditRowStyle BackColor="#999999" />
            <AlternatingRowStyle BackColor="White" ForeColor="#284775" />
        </asp:GridView>
    </div>
    <br />
    <div class="main">
        <table width="95%">
            <tr>
                <td width="50%">
                    <asp:FormView ID="FormView_Object" runat="server" DataSourceID="ObjectDataSource_FormView_Object"
                        CellPadding="4" DataKeyNames="ObjectId" ForeColor="#333333" Width="75%">
                        <FooterStyle BackColor="#5D7B9D" Font-Bold="True" ForeColor="White" />
                        <RowStyle BackColor="#F7F6F3" ForeColor="#333333" />
                        <EditItemTemplate>
                            ObjectName:
                            <asp:TextBox ID="ObjectNameTextBox" runat="server" Text='<%# Bind("ObjectName") %>'>
                            </asp:TextBox><br />
                            Description:
                            <asp:TextBox ID="DescriptionTextBox" runat="server" Text='<%# Bind("Description") %>'>
                            </asp:TextBox><br />
                            Tags:
                            <asp:TextBox ID="TagsTextBox" runat="server" Text='<%# Bind("Tags") %>'>
                            </asp:TextBox><br />
                            When:
                            <asp:TextBox ID="WhenTextBox" runat="server" Text='<%# Bind("When") %>'>
                            </asp:TextBox><br />
                            Modified:
                            <asp:TextBox ID="ModifiedTextBox" runat="server" Text='<%# Bind("Modified") %>'>
                            </asp:TextBox><br />
                            <asp:LinkButton ID="UpdateButton" runat="server" CausesValidation="True" CommandName="Update"
                                Text="Update">
                            </asp:LinkButton>
                            <asp:LinkButton ID="UpdateCancelButton" runat="server" CausesValidation="False" CommandName="Cancel"
                                Text="Cancel">
                            </asp:LinkButton>
                        </EditItemTemplate>
                        <InsertItemTemplate>
                            ObjectName:
                            <asp:TextBox ID="ObjectNameTextBox" runat="server" Text='<%# Bind("ObjectName") %>'>
                            </asp:TextBox><br />
                            Description:
                            <asp:TextBox ID="DescriptionTextBox" runat="server" Text='<%# Bind("Description") %>'>
                            </asp:TextBox><br />
                            Tags:
                            <asp:TextBox ID="TagsTextBox" runat="server" Text='<%# Bind("Tags") %>'>
                            </asp:TextBox><br />
                            When:
                            <asp:TextBox ID="WhenTextBox" runat="server" Text='<%# Bind("When") %>'>
                            </asp:TextBox><br />
                            Modified:
                            <asp:TextBox ID="ModifiedTextBox" runat="server" Text='<%# Bind("Modified") %>'>
                            </asp:TextBox><br />
                            <asp:LinkButton ID="InsertButton" runat="server" CausesValidation="True" CommandName="Insert"
                                Text="Insert">
                            </asp:LinkButton>
                            <asp:LinkButton ID="InsertCancelButton" runat="server" CausesValidation="False" CommandName="Cancel"
                                Text="Cancel">
                            </asp:LinkButton>
                        </InsertItemTemplate>
                        <ItemTemplate>
                            ObjectName:
                            <asp:Label ID="ObjectNameLabel" runat="server" Text='<%# Bind("ObjectName") %>'>
                            </asp:Label><br />
                            Description:
                            <asp:Label ID="DescriptionLabel" runat="server" Text='<%# Bind("Description") %>'>
                            </asp:Label><br />
                            Tags:
                            <asp:Label ID="TagsLabel" runat="server" Text='<%# Bind("Tags") %>'></asp:Label><br />
                            When:
                            <asp:Label ID="WhenLabel" runat="server" Text='<%# Bind("When") %>'></asp:Label><br />
                            Modified:
                            <asp:Label ID="ModifiedLabel" runat="server" Text='<%# Bind("Modified") %>'></asp:Label><br />
                            <asp:LinkButton ID="EditButton" runat="server" CausesValidation="False" CommandName="Edit"
                                Text="Edit">
                            </asp:LinkButton>
                            <asp:LinkButton ID="DeleteButton" runat="server" CausesValidation="False" CommandName="Delete"
                                Text="Delete">
                            </asp:LinkButton>
                            <asp:LinkButton ID="NewButton" runat="server" CausesValidation="False" CommandName="New"
                                Text="New">
                            </asp:LinkButton>
                        </ItemTemplate>
                        <PagerStyle BackColor="#284775" ForeColor="White" HorizontalAlign="Center" />
                        <HeaderStyle BackColor="#5D7B9D" Font-Bold="True" ForeColor="White" />
                        <EditRowStyle BackColor="#999999" />
                    </asp:FormView>
                </td>
                <td width="50%" style="vertical-align: text-top">
                    <asp:GridView ID="GridView_Links" runat="server" DataSourceID="ObjectDataSource_GridView_Links"
                        EmptyDataText="There are no data records to display." AllowPaging="True" AutoGenerateColumns="False"
                        AllowSorting="True" Width="90%" CellPadding="4" ForeColor="#333333" GridLines="Horizontal">
                        <Columns>
                            <asp:BoundField DataField="AxisName" HeaderText="AxisName" SortExpression="AxisName" />
                            <asp:BoundField DataField="Weight" HeaderText="Weight" ReadOnly="True" SortExpression="Weight" />
                            <asp:BoundField DataField="TotalSortEvents" HeaderText="TotalSortEvents" ReadOnly="True"
                                SortExpression="TotalSortEvents" />
                            <asp:BoundField DataField="SortCoverage" HeaderText="SortCoverage" ReadOnly="True"
                                SortExpression="SortCoverage" />
                        </Columns>
                        <SelectedRowStyle BackColor="#E2DED6" Font-Bold="True" ForeColor="#333333" />
                        <PagerSettings Position="Top" />
                        <RowStyle BackColor="#F7F6F3" ForeColor="#333333" />
                        <FooterStyle BackColor="#5D7B9D" Font-Bold="True" ForeColor="White" />
                        <PagerStyle BackColor="#284775" ForeColor="White" HorizontalAlign="Center" />
                        <HeaderStyle BackColor="#5D7B9D" Font-Bold="True" ForeColor="White" />
                        <EditRowStyle BackColor="#999999" />
                        <AlternatingRowStyle BackColor="White" ForeColor="#284775" />
                    </asp:GridView>
                </td>
            </tr>
        </table>
    </div>
    <asp:ObjectDataSource ID="ObjectDataSource_GridView_Objects" runat="server" SelectMethod="SearchObjects"
        TypeName="theWeelNet.theWeelDBTableAdapters.ObjectTableAdapter" OldValuesParameterFormatString="original_{0}"
        OnSelecting="ObjectDataSource_GridView_Objects_Selecting">
        <SelectParameters>
            <asp:Parameter Name="tags" Type="String" />
            <asp:Parameter Name="searchText" Type="String" />
        </SelectParameters>
    </asp:ObjectDataSource>
    <asp:ObjectDataSource ID="ObjectDataSource_FormView_Object" runat="server" InsertMethod="Insert"
        OldValuesParameterFormatString="original_{0}" SelectMethod="GetObjectById" TypeName="theWeelNet.theWeelDBTableAdapters.ObjectTableAdapter"
        UpdateMethod="Update" DeleteMethod="Delete">
        <UpdateParameters>
            <asp:Parameter Name="ObjectName" Type="String" />
            <asp:Parameter Name="Description" Type="String" />
            <asp:Parameter Name="Tags" Type="String" />
            <asp:Parameter Name="When" Type="DateTime" />
            <asp:Parameter Name="Modified" Type="DateTime" />
            <asp:Parameter Name="Original_ObjectId" Type="Int32" />
        </UpdateParameters>
        <SelectParameters>
            <asp:ControlParameter ControlID="GridView_Objects" Name="objectId" PropertyName="SelectedValue"
                Type="Int32" />
        </SelectParameters>
        <InsertParameters>
            <asp:Parameter Name="ObjectName" Type="String" />
            <asp:Parameter Name="Description" Type="String" />
            <asp:Parameter Name="Tags" Type="String" />
            <asp:Parameter Name="When" Type="DateTime" />
            <asp:Parameter Name="Modified" Type="DateTime" />
        </InsertParameters>
        <DeleteParameters>
            <asp:Parameter Name="Original_ObjectId" Type="Int32" />
        </DeleteParameters>
    </asp:ObjectDataSource>
    <asp:ObjectDataSource ID="ObjectDataSource_GridView_Links" runat="server" OldValuesParameterFormatString="original_{0}"
        SelectMethod="GetLinksByObjectId" TypeName="theWeelNet.theWeelDBTableAdapters.AxisWeightTableAdapter">
        <SelectParameters>
            <asp:ControlParameter ControlID="GridView_Objects" Name="objectId" PropertyName="SelectedValue"
                Type="Int32" />
        </SelectParameters>
    </asp:ObjectDataSource>
    </form>
</body>
</html>
