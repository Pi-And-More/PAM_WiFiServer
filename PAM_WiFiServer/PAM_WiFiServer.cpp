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
    String sendThis = pageName;
    if (pageName.length()==0 || pageName=="/") {
      sendThis = "/index.htm";
    }
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
      client.println("HTTP/1.0 404 Not Found");
      client.println("Content-Type: text/html");
      client.println("Connection: close");
      client.println();
      client.println("<!DOCTYPE HTML>");
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
        if (keepRequestString.length()<100) {
          keepRequestString = keepRequestString+c;
        }
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
