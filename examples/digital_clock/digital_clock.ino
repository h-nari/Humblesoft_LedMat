#include <time.h>
#include <ESP8266WiFi.h>

#include <Adafruit_GFX.h>				// https://github.com/adafruit/Adafruit-GFX-Library
#include <Fontx.h>							// https://github.com/h-nari/Fontx
#include <Humblesoft_GFX.h>			// https://github.com/h-nari/Humblesoft_GFX
#include <Humblesoft_LedMat.h>	// https://github.com/h-nari/Humblesoft_LedMat

#include <fontx/ILGH16XB.h>
#include <fontx/ILGZ16XB.h>

#include "conf.h"
// #define WIFI_SSID 	  "your_wifi_ssid"
// #define	WIFI_PASSWORD	"your_wifi_password"

#define JST	3600*9

RomFontx kfont(ILGH16XB,ILGZ16XB);

uint8_t imgBuf[1024*4];

void setup(void)
{
  Serial.begin(115200);
  delay(100);
  Serial.println("\n\nReset:");
	
  Serial.println("Initialize");
  LedMat.begin(LMMT64x32s16,1,1);
	LedMat.setLedMode(1);			// for HSLM-6432P4B
	LedMat.setImgBuf(imgBuf, sizeof imgBuf);
	LedMat.setBright(10);
	LedMat.clear();
	LedMat.println("Clock");
	LedMat.display();
	
  WiFi.mode(WIFI_STA);
  Serial.printf("Connected to %s .",WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("Connected.");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

	configTime( JST, 0, "ntp.nict.jp", "ntp.jst.mfeed.ad.jp");
}

void loop(void){
	static time_t t0;
	time_t t;
	struct tm *tm;
	static const char *wd[7] = {"日","月","火","水","木","金","土"};
	
	LedMat.checkSubcon();
	
	t = time(NULL);
	if(t != t0){
		tm = localtime(&t);

		Serial.printf("%02d/%02d/%02d %02d:%02d:%02d\n",
									tm->tm_year + 1900, tm->tm_mon+1, tm->tm_mday,
									tm->tm_hour,tm->tm_min,tm->tm_sec);

		LedMat.clear();
		LedMat.setFont();
		LedMat.setTextColor("red");
		LedMat.printf(" %4d\n", tm->tm_year+1900);
		LedMat.setTextColor("white");
		LedMat.printf(" %2d/%d",tm->tm_mon+1, tm->tm_mday);
		LedMat.setTextColor("red");
		
		LedMat.setFont(&kfont);
		LedMat.setCursor(40,0);
		LedMat.setTextColor("yellow");
		LedMat.printf("%s\n", wd[tm->tm_wday]);
		LedMat.setTextColor("green");
		LedMat.printf("%2d:%02d:%02d\n",tm->tm_hour,tm->tm_min,tm->tm_sec);
		LedMat.display();
		
		t0 = t;
	}
}


/*** Local variables: ***/
/*** tab-width:2      ***/
/*** truncate-lines:t ***/
/*** End:             ***/
