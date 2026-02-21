<%@ Page Language="C#" AutoEventWireup="true" CodeBehind="Sort.aspx.cs" Inherits="theWeelNet.TagFilter" %>

<%@ Register src="TagChooser.ascx" tagname="TagChooser" tagprefix="uc1" %>

<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<head runat="server">
    <title>Untitled Page</title>
    <link rel="stylesheet" media="screen" href="./stylesheet.css" type="text/css" />
</head>
<body>
    <form id="form1" runat="server">
    <div class="sidebar">
        <asp:HyperLink ID="HyperLink_Browse" runat="server" NavigateUrl="~/Objects.aspx">Browse Objects</asp:HyperLink>
        <br />
        <asp:HyperLink ID="HyperLink_Sort" runat="server" NavigateUrl="~/Sort.aspx">Sort Objects</asp:HyperLink>
        <br />
        <uc1:TagChooser ID="TagChooser1" runat="server" OnSelectedTagChanged="TagChooser1_SelectedTagChanged" />
        <br />
    </div>
    <div class="main">
        <h3>
            <asp:Label ID="Label_PageTitle" runat="server"></asp:Label></h3>
        <table style="width: 100%; border-style: solid; border-width: thin; vertical-align: top">
            <tr>
                <td style="width: 50%">
                    <asp:FormView ID="FormViewLeft" runat="server" DataKeyNames="ObjectId" DataSourceID="ObjectDataSource_LeftObject"
                        AllowPaging="True" Width="100%" CellPadding="4" ForeColor="#333333">
                        <FooterStyle BackColor="#5D7B9D" Font-Bold="True" ForeColor="White" />
                        <RowStyle BackColor="#F7F6F3" ForeColor="#333333" />
                        <ItemTemplate>
                            <asp:LinkButton ID="LinkButton1" runat="server" Text='<%# Bind("ObjectName") %>'
                                OnClick="ObjectNameLabelLeft_Click"></asp:LinkButton>
                            <br />
                            <br />
                            <asp:TextBox ID="TextBox1" runat="server" Text='<%# Bind("Description") %>' ReadOnly="True"
                                TextMode="MultiLine" Width="90%"></asp:TextBox><br />
                            <br />
                            <span style="font-style: italic; text-align: right">
                                <asp:Label ID="Label1" runat="server" Text='<%# Bind("Tags") %>'></asp:Label></span>
                        </ItemTemplate>
                        <PagerStyle BackColor="#284775" ForeColor="White" HorizontalAlign="Center" />
                        <HeaderStyle BackColor="#5D7B9D" Font-Bold="True" ForeColor="White" />
                        <EditRowStyle BackColor="#999999" />
                        <EditItemTemplate>
                            ObjectName:
                            <asp:TextBox ID="ObjectNameTextBox" runat="server" Text='<%# Bind("ObjectName") %>'></asp:TextBox><br />
                            Description:
                            <asp:TextBox ID="DescriptionTextBox" runat="server" Text='<%# Bind("Description") %>'></asp:TextBox><br />
                            Tags:
                            <asp:TextBox ID="TagsTextBox" runat="server" Text='<%# Bind("Tags") %>'></asp:TextBox><br />
                            <asp:LinkButton ID="UpdateButton" runat="server" CausesValidation="True" CommandName="Update"
                                Text="Update"></asp:LinkButton>
                            <asp:LinkButton ID="UpdateCancelButton" runat="server" CausesValidation="False" CommandName="Cancel"
                                Text="Cancel"></asp:LinkButton>
                        </EditItemTemplate>
                        <InsertItemTemplate>
                            ObjectName:
                            <asp:TextBox ID="ObjectNameTextBox" runat="server" Text='<%# Bind("ObjectName") %>'></asp:TextBox><br />
                            Description:
                            <asp:TextBox ID="DescriptionTextBox" runat="server" Text='<%# Bind("Description") %>'></asp:TextBox><br />
                            Tags:
                            <asp:TextBox ID="TagsTextBox" runat="server" Text='<%# Bind("Tags") %>'></asp:TextBox><br />
                            When:
                            <asp:TextBox ID="WhenTextBox" runat="server" Text='<%# Bind("When") %>'></asp:TextBox><br />
                            ImageUrl:
                            <asp:TextBox ID="ImageUrlTextBox" runat="server" Text='<%# Bind("ImageUrl") %>'></asp:TextBox><br />
                            Url:
                            <asp:TextBox ID="UrlTextBox" runat="server" Text='<%# Bind("Url") %>'></asp:TextBox><br />
                            Modified:
                            <asp:TextBox ID="ModifiedTextBox" runat="server" Text='<%# Bind("Modified") %>'></asp:TextBox><br />
                            History:
                            <asp:TextBox ID="HistoryTextBox" runat="server" Text='<%# Bind("History") %>'></asp:TextBox><br />
                            <asp:LinkButton ID="InsertButton" runat="server" CausesValidation="True" CommandName="Insert"
                                Text="Insert"></asp:LinkButton>
                            <asp:LinkButton ID="InsertCancelButton" runat="server" CausesValidation="False" CommandName="Cancel"
                                Text="Cancel"></asp:LinkButton>
                        </InsertItemTemplate>
                    </asp:FormView>
                </td>
                <td style="width: 50%">
                    <asp:FormView ID="FormViewRight" runat="server" DataKeyNames="ObjectId" DataSourceID="ObjectDataSource_LeftObject"
                        AllowPaging="True" Width="100%" CellPadding="4" ForeColor="#333333">
                        <ItemTemplate>
                            <asp:LinkButton ID="ObjectNameLabel" runat="server" Text='<%# Bind("ObjectName") %>'
                                OnClick="ObjectNameLabelLeft_Click"></asp:LinkButton>
                            <br />
                            <br />
                            <asp:TextBox ID="DescriptionLabel" runat="server" Text='<%# Bind("Description") %>'
                                ReadOnly="True" TextMode="MultiLine" Width="90%"></asp:TextBox><br />
                            <br />
                            <span style="font-style: italic; text-align: right">
                                <asp:Label ID="TagsLabel" runat="server" Text='<%# Bind("Tags") %>'></asp:Label></span>
                        </ItemTemplate>
                        <FooterStyle BackColor="#5D7B9D" Font-Bold="True" ForeColor="White" />
                        <RowStyle BackColor="#F7F6F3" ForeColor="#333333" />
                        <PagerStyle BackColor="#284775" ForeColor="White" HorizontalAlign="Center" />
                        <HeaderStyle BackColor="#5D7B9D" Font-Bold="True" ForeColor="White" />
                        <EditRowStyle BackColor="#999999" />
                    </asp:FormView>
                </td>
            </tr>
        </table>
    </div>
    </form>
    <asp:ObjectDataSource ID="ObjectDataSource_LeftObject" runat="server" OldValuesParameterFormatString="original_{0}"
        SelectMethod="GetObjectsForTag" TypeName="theWeelNet.theWeelDBTableAdapters.ObjectTableAdapter"
        OnSelected="ObjectDataSource_LeftObject_Selected" DeleteMethod="Delete" InsertMethod="Insert"
        UpdateMethod="Update">
        <SelectParameters>
            <asp:ControlParameter ControlID="TagChooser1" Name="tag" PropertyName="SelectedTag"
                Type="String" DefaultValue="speedup" />
        </SelectParameters>
        <DeleteParameters>
            <asp:Parameter Name="Original_ObjectId" Type="Int32" />
            <asp:Parameter Name="Original_ObjectName" Type="String" />
            <asp:Parameter Name="Original_Tags" Type="String" />
            <asp:Parameter Name="Original_When" Type="DateTime" />
            <asp:Parameter Name="Original_ImageUrl" Type="String" />
            <asp:Parameter Name="Original_Url" Type="String" />
            <asp:Parameter Name="Original_Modified" Type="DateTime" />
        </DeleteParameters>
        <UpdateParameters>
            <asp:Parameter Name="ObjectName" Type="String" />
            <asp:Parameter Name="Description" Type="String" />
            <asp:Parameter Name="Tags" Type="String" />
            <asp:Parameter Name="When" Type="DateTime" />
            <asp:Parameter Name="ImageUrl" Type="String" />
            <asp:Parameter Name="Url" Type="String" />
            <asp:Parameter Name="Modified" Type="DateTime" />
            <asp:Parameter Name="History" Type="Object" />
            <asp:Parameter Name="Original_ObjectId" Type="Int32" />
            <asp:Parameter Name="Original_ObjectName" Type="String" />
            <asp:Parameter Name="Original_Tags" Type="String" />
            <asp:Parameter Name="Original_When" Type="DateTime" />
            <asp:Parameter Name="Original_ImageUrl" Type="String" />
            <asp:Parameter Name="Original_Url" Type="String" />
            <asp:Parameter Name="Original_Modified" Type="DateTime" />
        </UpdateParameters>
        <InsertParameters>
            <asp:Parameter Name="ObjectName" Type="String" />
            <asp:Parameter Name="Description" Type="String" />
            <asp:Parameter Name="Tags" Type="String" />
            <asp:Parameter Name="When" Type="DateTime" />
            <asp:Parameter Name="ImageUrl" Type="String" />
            <asp:Parameter Name="Url" Type="String" />
            <asp:Parameter Name="Modified" Type="DateTime" />
            <asp:Parameter Name="History" Type="Object" />
        </InsertParameters>
    </asp:ObjectDataSource>
    <asp:ObjectDataSource ID="ObjectDataSource_NewSortEvent" runat="server" TypeName="theWeelNet.theWeelDBTableAdapters.SortEventTableAdapter"
        InsertMethod="Insert">
        <InsertParameters>
            <asp:Parameter Name="ResponseAt" Type="DateTime" />
            <asp:Parameter Name="AxisName" Type="String" />
            <asp:Parameter Name="MatchObjectId" Type="Int32" />
            <asp:Parameter Name="RejectObjectId" Type="Int32" />
        </InsertParameters>
    </asp:ObjectDataSource>
</body>
</html>
