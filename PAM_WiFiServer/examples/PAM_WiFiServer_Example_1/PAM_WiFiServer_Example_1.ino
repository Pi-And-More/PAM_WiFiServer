//////////////////////////////////////////////////////////////////////
//
//                    PI and more
//              PAM_WiFiServer library
//
// https://piandmore.wordpress.com/tag/pam_wifiserver
//
//////////////////////////////////////////////////////////////////////
//
// Include the PAM_WiFiServer library
//
#include <PAM_WiFiServer.h>
//
// Include the PAM_WiFiConnect library
//
#include <PAM_WiFiConnect.h>

void setup() {
//
// Connect to your wifi. Remember to change the SSID and password
//
  wifiConnect("YourSSID","YourPassword");
//
// Start the webserver
//
  wifiServerStart();
}

//
// This example just sends the standard index.htm regardless of what page is requested
// Please remember to upload the .htm file(s) to the ESP
//

void loop() {
//
// Check if there is a page request by a client
//
  bool pageRequest = serverRequestReceived();
//
// If there is a page request, send the index.htm file
//
  if (pageRequest) {
    serverSendPage("index.htm");
  } else {
    delay(20);
  }
}

