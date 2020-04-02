#include <Adafruit_GFX.h>
#include <Arduino.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <Fontx.h>
#include <Humblesoft_GFX.h>
#include <Humblesoft_LedMat.h>
#include <ScrollText.h>
#include <WiFiClientSecureBearSSL.h>
#include <fontx/ILGH16XB.h>
#include <fontx/ILGZ16XB.h>
#include <time.h>

#include "layout.h"
#include "rssBuf.h"
#include "rssParser.h"

#define DAYSEC (3600 * 24)

#define WIFI_SSID "your_wifi_ssid"
#define WIFI_PASSWORD "your_wifi_passwd"

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
typedef struct {
  const char *name;
  int max;
  const char *url;
  const char *figerprint;
} site_t;

const char *yahoo_fingerprint = "2c4cc1be32d3a6021bbce7f918290af709972130";

const site_t site[] = {
    {"Yahoo News", 20, "https://news.yahoo.co.jp/pickup/rss.xml",
     yahoo_fingerprint},
    {"BBC", 20, "https://headlines.yahoo.co.jp/rss/bbc-c_int.xml",
     yahoo_fingerprint},
    {"CNN", 20, "https://headlines.yahoo.co.jp/rss/cnn-c_int.xml",
     yahoo_fingerprint},
    {"お笑いナタリー", 10,
     "https://headlines.yahoo.co.jp/rss/natalieo-c_ent.xml", yahoo_fingerprint},
    {"アスキー", 20, "https://headlines.yahoo.co.jp/rss/ascii-c_sci.xml",
     yahoo_fingerprint},
    {"熊日新聞", 20, "https://headlines.yahoo.co.jp/rss/kumanichi-loc.xml",
     yahoo_fingerprint},
    {"Yahoo Sports", 10, "https://news.yahoo.co.jp/pickup/sports/rss.xml",
     yahoo_fingerprint},
    {"Yahoo IT", 10, "https://news.yahoo.co.jp/pickup/computer/rss.xml",
     yahoo_fingerprint},
    {"sankei", 10, "https://headlines.yahoo.co.jp/rss/san-dom.xml",
     yahoo_fingerprint},
    {"Make", 5, "https://feeds.feedburner.com/make_jp",
     "058b82bcfeb3f43afa4f7745d2d048e36e29bdce"},
    {NULL},
};

