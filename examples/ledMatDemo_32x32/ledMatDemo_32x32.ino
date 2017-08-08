#include <Adafruit_GFX.h>				// https://github.com/adafruit/Adafruit-GFX-Library
#include <Fontx.h>							// https://github.com/h-nari/Fontx
#include <Humblesoft_GFX.h>			// https://github.com/h-nari/Humblesoft_GFX
#include <Humblesoft_LedMat.h>	// https://github.com/h-nari/Humblesoft_LedMat
#include <ScrollText.h>				  // included in Humblesoft_LedMat.h

ScrollText scroll(&LedMat);
uint8_t imgBuf[1024*4];

void plasma();

void demo1()
{
	LedMat.clear();

	LedMat.setCursor(1,0);
	LedMat.setTextColor("#ff0000");
	LedMat.print('3');
	LedMat.setTextColor("#ff8000");
	LedMat.print('2');
	LedMat.setTextColor("#ffff00");
	LedMat.print('x');
	LedMat.setTextColor("#80ff00");
	LedMat.print('3');
	LedMat.setTextColor("#00ff00");
	LedMat.print('2');

	LedMat.setCursor(1,9);
	LedMat.setTextColor("#00ffff");
	LedMat.print('*');
	LedMat.setTextColor("#0080ff");
	LedMat.print('R');
	LedMat.setTextColor("#0000ff");
	LedMat.print('G');
	LedMat.setTextColor("#8000ff");
	LedMat.print('B');
	LedMat.setTextColor("#ff00ff");
	LedMat.print('*');
	LedMat.display();
}

void setup(void)
{
  Serial.begin(115200);
  delay(100);
  Serial.println("\n\nReset:");
	
  Serial.println("Initialize");
	LedMat.setImgBuf(imgBuf, sizeof(imgBuf));
  // LedMat.begin(LMMT32x16s8);
  LedMat.begin(LMMT32x32s16);
  // LedMat.begin(LMMT64x32s16);

	scroll.setScrollArea();
	scroll.setYPos();
	scroll.setSpeed(32.0f);
	scroll.setTextColor("blue");
}

void loop(void){
	LedMat.checkSubcon();
	
	scroll.scrollOut();
	while(scroll.update())
		delay(0);

	LedMat.setBright(20);
	scroll.print("Hello World!");
	scroll.scrollOut();
	while(scroll.update())
		delay(0);
	delay(1000);
	
	LedMat.clear();
	LedMat.display();
	LedMat.setBright(20);
	demo1();
	delay(2000);
	
	LedMat.clear();
	LedMat.display();
	LedMat.setBright(5);
	for(int i=0; i<200;i++){
		plasma();
		delay(10);
	}
}


/*** Local variables: ***/
/*** tab-width:2      ***/
/*** truncate-lines:t ***/
/*** End:             ***/
