#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

#include <Adafruit_GFX.h>				// https://github.com/adafruit/Adafruit-GFX-Library
#include <Fontx.h>							// https://github.com/h-nari/Fontx
#include <FsFontx.h>
#include <Humblesoft_GFX.h>			// https://github.com/h-nari/Humblesoft_GFX
#include <Humblesoft_LedMat.h>	// https://github.com/h-nari/Humblesoft_LedMat
#include <FsHandler.h>					// https://github.com/h-nari/FsHandler
#include <SpiffsHandler.h>

#define WIFI_SSID 		"__your_wifi_ssid__"
#define	WIFI_PASSWORD	"__your_wifi_password__"

ESP8266WebServer server(80);
SpiffsHandler fsHandler("rom",true,true);
FsFontx fx("/ILGH16XB.FNT","/ILGZ16XB.FNT");
uint8_t imgBuf[1024*4];

void setup(void)
{
  Serial.begin(115200);
  delay(100);
  Serial.println("\n\nReset:");

	WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  server.on("/", []{
      String s;
      s += F("<h1>ledMatDemo_fsfontx_webserver</h1>");
			s += F("<p>");
      s += F("<a href='./rom/'>rom</a><br/>");
			s += F("</p>");
      server.send(200,"text/html",s);
    });
	
  server.addHandler(&fsHandler);
  server.begin();
  Serial.println("HTTP server started");
	
  LedMat.begin(LMMT64x32s16);
	LedMat.setLedMode(1);
	LedMat.setImgBuf(imgBuf, sizeof imgBuf);
	if(fx.checkFontFile()){
		LedMat.setFont(&fx);
	} else {
		Serial.println("Fontx files not found.");
		Serial.println("Contents of SPIFFS");
		fx.dumpFileSystem();
		Serial.println();
	}
	LedMat.println("Hello");
	LedMat.println("漢字");
	LedMat.display();
}

void loop(void){
  server.handleClient();
}


/*** Local variables: ***/
/*** tab-width:2      ***/
/*** truncate-lines:t ***/
/*** End:             ***/
