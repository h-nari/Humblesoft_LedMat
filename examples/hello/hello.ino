#include <Adafruit_GFX.h>				// https://github.com/adafruit/Adafruit-GFX-Library
#include <Fontx.h>							// https://github.com/h-nari/Fontx
#include <Humblesoft_GFX.h>			// https://github.com/h-nari/Humblesoft_GFX
#include <Humblesoft_LedMat.h>	// https://github.com/h-nari/Humblesoft_LedMat

uint8_t imgBuf[1024*4];

void setup(void)
{
  Serial.begin(115200);
  delay(100);
  Serial.println("\n\nReset:");
	
  LedMat.begin(LMMT64x32s16);
	LedMat.setLedMode(1);
	LedMat.setImgBuf(imgBuf, sizeof imgBuf);

	LedMat.println("Hello");
	LedMat.display();
	delay(1000);
}

void loop(void){
	static unsigned int ci = 0;
	static const char *colors[] = {
		"WHITE","RED","GREEN","BLUE","YELLOW",
	};
	
	LedMat.checkSubcon();
	if(ci >= sizeof(colors)/sizeof(colors[0]))
		ci = 0;

	uint16_t fg = LedMat.rgb(colors[ci]);

	LedMat.clear();
	LedMat.setTextColor(fg);
	LedMat.setCursor(4,8);
	LedMat.println(colors[ci]);
	LedMat.drawRect(0, 0, LedMat.width(), LedMat.height(), fg);
	LedMat.display();
	ci++;

	delay(1000);
}


/*** Local variables: ***/
/*** tab-width:2      ***/
/*** truncate-lines:t ***/
/*** End:             ***/
