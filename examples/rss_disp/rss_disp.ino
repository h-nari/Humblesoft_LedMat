#include "conf.h"

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPClient.h>
#include <time.h>

#include <Adafruit_GFX.h>
#include <Fontx.h>
#include <fontx/ILGH16XB.h>
#include <fontx/ILGZ16XB.h>
#include <Humblesoft_GFX.h>
#include <Humblesoft_LedMat.h>
#include <ScrollText.h>
#include "rssParser.h"
#include "rssBuf.h"
#include "layout.h"

#define DAYSEC  (3600*24)

void date_disp(LayoutElem *elem, bool bInit);
void time_disp(LayoutElem *elem, bool bInit);
void rss_disp(LayoutElem *elem, bool bInit);

enum AppState {
  AS_READ,
  AS_DISP,
};

enum SW_STATE {
	SWS_Pushed,
	SWS_Released,
	SWS_Stabled,
};

const char *site[] = {
  "http://numbers2007.blog123.fc2.com/?xml",
  "http://feeds.feedburner.com/make_jp",
  "http://www.jiji.com/rss/ranking.rdf",
  "http://rss.rssad.jp/rss/impresswatch/pcwatch.rdf",
  "http://rss.asahi.com/rss/asahi/newsheadlines.rdf",
  NULL
};

Layout layouts[] = {
	{
		{
			{ 0,  0, 64,  8, date_disp},
			{ 0,  8, 64,  8, time_disp},
			{ 0, 16, 64, 16, rss_disp },
		},
	},
};

uint8_t imgBuf[1024];

AppState gState;
int httpCode;
int iItem;
uint8_t iLayout;

RssBuf rssBuf;
RssParser rss;
const char **pSite = site;

SW_STATE sw_state = SWS_Stabled;
unsigned long sw_release_time;

RomFontx fontx(ILGH16XB,ILGZ16XB);
ScrollText scroll(&LedMat);

void rss_update()
{
  if(gState == AS_READ){
    rssBuf.clear();

    HTTPClient http;
    Serial.printf("[HTTP]begin..\n");

    http.begin(*pSite);
    Serial.printf("[HTTP]GET %s\n",*pSite);

    httpCode = http.GET();
    if(httpCode > 0){
      Serial.printf("[HTTP] GET ... code: %d\n",httpCode);
      if(httpCode == HTTP_CODE_OK) {
				int len = http.getSize();
				uint8_t buff[128] = {0};
				WiFiClient *stream = http.getStreamPtr();
				while(http.connected() && (len > 0 || len == -1)){
					size_t size = stream->available();
					if(size){
						if(size > sizeof buff) size = sizeof buff;
						int c = stream->readBytes(buff,size);
						for(int i=0; i<c; i++)
							rss.put(buff[i]);
						if(len > 0) len -= c;
					}
					delay(0);
				}
				Serial.printf("[HTTP] ItemCount:%d\n", rssBuf.getItemCount());
      }
      gState = AS_DISP;
			iItem = 0;
    } else {
      Serial.printf("[HTTP] GET ... failed, error: %s\n",
										http.errorToString(httpCode).c_str());
      if(*++pSite == NULL) pSite = site;
      gState = AS_READ;
    }
    http.end();
  }
  else if(gState == AS_DISP) {
		if(scroll.update()){
			// delay(10);
		} else {
			if(iItem < rssBuf.getItemCount()){
				scroll.print(rssBuf.getItem(iItem++));
				scroll.scrollOut();
			} else {
				if(*++pSite == NULL) pSite = site;
				gState = AS_READ;
			}
		}
  }
}

void rss_disp(LayoutElem *elem, bool bInit)
{
	int16_t  x = elem->x;
	int16_t  y = elem->y;
	uint16_t w = elem->w;
	uint16_t h = elem->h;
	uint8_t textSize = h > 16 ? 2 : 1;

	if(bInit){
		scroll.setSpeed(32 * textSize);
		scroll.setTextSize(textSize);
		scroll.setScrollArea(x, y, w, h);
		scroll.setYPos();
		scroll.setTextColor("red");
	} else {
		rss_update();
	}
}

void date_disp(LayoutElem *elem, bool bInit)
{
	int16_t  x = elem->x;
	int16_t  y = elem->y;
	uint16_t w = elem->w;
	uint16_t h = elem->h;
	uint8_t textSize = h > 16 ? 2 : 1;
  static time_t t0;
  time_t t = time(NULL);
  time_t dt = t / DAYSEC * DAYSEC;

  if(bInit || dt != t0){
    struct tm *tm = localtime(&t);
    const char *wd[7] = {"Sun","Mon","Tue","Wed","Thr","Fri","Sat"};
    LedMat.setTextColor("yellow","black");
    LedMat.setTextSize(textSize);
		LedMat.alignPrintf(x+w/2,y, TA_CENTER, TA_TOP,
											 "%d/%d(%s)", 
											 tm->tm_mon+1, tm->tm_mday, wd[tm->tm_wday]);
    t0 = dt;
	}
}

void time_disp(LayoutElem *elem, bool bInit)
{
	int16_t  x = elem->x;
	int16_t  y = elem->y;
	uint16_t w = elem->w;
	uint16_t h = elem->h;
	uint8_t textSize = h > 16 ? 2 : 1;
  static time_t t0;
  time_t t = time(NULL);

  if(t != t0){
    struct tm *tm = localtime(&t);
    LedMat.setTextSize(textSize);
		LedMat.setTextColor("green","black");
    LedMat.alignPrintf(x+w/2,y, TA_CENTER, TA_TOP,
											 "%02d:%02d:%02d", tm->tm_hour, tm->tm_min, tm->tm_sec);
    t0 = t;
  }
}

void setup()
{
  Serial.begin(115200);
  delay(100);
  Serial.println("\n\nReset:");

  LedMat.begin();
	LedMat.setImgBuf(imgBuf, sizeof imgBuf);
	LedMat.setPlane(1);
	LedMat.setLedMode(1);			// for HSLM-6432P4B
  LedMat.setRotation(0);
  LedMat.clear();
  LedMat.display();
  scroll.setTextSize(1);
  scroll.setYPos();

  scroll.setFont(&fontx);
	LedMat.setFont(&fontx);
	LedMat.setTextWrap(false);

	LedMat.clear();
	LedMat.setTextSize(1);
	LedMat.setCursor(0,0);
	LedMat.println("connecting to network");
	LedMat.display();

	WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
	while(WiFi.status() != WL_CONNECTED) {
		Serial.print('.');
		LedMat.print('.');
		LedMat.display();
		delay(500);
	}
	Serial.println();
	Serial.printf("Connected, IP address: ");
	Serial.println(WiFi.localIP());
	LedMat.print("\nIP address: ");
	LedMat.println(WiFi.localIP());

	configTime( 9 * 3600, 0,
							"ntp.nict.jp", "ntp.jst.mfeed.ad.jp", NULL);

	gState = AS_READ;

	delay(500);
	iLayout = 0;
	LedMat.clear();
	layout_set(&layouts[iLayout]);
	LedMat.display();
}

void loop()
{
	layout_update(&layouts[iLayout]);
}

/*** Local variables: ***/
/*** tab-width:2 ***/
/*** End: ***/
