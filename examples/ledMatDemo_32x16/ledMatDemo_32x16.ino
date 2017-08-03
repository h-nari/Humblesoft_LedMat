#include <Adafruit_GFX.h>				// https://github.com/adafruit/Adafruit-GFX-Library
#include <Fontx.h>							// https://github.com/h-nari/Fontx
#include <Humblesoft_GFX.h>			// https://github.com/h-nari/Humblesoft_GFX
#include <Humblesoft_LedMat.h>	// https://github.com/h-nari/Humblesoft_LedMat
#include <ScrollText.h>				  // included in Humblesoft_LedMat.h

#if 0
#include <fontx/ILGH16XB.h>
#include <fontx/ILGZ16XB.h>
RomFontx fontx(ILGH16XB,ILGZ16XB);
#endif

Humblesoft_LedMat ledMat;
ScrollText scroll(&ledMat);
uint8_t imgBuf[1024*4];

void plasma();

void demo1()
{
	ledMat.clear();

	ledMat.setCursor(1,0);
	ledMat.setTextColor("#ff0000");
	ledMat.print('1');
	ledMat.setTextColor("#ff8000");
	ledMat.print('6');
	ledMat.setTextColor("#ffff00");
	ledMat.print('x');
	ledMat.setTextColor("#80ff00");
	ledMat.print('3');
	ledMat.setTextColor("#00ff00");
	ledMat.print('2');

	ledMat.setCursor(1,9);
	ledMat.setTextColor("#00ffff");
	ledMat.print('*');
	ledMat.setTextColor("#0080ff");
	ledMat.print('R');
	ledMat.setTextColor("#0000ff");
	ledMat.print('G');
	ledMat.setTextColor("#8000ff");
	ledMat.print('B');
	ledMat.setTextColor("#ff00ff");
	ledMat.print('*');
	ledMat.display();
}

void setup(void)
{
  Serial.begin(115200);
  delay(100);
  Serial.println("\n\nReset:");
	
  Serial.println("Initialize");
	ledMat.setImgBuf(imgBuf, sizeof(imgBuf));
  ledMat.begin(LMMT32x16s8);
  // ledMat.begin(LMMT32x32s16);
  // ledMat.begin(LMMT64x32s16);

	scroll.setScrollArea();
	scroll.setYPos();
	scroll.setSpeed(32.0f);
	scroll.setTextColor("blue");
}

void loop(void){
	ledMat.checkSubcon();
	
	scroll.scrollOut();
	while(scroll.update())
		delay(0);

	ledMat.setBright(20);
	scroll.print("Hello World!");
	scroll.scrollOut();
	while(scroll.update())
		delay(0);
	delay(1000);
	
	ledMat.clear();
	ledMat.display();
	ledMat.setBright(20);
	demo1();
	delay(2000);
	
	ledMat.clear();
	ledMat.display();
	ledMat.setBright(5);
	for(int i=0; i<200;i++){
		plasma();
		delay(10);
	}
}


/*** Local variables: ***/
/*** tab-width:2      ***/
/*** truncate-lines:t ***/
/*** End:             ***/
