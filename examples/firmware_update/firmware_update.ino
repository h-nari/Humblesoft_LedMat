#include <ESP8266WiFi.h>
#include <Adafruit_GFX.h>       // https://github.com/adafruit/Adafruit-GFX-Library
#include <Fontx.h>              // https://github.com/h-nari/Fontx
#include <Humblesoft_GFX.h>     // https://github.com/h-nari/Humblesoft_GFX
#include <Humblesoft_LedMat.h>  // https://github.com/h-nari/Humblesoft_LedMat

#include "bnh_header.h"

const char *ssid     = "input_your_wifi_ssid";
const char *password = "input_your_wifi_passowrd";

const char *host = "www.humblesoft.com";
const char *path = "/pub/HSES-LMC1/";

int verbose = 0;

void setup(void)
{
  uint8_t buf[1024-2];
  size_t n;
  char filename[40];
  
  Serial.begin(115200);
  delay(100);
  Serial.println("\n\nReset:");
  Serial.println("Firmware Update program");
  Serial.println();

	Serial.println("Initialize LedMat");
  LedMat.setVerbose(1);
  if(!LedMat.begin()){
    Serial.println("LedMat.begin() failed.");
    return;
  }

	
  WiFi.mode(WIFI_STA);
  Serial.printf("Connected to %s .",ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("Connected.");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  Serial.println();
  
  while(1){
    Serial.println("Do you update HSES-LMC1 firmware ? [y/n]");
    do n = Serial.readBytesUntil('\n', buf, sizeof buf);
		while(n == 0);

    if(buf[0] == 'y') break;
    if(buf[0] == 'n') {
      Serial.println("firmware_update program exit.");
      Serial.println("Press Reset to restart this program.");
      return;
    }
    Serial.println("Press y or n");
  }
  Serial.println();

  WiFiClient client;
  const int port = 80;

  if(!client.connect(host, port)){
    Serial.println("connection failed.");
    return;
  }

  String url = path;
  url += "firmware.txt";

  Serial.printf("Requesting URL:%s\n",url.c_str());
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
	       "Host: " + host + "\r\n" +
	       "Connection: close\r\n\r\n");
  unsigned long start = millis();
  while(client.available() == 0){
    if(millis() - start > 5000){
      Serial.println("client timeout.");
      client.stop();
      return;
    }
  }

  
  while(client.available()){
    n = client.readBytesUntil('\n', buf, sizeof(buf)-1);
    buf[n] = 0;
    if(buf[0] == '\r') break;
    if(verbose) Serial.printf("[Header]:%s\n",buf);
  }

  filename[0] = 0;
  while(client.available()){
    n = client.readBytesUntil('\n', buf, sizeof(buf)-1);
    buf[n] = 0;
    if(verbose) Serial.printf("[Contents]:%s\n",buf);
    
    const char *p = (char *)buf;
    if(*p && !isspace(*p)){
      while(*p && !isspace(*p)) p++;
      if(*p){
	while(isspace(*p)) p++;
	if(*p){
	  uint16_t i;
	  for(i=0; i<sizeof(filename)-1; i++){
	    if(*p == 0 || isspace(*p)) break;
	    filename[i] = *p++;
	  }
	  filename[i] = 0;
	  Serial.printf("filename:%s\n",filename);
	}
      }
    }
  }

  if(!filename[0]){
    Serial.println("no filename found.");
    return;
  }

  url = path;
  url += filename;
  Serial.printf("Requesting URL:%s\n",url.c_str());

  if(!client.connect(host, port)){
    Serial.println("connection failed.");
    return;
  }

  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
	       "Host: " + host + "\r\n" +
	       "Connection: close\r\n\r\n");

  start = millis();
  while(client.available() == 0){
    if(millis() - start > 5000){
      Serial.println("client timeout.");
      client.stop();
      return;
    }
  }

  // http header読み飛ばし
  
  while(client.available()){
    n = client.readBytesUntil('\n', buf, sizeof(buf)-1);
    buf[n] = 0;
    if(buf[0] == '\r') break;
    if(verbose) Serial.printf("[Header]:%s\n",buf);
    delay(0);
  }
  
  // bnh header 読み込み

  bnh_header_t header;
  n = client.readBytes((uint8_t *)&header, sizeof header);
  if(n != sizeof header){
    Serial.println("BNH header read error");
    return;
  }

  if(strcmp(header.signature, BNH_SIGNATURE)){
    Serial.printf("Bad BNH Signature:%s\n", header.signature);
    return;
  }

  if(!LedMat.firmwareBegin(header.salt, !!(header.flags & BNH_FLAG_ENCRYPTED),
			   header.addr_min, header.addr_max)){
    Serial.println("firmwareBegin failed.");
    return;
  }
  
  int size = 0;
  while(client.available()){
    n = client.readBytes(buf, sizeof buf);
    if(!LedMat.firmwareData(buf, n)){
      Serial.printf("firmwareData failed. pos:%d:%u\n",size,n);
      return;
    }
    size += n;
    Serial.print('#');
  }
  Serial.println();
  if(!LedMat.firmwareEnd()){
    Serial.printf("firmwareEnd failed. size:%d\n",size);
    return;
  }
  Serial.printf("write %d bytes.\n",size);
  Serial.println("firmware update successed.");
}

void loop(void)
{
}


/*** Local variables: ***/
/*** tab-width:2      ***/
/*** truncate-lines:t ***/
/*** End:             ***/
