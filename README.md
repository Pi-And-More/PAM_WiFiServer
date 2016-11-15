# PAM_WiFiServer

An extension for the ESP8266WiFi library which allows for quickly setting up a webserver.

The functions contained in it are:

//
// Start the webserver. This needs to be called once, for example in setup()
//
void wifiServerStart()

//
// Send a page back to the client. The page is retrieved from the file system and
// parsed line by line where parameters are replaced by the corresponding values.
//
// The page with page name pageName is retrieved from the file system. The function
// expects the two arrays, paramName and paramValue to be of the same length, paramCount.
//
// The parameter replacement is done by searching each line for the string %paramName%
// and replace it with the corresponding %paramValue%. An example is included with
// this library to show how it is done.
// Pages are stored in the file system in directory '/page'.
//
void serverSendPage (String paramName[], String paramValue[], int paramCount, String pageName)

//
// Redirect the browser to another location. This can be convenient for example
// if you receive requests through a HTTP GET and you don't want the user to
// send the same info again. In that case, process the GET request and redirect
// to another page (or the same page without GET parameters)
//
void serverSendRedirect (String newLocation)

//
// If no pagename is given, it is assumed that the page requested
// has to be send.
//
void serverSendPage (String paramName[], String paramValue[], int paramCount)

//
// If no parameters are given, it is assumed that no parameter
// replacement is needed and that the requested page needs to be send.
//
void serverSendPage (String pageName)

//
// if no pagename and no parameters are given, it is assumed that the page
// requested has to be send where no parameter replacement is needed.
//
void serverSendPage ()

//
// Check whether there is a page requested from a client
// True is returned if a page is requested after which the main
// program should call a version of serverSendPage()
//
bool serverRequestReceived()

//
// Return the requested page
//
String pageRequested ()

//
// Return the value of the parameter requested or PARAMNOTFOUND
//
String giveURLParam (String search)

You can read about it in more detail on https://piandmore.wordpress.com/tag/pam_wifiserver/
