#include <Adafruit_GFX.h>				// https://github.com/adafruit/Adafruit-GFX-Library
#include <Fontx.h>							// https://github.com/h-nari/Fontx
#include <Humblesoft_GFX.h>			// https://github.com/h-nari/Humblesoft_GFX
#include <Humblesoft_LedMat.h>	// https://github.com/h-nari/Humblesoft_LedMat

#if 0
#include <fontx/ILGH16XB.h>
#include <fontx/ILGZ16XB.h>
RomFontx fontx(ILGH16XB,ILGZ16XB);
#endif

Humblesoft_LedMat ledMat;
uint8_t imgBuf[1024*8];

void demo1()
{
	ledMat.clear();
	ledMat.fillScreen(0);
	// ledMat.setFont(&fontx);

	ledMat.moveCursor(2,0);
	ledMat.setTextColor("red");
	ledMat.print("WiFi");
	ledMat.setTextColor("green");
	ledMat.print(" + ");
	ledMat.setTextColor("blue");
	ledMat.println("LED");

	ledMat.moveCursor(2, 0);
	ledMat.setTextColor("#004040");
	ledMat.print("on ");
	ledMat.setTextColor("white");
	ledMat.println("ARDUINO");
	
	ledMat.moveCursor(2, 5);
	ledMat.setTextColor("#ff00ff");
	ledMat.println("LED MATRIX");

	ledMat.moveCursor(2, 0);
	ledMat.setTextColor("#ffff00");
	ledMat.println("CONTROLLER");

	int x,y = ledMat.getCursorY() + 2;
	int h = ledMat.height() - y;
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
		ledMat.drawFastVLine(x, y, h, ledMat.rgb(r,g,b));
	}

	x = 31;
	y = 52;
	const char *mes = "FULL COLOR";
	ledMat.setTextColor("white");
	ledMat.alignPrintf(x+1, y+1, TA_CENTER, TA_CENTER, mes);
	ledMat.setTextColor("#001000");
	ledMat.alignPrintf(x, y, TA_CENTER, TA_CENTER, mes);
	ledMat.display();
}


void setup(void)
{
  Serial.begin(115200);
  delay(100);
  Serial.println("\n\nReset:");
	
  Serial.println("Initialize");
  ledMat.begin(LMMT64x32s16,1,2);
	ledMat.setImgBuf(imgBuf, sizeof imgBuf);

  demo1();
}

void loop(void){
	static int rotation;

	delay(1000);
	ledMat.checkSubcon();
	Serial.printf("rotation:%d\n",rotation);
	rotation = (rotation + 1) % 4;
	ledMat.setRotation(rotation);
	demo1();
}


/*** Local variables: ***/
/*** tab-width:2      ***/
/*** truncate-lines:t ***/
/*** End:             ***/
