#include <ESP8266WiFi.h>
#include "aWOT.h"
#include "StaticFiles.h"
#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRutils.h>
#include <IRsend.h>


#define LED_BUILTIN D4
#define LED_GREEN D8

const char WiFiAPPSK[] = "coinpass";
const uint16_t kIrLed = D2;
IRsend irsend(kIrLed);

const uint16_t kRecvPin = D5;
IRrecv irrecv(kRecvPin);
decode_results results;

WiFiServer server(80);
Application app;
bool ledOn;
bool codeRecorded=false;

  void readLed(Request &req, Response &res) {
    res.print(ledOn);
  }

  void updateLed(Request &req, Response &res) {
    ledOn = (req.read() != '0');
    digitalWrite(LED_BUILTIN, ledOn);
    digitalWrite(LED_GREEN, !ledOn);
    return readLed(req, res);
  } 

void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  irrecv.enableIRIn();
  WiFi.mode(WIFI_AP);

  // Do a little work to get a unique-ish name. Append the
  // last two bytes of the MAC (HEX'd) to "Coin-":
  uint8_t mac[WL_MAC_ADDR_LENGTH];
  WiFi.softAPmacAddress(mac);
  String macID = String(mac[WL_MAC_ADDR_LENGTH - 2], HEX) +
    String(mac[WL_MAC_ADDR_LENGTH - 1], HEX);
  macID.toUpperCase();
  String AP_NameString = "Coin_" + macID;

  char AP_NameChar[AP_NameString.length() + 1];
  memset(AP_NameChar, 0, AP_NameString.length() + 1);

  for (int i=0; i<AP_NameString.length(); i++)
    AP_NameChar[i] = AP_NameString.charAt(i);

  
  app.get("/led", &readLed);
  app.put("/led", &updateLed);
  app.route(staticFiles());
  WiFi.softAP(AP_NameChar, WiFiAPPSK);
  irsend.begin();
  irrecv.decode(&results);
  server.begin();
}

void loop() {
  WiFiClient client = server.available();
 
  if (client.connected()) {
    app.process(&client);
  if(ledOn){
      Serial.println("test");
      irsend.sendRaw(resultToRawArray(&results),getCorrectedRawLength(&results),38);
      Serial.println(resultToSourceCode(&results));
      codeRecorded=false;
      irrecv.resume();
    }
    else{
       if (irrecv.decode(&results)&&(codeRecorded==false)) {
      // print() & println() can't handle printing long longs. (uint64_t)
      if (results.decode_type != UNKNOWN){
      Serial.println(resultToSourceCode(&results));
      codeRecorded=true;
      Serial.println("");
        // Receive the next value
      }
  }
  delay(100);  
    }
  }
}
