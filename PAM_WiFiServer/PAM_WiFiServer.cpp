//////////////////////////////////////////////////////////////////////
//
//                    PI and more
//              PAM_WiFiServer library
//
// https://piandmore.wordpress.com/tag/pam_wifiserver
//
//////////////////////////////////////////////////////////////////////
//
// Include the PAM_WiFiServer header
//
#include <PAM_WiFiServer.h>
//
// Include the PAM_Tools library
//
#include <PAM_Tools.h>
//
// Include the FileSytem library
//
#include <FS.h>

//
// Initialize the webserver
//
WiFiServer server(80);
//
// The client request is in this variable. Since the ESP cannot handle multiple
// requests at the same time, it is safe to use juse one variable.
//
WiFiClient client;
//
// The String keepRequestString keeps the last request by a client for later
// parsing for page name and variables
//
String keepRequestString = "";

bool deviceNeedsRestart = false;

//
// Start the webserver. This needs to be called once, for example in setup()
//
void wifiServerStart() {
  server.begin();
}

//
// Send a page back to the client. The page is retrieved from the file system and
// parsed line by line where parameters are replaced by the corresponding values.
//
// The default page name, if none is given, is index.htm
//
// The page with page name pageName is retrieved from the file system. The function
// expects the two arrays, paramName and paramValue to be of the same length, paramCount.
//
// The parameter replacement is done by searching each line for the string %paramName%
// and replace it with the corresponding %paramValue%. An example is included with
// this library to show how it is done.
// Pages are stored in the file system in directory '/page'.
//
void serverSendPage (String paramName[], String paramValue[], int paramCount, String pageName) {
  if (client) {
    toolsSetup();
    //
    // If the pagename is empty, assume they want index.htm
    //
    String sendThis = pageName;
    if (sendThis.length()==0 || sendThis=="/") {
      sendThis = "/index.htm";
    }
    //
    // If the pagename does not start with a leading slash, add it
    //
    if (sendThis.charAt(0)!='/') {
      sendThis = "/"+sendThis;
    }
    //
    // Check if the requested page exists.
    // If it does, send it and cycle through the parameter and replace
    // the parameter with the value.
    //
    if (SPIFFS.exists("/page"+sendThis)) {
      client.println("HTTP/1.1 200 OK");
      client.println("Content-Type: text/html");
      client.println("Connection: close");
      client.println();
      client.println("<!DOCTYPE HTML>");
      File f = SPIFFS.open("/page"+sendThis,"r");
      while(f.available()) {
        String line = f.readStringUntil('\n');
        if (line.indexOf('%')!=-1) {
          for (byte i=0;i<paramCount;i++) {
            line.replace("%"+paramName[i]+"%",paramValue[i]);
          }
        }
        client.println(line);
      }
    } else {
      //
      // The requested page does not exist, so send a 404
      //
      client.println("HTTP/1.0 404 Not Found");
      client.println("Content-Type: text/html");
      client.println("Connection: close");
      client.println();
      client.println("<!DOCTYPE HTML>");
      //
      // If there is a 404.htm page than send that.
      // If not, send a brief explanation.
      //
      if (SPIFFS.exists("/page/404.htm")) {
        File f = SPIFFS.open("/page/404.htm","r");
        while(f.available()) {
          String line = f.readStringUntil('\n');
          if (line.indexOf('%')!=-1) {
            line.replace("%page%",sendThis);
          }
          client.println(line);
        }
      } else {
        client.println("<html><head><title>Not Found</title></head><body>");
        client.println("<h2>Sorry, the page you requested, <font color=red>");
        client.println(sendThis);
        client.println("</font>, was not found.</h2></body><html>");
      }
    }
    delay(1);
    client.stop();
  }
}

//
// Redirect the browser to another location. This can be convenient for example
// if you receive requests through a HTTP GET and you don't want the user to
// send the same info again. In that case, process the GET request and redirect
// to another page (or the same page without GET parameters)
//
void serverSendRedirect (String newLocation) {
  if (client) {
    toolsSetup();
    client.println("HTTP/1.1 307 Temporary Redirect");
    client.print("Location: ");
    client.println(newLocation);
    client.println("Connection: close");
    client.println();
    delay(1);
    client.stop();
  }
}


