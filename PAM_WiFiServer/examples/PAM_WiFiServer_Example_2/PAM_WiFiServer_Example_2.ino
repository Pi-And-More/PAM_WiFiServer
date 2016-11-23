//////////////////////////////////////////////////////////////////////
//
//                    PI and more
//              PAM_WiFiServer library
//
// https://piandmore.wordpress.com/tag/pam_wifiserver
// Example 2: LM75A temperature sensor with webserver
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
//
// Include the PAM_Tools library
//
#include <PAM_Tools.h>
//
// Include the PAM_WebAPI library
//
#include <PAM_WebAPI.h>
//
// Include the library for the LM75A temperature sensor
//
#include <LM75A.h>

//
// Create an instance of the LM75A sensor.
//
LM75A lm75a_sensor;

//
// The parameters used with the webpages
//
#define NAMECOUNT 3
String names[NAMECOUNT] = { "name","temp","channelid" };

//
// Variable to hold the current temperature.
//
float temperature;
//
// Since the loop function cannot contain a long delay
// because the webserver needs to poll if there is a request,
// we have to configure (uploadCounterMax) and count
// (uploadCounter) how many times we have done the loop
// and know when we need to upload
//
int uploadCounter = 0;
int uploadCounterMax = 600*10;
//
// If we cannot upload we have to count how many errors we
// have before sending the next error message through
// IFTTT message type.
//
int errorCounter = 0;
int errorCounterMax = 100;
//
// The following variables are stored in the file system.
// You can if you want to set an initial value. This can
// also be done through the values in the /data/var folder
// as is done with this example.
//
// modulename = Module Name
// tsAPI = API key for ThingSpeak
// iftttAPI = API key for IFTTT
// alertEvent = IFTTT event type for alert/messaging
// bootEvent = IFTTT event type for send a boot message
// channelId = The channel id of our ThingSpeak channel
//
String modulename = "TemperatureLogger";
String tsAPI = "";
String iftttAPI = "";
String alertEvent = "";
String bootEvent = "";
String channelId = "";

void setup() {
  //
  // Start Serial. Not necessary with this example but
  // generally speaking convenient to see what is
  // going on.
  //
  Serial.begin(115200);
  Serial.println("");
  Serial.println("");
  //
  // All the variables are filled with the information
  // in the file system. If it is not there, the default
  // value is used as defined earlier.
  //
  modulename = getPutStringKey("var","Module-name",modulename);
  uploadCounterMax = getPutIntKey("var","Upload-every_100ms",uploadCounterMax);
  tsAPI = getPutStringKey("var","ThingSpeak-API",tsAPI);
  iftttAPI = getPutStringKey("var","IFTTT-API",iftttAPI);
  errorCounterMax = getPutIntKey("var","Alert-every_errors",errorCounterMax);
  alertEvent = getPutStringKey("var","IFTTT-msg-event",alertEvent);
  bootEvent = getPutStringKey("var","IFTTT-boot-event",bootEvent);
  channelId = getPutStringKey("var","TS Channel id",channelId);
  //
  // Set both the errorcounter and the uploadcounter to the
  // maximum so we know that at startup we log the temperature
  // right away and if we get our first error we are notified
  // immediately.
  //
  errorCounter = errorCounterMax;
  uploadCounter = uploadCounterMax;
  //
  // Signal we are ready for wifi
  //
  Serial.print(modulename);
  Serial.println(" Ready");
  Serial.println("");
  //
  // Start wifi. Since we are logging the temperature
  // to ThingSpeak, we use waitWiFiConnect instead of
  // wifiConnect because the device is useless if we
  // have no internet connection.
  //
  waitWiFiConnect();
  //
  // We are connected. Send a boot event through IFTTT.
  //
  Serial.println(" ");
  ifttt(iftttAPI,bootEvent,modulename,WiFi.localIP().toString());
  Serial.println(" ");
  //
  // Wifi connected, next we start the wifi server.
  //
  wifiServerStart();
}

//
// The main loop will check the temperature every cycle
// so if requested by a webpage we know the current temperature.
// It will also poll if there is a webclient requesting a page.
//
void loop() {
  //
  // Read the temperature using the LM75A library.
  //
  temperature = lm75a_sensor.getTemperatureInDegrees();
  //
  // If we get an invalid temperature, check whether we
  // have an IFTTT key and an alert event type. If so,
  // increase the counter and if we reach the threshold,
  // send the alert message.
  //
  if (temperature==INVALID_LM75A_TEMPERATURE) {
    if (alertEvent.length()>0 && iftttAPI.length()>0) {
      errorCounter++;
      if (errorCounter>=errorCounterMax) {
        ifttt(iftttAPI,alertEvent,modulename,WiFi.localIP().toString()+", error reading temperature.");
        errorCounter = 0;
      }
    }
  } else {
    //
    // We have a correct temperature. If we have cycled through
    // often enough, send an update to ThingSpeak.
    //
    if (tsAPI.length()>0) {
      uploadCounter++;
      if (uploadCounter>=uploadCounterMax) {
        thingspeak(tsAPI,temperature);
        uploadCounter = 0;
      }
    }
  }
  //
  // Check whether there is a webclient asking for a page
  //
  bool pageRequest = serverRequestReceived();
  if (pageRequest) {
    //
    // We have a page request. First send the request to
    // the pageSettings function which will handle
    // settings.htm, an automatically generated page
    // to manage the variables in our file system.
    //
    if (!pageSettings(modulename)) {
      //
      // The page was not handled by pageSettings, so
      // we fill our page variables and send the
      // default page, index.htm
      //
      String values[NAMECOUNT];
      values[0] = modulename;
      values[1] = String(temperature);
      values[2] = channelId;
      serverSendPage(names,values,NAMECOUNT,"index.htm");
    }
  }
  //
  // Delay to prevent the temperature sensor from being read
  // constantly while at the same time short enough that
  // the webclient will be detected in time
  //
  delay(100);
}

