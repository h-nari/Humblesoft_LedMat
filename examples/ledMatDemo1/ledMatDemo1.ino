#include <Adafruit_GFX.h>				// https://github.com/adafruit/Adafruit-GFX-Library
#include <Fontx.h>							// https://github.com/h-nari/Fontx
#include <Humblesoft_GFX.h>			// https://github.com/h-nari/Humblesoft_GFX
#include <Humblesoft_LedMat.h>	// https://github.com/h-nari/Humblesoft_LedMat

uint8_t imgBuf[1024*8];

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
	
  Serial.println("Initialize");
  LedMat.begin(LMMT64x32s16,1,2);
	LedMat.setImgBuf(imgBuf, sizeof imgBuf);

  demo1();
}

void loop(void){
	static int rotation;

	delay(1000);
	LedMat.checkSubcon();
	Serial.printf("rotation:%d\n",rotation);
	rotation = (rotation + 1) % 4;
	LedMat.setRotation(rotation);
	demo1();
}


/*** Local variables: ***/
/*** tab-width:2      ***/
/*** truncate-lines:t ***/
/*** End:             ***/