//
// If no pagename is given, it is assumed that the page requested
// has to be send.
//
void serverSendPage (String paramName[], String paramValue[], int paramCount) {
  serverSendPage(paramName,paramValue,paramCount,pageRequested());
}

//
// If no parameters are given, it is assumed that no parameter
// replacement is needed and that the requested page needs to be send.
//
void serverSendPage (String pageName) {
  String tmp[1] = { "" };
  serverSendPage(tmp,tmp,0,pageName);
}

//
// if no pagename and no parameters are given, it is assumed that the page
// requested has to be send where no parameter replacement is needed.
//
void serverSendPage () {
  String tmp[1] = { "" };
  serverSendPage(tmp,tmp,0,pageRequested());
}

//
// Check whether there is a page requested from a client
// True is returned if a page is requested after which the main
// program should call a version of serverSendPage()
//
bool serverRequestReceived() {
  client = server.available();
  if (client) {
    //
    // A web client is asking for a page
    //
    boolean lastLineBlank = true;
    keepRequestString = "";
    while (client.connected()) {
      //
      // We need to read the request of the client
      //
      if (client.available()) {
        char c = client.read();
        keepRequestString = keepRequestString+c;
        if (c=='\n' && lastLineBlank) {
          return true;
        }
        if (c=='\n') {
          lastLineBlank = true;
        } else if (c!='\r') {
          lastLineBlank = false;
        }
      }
    }
    return false;
  }
  return false;
}

//
// Return just the URI
//
String uriRequested () {
  byte p = keepRequestString.indexOf("T /")+2;
  return keepRequestString.substring(p,keepRequestString.indexOf(" ",p));
}

//
// Return the requested page
//
String pageRequested () {
  String t = uriRequested();
  if (t.indexOf("?")>=0) {
    t = t.substring(0,t.indexOf("?"));
  }
  if (t.length()==0) {
    t = "index.html";
  }
  if (t.charAt(0)=='/') {
    t = t.substring(1);
  }
  return t;
}

//
// Return the value of the parameter requested or PARAMNOTFOUND
//
String giveURLParam (String search) {
  String t = uriRequested();
  if (t.indexOf("?")>=0) {
    t = "&"+t.substring(t.indexOf("?")+1);
    int p = t.indexOf("&"+search+"=");
    if (p==-1) {
      return PARAMNOTFOUND;
    } else {
      p = t.indexOf("=",p)+1;
      t = t.substring(p,t.indexOf("&",p));
      return t;
    }
  } else {
    return PARAMNOTFOUND;
  }
}

