#include <Adafruit_GFX.h>				// https://github.com/adafruit/Adafruit-GFX-Library
#include <Fontx.h>							// https://github.com/h-nari/Fontx
#include <Humblesoft_GFX.h>			// https://github.com/h-nari/Humblesoft_GFX
#include <Humblesoft_LedMat.h>	// https://github.com/h-nari/Humblesoft_LedMat

uint8_t imgBuf[1024*4];

void demo1()
{
	LedMat.clear();
	LedMat.fillScreen(0);

	LedMat.moveCursor(2,0);
	LedMat.setTextColor("red");
	LedMat.print("WiFi");
	LedMat.setTextColor("green");
	LedMat.print(" + ");
	LedMat.setTextColor("blue");
	LedMat.println("LED");

	LedMat.moveCursor(2, 0);
	LedMat.setTextColor("#004040");
	LedMat.print("on ");
	LedMat.setTextColor("white");
	LedMat.println("ARDUINO");
	
	LedMat.moveCursor(2, 5);
	LedMat.setTextColor("#ff00ff");
	LedMat.println("LED MATRIX");

	LedMat.moveCursor(2, 0);
	LedMat.setTextColor("#ffff00");
	LedMat.println("CONTROLLER");

	int x,y = LedMat.getCursorY() + 2;
	int h = LedMat.height() - y;
	int w = 64;
	
	for(x=0; x<w; x++){
		int r,g,b,xx;
		int q = w/4;
		if(x < q){
			r = 255;
			g = b = 255 * (q - x) / q;
		} else if(x < 2*q) {
			xx = x - q;
			r = 255 * (q - xx) / q;
			g = 255 * xx / q;
			b = 0;
		} else if(x < 3*q) {
			xx = x - 2*q;
			r = 0;
			g = 255 * (q - xx) / q;
			b = 255 * xx / q;
		} else {
			xx = x - 3*q;
			r = g = 255 * xx / q;
			b = 255;
		}
		LedMat.drawFastVLine(x, y, h, LedMat.rgb(r,g,b));
	}

	x = 31;
	y = 52;
	const char *mes = "FULL COLOR";
	LedMat.setTextColor("white");
	LedMat.alignPrintf(x+1, y+1, TA_CENTER, TA_CENTER, mes);
	LedMat.setTextColor("#001000");
	LedMat.alignPrintf(x, y, TA_CENTER, TA_CENTER, mes);
	LedMat.display();
}


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