Layout layouts[] = {
    {
        {
            {0, 0, 64, 8, date_disp},
            {0, 8, 64, 8, time_disp},
            {0, 16, 64, 16, rss_disp},
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
const site_t *pSite = site;

SW_STATE sw_state = SWS_Stabled;
unsigned long sw_release_time;

RomFontx fontx(ILGH16XB, ILGZ16XB);
ScrollText scroll(&LedMat);
unsigned long tStart;

static void site_next() {
  pSite++;
  if (pSite->name == NULL) {
    unsigned long tNow = millis();
    pSite = site;
    Serial.printf("loop time: %d sec\n", (tNow - tStart) / 1000);
  }
}

void rss_update() {
  if (gState == AS_READ) {
    rssBuf.clear();
    rssBuf.setItemMax(pSite->max);
    rss.init();

    std::unique_ptr<BearSSL::WiFiClientSecure> client(
        new BearSSL::WiFiClientSecure);
    HTTPClient http;
    Serial.printf("[HTTP]begin %s\n", pSite->name);

    client->setFingerprint(pSite->figerprint);

    if (!http.begin(*client, pSite->url)) {
      Serial.printf("connection failed.\n");
      scroll.printf("[[%sへの接続に失敗しました。]]  ", pSite->name);
      site_next();
    } else {
      Serial.printf("connected\n");

      httpCode = http.GET();
      if (httpCode > 0) {
        Serial.printf("[HTTP] GET ... code: %d\n", httpCode);
        if (httpCode == HTTP_CODE_OK) {
          int len = http.getSize();
          uint8_t buff[128] = {0};
          WiFiClient *stream = http.getStreamPtr();
          unsigned long tStart = millis();
          while (http.connected() && (len > 0 || len == -1)) {
            size_t size = stream->available();
            if (size) {
              if (size > sizeof buff) size = sizeof buff;
              int c = stream->readBytes(buff, size);
              for (int i = 0; i < c; i++) rss.put(buff[i]);
              if (len > 0) len -= c;
              tStart = millis();
            } else if (millis() - tStart > 5000) {
              break;
            } else if (rssBuf.isFull()) {
              break;
            }
            layout_update(&layouts[iLayout], rss_disp);
          }
          Serial.printf("[HTTP] ItemCount:%d\n", rssBuf.getItemCount());
        }
        gState = AS_DISP;
        iItem = 0;
      } else {
        Serial.printf("[HTTP] GET ... failed, error: %s\n",
                      http.errorToString(httpCode).c_str());
        scroll.printf("[[%sからの取得に失敗しました。code:%d]]  ", pSite->name,
                      httpCode);
        site_next();
        gState = AS_READ;
      }
    }
    http.end();
  } else if (gState == AS_DISP) {
    if (scroll.update()) {
      // delay(10);
    } else {
      if (iItem < rssBuf.getItemCount()) {
        scroll.print(rssBuf.getItem(iItem++));
        scroll.scrollOut();
      } else {
        site_next();
        gState = AS_READ;
      }
    }
  }
}

void rss_disp(LayoutElem *elem, bool bInit) {
  int16_t x = elem->x;
  int16_t y = elem->y;
  uint16_t w = elem->w;
  uint16_t h = elem->h;
  uint8_t textSize = h > 16 ? 2 : 1;

  if (bInit) {
    scroll.setSpeed(32 * textSize);
    scroll.setTextSize(textSize);
    scroll.setScrollArea(x, y, w, h);
    scroll.setYPos();
    scroll.setTextColor("red");
  } else {
    rss_update();
  }
}

void date_disp(LayoutElem *elem, bool bInit) {
  int16_t x = elem->x;
  int16_t y = elem->y;
  uint16_t w = elem->w;
  uint16_t h = elem->h;
  uint8_t textSize = h > 16 ? 2 : 1;
  static time_t t0;
  time_t t = time(NULL);
  time_t dt = t / DAYSEC * DAYSEC;

  if (bInit || dt != t0) {
    struct tm *tm = localtime(&t);
    const char *wd[7] = {"Sun", "Mon", "Tue", "Wed", "Thr", "Fri", "Sat"};
    LedMat.setTextColor("yellow", "black");
    LedMat.setTextSize(textSize);
    LedMat.fillRect(x, y, w, h, 0);
    LedMat.alignPrintf(x + w / 2, y, TA_CENTER, TA_TOP, "%d/%d(%s)",
                       tm->tm_mon + 1, tm->tm_mday, wd[tm->tm_wday]);
    t0 = dt;
  }
}

void time_disp(LayoutElem *elem, bool bInit) {
  int16_t x = elem->x;
  int16_t y = elem->y;
  uint16_t w = elem->w;
  uint16_t h = elem->h;
  uint8_t textSize = h > 16 ? 2 : 1;
  static time_t t0;
  time_t t = time(NULL);

  if (t != t0) {
    struct tm *tm = localtime(&t);
    LedMat.setTextSize(textSize);
    LedMat.setTextColor("green", "black");
    LedMat.alignPrintf(x + w / 2, y, TA_CENTER, TA_TOP, "%02d:%02d:%02d",
                       tm->tm_hour, tm->tm_min, tm->tm_sec);
    t0 = t;
  }
}

void setup() {
  Serial.begin(115200);
  delay(100);
  Serial.println("\n\nReset:");

  LedMat.begin(LMMT64x32s16, 1, 1);
  LedMat.setImgBuf(imgBuf, sizeof imgBuf);
  LedMat.setPlane(1);
  LedMat.setLedMode(1);  // for HSLM-6432P4B,色が変だったら値を0に変える 
  LedMat.setRotation(0);
  LedMat.setBright(10);  // 1..100
  LedMat.clear();
  LedMat.display();
  scroll.setTextSize(1);
  scroll.setYPos();

  scroll.setFont(&fontx);
  LedMat.setFont(&fontx);
  LedMat.setTextWrap(false);

  LedMat.clear();
  LedMat.setTextSize(1);
  LedMat.setCursor(0, 0);
  LedMat.println("connecting to network");
  LedMat.display();

  uint8_t mac[6];
  WiFi.macAddress(mac);
  Serial.print("mac:");
  for (int i = 0; i < 6; i++)
    Serial.printf("%c%02x", i > 0 ? '-' : ' ', mac[i]);
  Serial.println();

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  unsigned long tStart = millis();
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    LedMat.print('.');
    LedMat.display();
    delay(500);
    if (millis() - tStart > 20 * 1000) ESP.restart();
  }
  Serial.println();
  Serial.printf("Connected, IP address: ");
  Serial.println(WiFi.localIP());
  LedMat.print("\nIP address: ");
  LedMat.println(WiFi.localIP());

  configTime(9 * 3600, 0, "ntp.nict.jp", "ntp.jst.mfeed.ad.jp", NULL);

  gState = AS_READ;

  delay(500);
  iLayout = 0;
  LedMat.clear();
  layout_set(&layouts[iLayout]);
  LedMat.display();
}

void loop() { layout_update(&layouts[iLayout]); }

/*** Local variables: ***/
/*** tab-width:2 ***/
/*** End: ***/