//
// This function will check whether the page settings.htm was requested
// and if so, generate a settings page based on the vars in the file system.
// It will also handle modifying the settings use the PAM_Tools library.
// If you want to include some description, you can put that section
// on the filesystem as /page/settings.htm and it will include that
// directly after the header.
// If the page requested is not processed, it will return a false
// so you know to handle the page further on in the code.
// The idea behind this function is that it is always called if you have
// a page request and only handle the page if it returns false.
//
bool pageSettings (String modulename) {
  if (client) {
    if (pageRequested()=="settings.htm") {
      toolsSetup();
      //
      // This section processes all updates to the parameters
      // If there are any changes done, send a redirect
      // to settings.htm to prevent reloading of the
      // same update.
      if (giveURLParam("change")!=PARAMNOTFOUND) {
        String p = uriRequested();
        if (p.indexOf("?")>=0) {
          //
          // Cycle through every parameter and see if it starts
          // with a String, int, float
          // and if so, process the change.
          //
          p = p.substring(p.indexOf("?")+1)+"&";
          while (p.length()>0) {
            String ap = p.substring(0,p.indexOf("&"));
            String val = "";
            if (p.indexOf("=")!=-1) {
              val = ap.substring(p.indexOf("=")+1);
              ap = ap.substring(0,p.indexOf("="));
            }
            if (ap.substring(0,6)=="String") {
              putStringKey("var",ap.substring(6),val);
              deviceNeedsRestart = true;
            } else if (ap.substring(0,3)=="int") {
              putIntKey("var",ap.substring(3),val.toInt());
              deviceNeedsRestart = true;
            } else if (ap.substring(0,5)=="float") {
              putFloatKey("var",ap.substring(5),val.toFloat());
              deviceNeedsRestart = true;
            }
            if (p.indexOf("&")==-1) {
              p = "";
            } else {
              p = p.substring(p.indexOf("&")+1);
            }
          }
        }
        serverSendRedirect("settings.htm");
        return true;
      } else if (giveURLParam("del")!=PARAMNOTFOUND) {
        //
        // A request to delete a parameter was received.
        // Delete it and go back the the settings.htm page.
        //
        String ap = giveURLParam("del");
        if (ap.substring(0,6)=="String") {
          SPIFFS.remove("/var/"+ap.substring(6)+".txt");
          deviceNeedsRestart = true;
        } else if (ap.substring(0,3)=="int") {
          SPIFFS.remove("/var/int/"+ap.substring(3)+".txt");
          deviceNeedsRestart = true;
        } else if (ap.substring(0,5)=="float") {
          SPIFFS.remove("/var/float/"+ap.substring(5)+".txt");
          deviceNeedsRestart = true;
        } else if (ap.substring(0,4)=="bool") {
          SPIFFS.remove("/var/bool/"+ap.substring(4)+".txt");
          deviceNeedsRestart = true;
        }
        serverSendRedirect("settings.htm");
        return true;
      }
      //
      // When we arrive here, we know that there was no request to delete
      // and nor were there changes to process.
      // Now we send the standard settings.htm which is generated based on all
      // variabbles in '/var/'
      //
      //
      // Start with a basic header, which includes using the module name
      // supplied to the function
      //
      client.println("HTTP/1.1 200 OK");
      client.println("Content-Type: text/html");
      client.println("Connection: close");
      client.println();
      client.println("<!DOCTYPE HTML>");
      client.println("<html><head><title>Settings for "+modulename+"</title></head>");
      client.println("<body><h1>Settings for <a href='index.htm'>"+modulename+"</a></h1><br>");
      //
      // If there is a page called settings.htm, this will contain text
      // to explain what each variable does, so include it.
      //
      if (SPIFFS.exists("/page/settings.htm")) {
        File f = SPIFFS.open("/page/settings.htm","r");
        while(f.available()) {
          String line = f.readStringUntil('\n');
          line.replace("%name%",modulename);
          client.println(line);
        }
      }
      //
      // Next we create the form with the variables
      //
      client.println("<form action=settings.htm method=get><table>");
      Dir dir = SPIFFS.openDir("/var");
      while (dir.next()) {
        String fn = dir.fileName();
        File f = SPIFFS.open(fn,"r");
        //
        // Read the first line of the file which is than returned
        // to get the current value
        //
        String line = f.readStringUntil('\n');
        if (line.length()>0 && line.charAt(line.length()-1)=='\r') {
          line = line.substring(0,line.length()-1);
        }
        fn = fn.substring(5,fn.length()-4);
        String fnadd = "String";
        if (fn.substring(0,4)=="int/") {
          fnadd = "int";
          fn = fn.substring(4);
        }
        String fnt = fn;
        if (fn.indexOf("_")!=-1) {
          fnt = fn.substring(0,fn.indexOf("_"));
        }
        fnt.replace("-","&nbsp;");
        //
        // Create the table row with a Del(ete) reference
        // and a text field
        //
        client.print("<tr><td><a href='?del="+fnadd+fn+"'>Del</a></td><td>");
        client.print(fnt+": </td><td><input type=text name='"+fnadd);
        client.print(fn+"' value='"+line+"' size=");
        if (fnadd=="String") {
          client.print("2");
        } else if (fnadd=="float") {
          client.print("1");
        }
        client.print("5>");
        if (fn.indexOf("_")!=-1) {
          String fnt = fn.substring(fn.indexOf("_")+1);
          fnt.replace("-","&nbsp;");
          client.print(" "+fnt);
        }
        client.println("</td></tr>");
      }
      //
      // Close the form and the table
      //
      client.println("</table><br><input type=submit name=change value='Change'>&nbsp; &nbsp;");
      client.println("<input type=reset name=reset value='Reset changes'></body></html>");
      delay(1);
      client.stop();
      return true;
    }
  }
  return false;
}
