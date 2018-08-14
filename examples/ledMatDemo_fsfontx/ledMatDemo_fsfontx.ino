#include <Adafruit_GFX.h>				// https://github.com/adafruit/Adafruit-GFX-Library
#include <Fontx.h>							// https://github.com/h-nari/Fontx
#include <FsFontx.h>
#include <Humblesoft_GFX.h>			// https://github.com/h-nari/Humblesoft_GFX
#include <Humblesoft_LedMat.h>	// https://github.com/h-nari/Humblesoft_LedMat

FsFontx fx("/ILGH16XB.FNT","/ILGZ16XB.FNT");
uint8_t imgBuf[1024*4];

void setup(void)
{
  Serial.begin(115200);
  delay(100);
  Serial.println("\n\nReset:");

	SPIFFS.begin();
	
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
	LedMat.println("世界");
	LedMat.display();
}

void loop(void){
}


/*** Local variables: ***/
/*** tab-width:2      ***/
/*** truncate-lines:t ***/
/*** End:             ***/
