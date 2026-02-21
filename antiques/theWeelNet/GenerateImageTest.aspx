<%@ Page Language="C#" AutoEventWireup="true" Codebehind="GenerateImageTest.aspx.cs" Inherits="TestImageUpdateWeb._Default" %>

<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<head runat="server">
    <title>Untitled Page</title>
</head>
<body>
    <form id="form1" runat="server">
        <div>
            <img id="genImage" src="GenerateImage.ashx?size=25" alt="ellipse" onmousedown="dragStart(event)" />
        </div>
    </form>

    <script type="text/javascript">

    // global object to hold drag information.
    var dragObj = new Object();

    /////////////////////////////////////////////////////
    function dragStart(event) {

        // get target element
        if (browser.isIE) dragObj.elementNode = window.event.srcElement;
        if (browser.isNS) dragObj.elementNode = event.target;

        // get mouse position with respect to the page.
        if (browser.isIE) {
            dragObj.startX = window.event.clientX + document.documentElement.scrollLeft
                + document.body.scrollLeft;
            dragObj.startY = window.event.clientY + document.documentElement.scrollTop
                + document.body.scrollTop;
        }
        else if (browser.isNS) {
            dragObj.startX = event.clientX + window.scrollX;
            dragObj.startY = event.clientY + window.scrollY;
        }

        // capture mousemove and mouseup events on the page.
        if (browser.isIE) {
            document.attachEvent("onmousemove", dragGo);
            document.attachEvent("onmouseup",   dragStop);
            window.event.cancelBubble = true;
            window.event.returnValue = false;
        }
        else if (browser.isNS) {
            document.addEventListener("mousemove", dragGo,   true);
            document.addEventListener("mouseup",   dragStop, true);
            event.preventDefault();
        }
    }

    /////////////////////////////////////////////////////
    function dragGo(event) {
    
        if (browser.isIE) {
            dragObj.offsetX = (window.event.clientX + document.documentElement.scrollLeft
              + document.body.scrollLeft);
            dragObj.offsetY = window.event.clientY + document.documentElement.scrollTop
              + document.body.scrollTop;
        }
        else if (browser.isNS) {
            dragObj.offsetX = event.clientX + window.scrollX;
            dragObj.offsetY = event.clientY + window.scrollY;
        }
        dragObj.offsetX -= dragObj.startX;
        dragObj.offsetY -= dragObj.startY;

        // determine new parameter based on drag
        dragObj.elementNode.src = "GenerateImage.ashx?size=" + dragObj.offsetY;

        if (browser.isIE) {
            window.event.cancelBubble = true;
            window.event.returnValue = false;
        }
        else if (browser.isNS) {
            event.preventDefault();
        }
    }        

    /////////////////////////////////////////////////////
    function dragStop(event) {

        // stop capturing mousemove and mouseup events.
        if (browser.isIE) {
            document.detachEvent("onmousemove", dragGo);
            document.detachEvent("onmouseup",   dragStop);
        }          
        else if (browser.isNS) {
            document.removeEventListener("mousemove", dragGo,   true);
            document.removeEventListener("mouseup",   dragStop, true);
        }
    }      
    
    //////////////////////////////////////////////////////////////
    function Browser() 
    {
      var ua, s, i;

      this.isIE    = false;
      this.isNS    = false;
      this.version = null;

      ua = navigator.userAgent;

      s = "MSIE";
      if ((i = ua.indexOf(s)) >= 0) {
        this.isIE = true;
        this.version = parseFloat(ua.substr(i + s.length));
        return;
      }

      s = "Netscape6/";
      if ((i = ua.indexOf(s)) >= 0) {
        this.isNS = true;
        this.version = parseFloat(ua.substr(i + s.length));
        return;
      }

      // Treat any other "Gecko" browser as NS 6.1.
      s = "Gecko";
      if ((i = ua.indexOf(s)) >= 0) {
        this.isNS = true;
        this.version = 6.1;
        return;
      }      
      return;
    }
    var browser = new Browser();
        
    </script>

</body>
</html>
