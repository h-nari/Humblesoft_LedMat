#include <Adafruit_GFX.h>				// https://github.com/adafruit/Adafruit-GFX-Library
#include <Humblesoft_GFX.h>			// https://github.com/h-nari/Humblesoft_GFX
#include <Humblesoft_LedMat.h>	// https://github.com/h-nari/Humblesoft_LedMat
#include <ScrollText.h>

ScrollText sc1(&LedMat);
ScrollText sc2(&LedMat);
uint8_t imgBuf[1024*4];

void setup(void)
{
  Serial.begin(115200);
  delay(100);
  Serial.println("\n\nReset:");

  LedMat.begin(LMMT64x32s16);
	LedMat.setLedMode(1);
	LedMat.setImgBuf(imgBuf, sizeof imgBuf);

	int w = LedMat.width();
	int h = LedMat.height();
	
	sc1.setSpeed(16);
	sc1.setScrollArea(16, 0, w - 16 - 2, h/2);
	sc1.setYPos();

	sc2.setSpeed(32);
	sc2.setScrollArea(0, h/2, w, h/2);
	sc2.setYPos();

	LedMat.clear();
	LedMat.display();
}

const char *colors[] =
	{ "WHITE", "RED", "GREEN", "BLUE",
		"YELLOW", "PURPLE", "CYAN", "MAGENTA", "NAVY",
		"DARKGREEN", "DARKCYAN", "MAROON", "OLIVE", "LIGHTGREY",
		"DARKGRAY", "ORANGE", "GREENYELLOW", "PINK",
	};


void loop(void){
	static int idx1,idx2;
	static const char *prev_color2 = "black";
	if(!sc1.update()){
		LedMat.fillRect(0,0,16,16,"black");
		LedMat.setCursor(2,4);
		LedMat.print(idx1);
		
		if(idx1 >= sizeof(colors)/sizeof(colors[0]))
			idx1 = 0;
		sc1.setTextColor(colors[idx1]);
		sc1.print(colors[idx1]);
		sc1.scrollOut();
		idx1++;
	}
	
	if(!sc2.update()){
		if(idx2 >= sizeof(colors)/sizeof(colors[0]))
			idx2 = 0;
		sc2.setTextColor(colors[idx2]);
		sc2.setBgColor(prev_color2);
		sc2.print(' ');
		sc2.print(colors[idx2]);
		sc2.print(' ');
		prev_color2 = colors[idx2];
		idx2++;
	}
}


/*** Local variables: ***/
/*** tab-width:2      ***/
/*** truncate-lines:t ***/
/*** End:             ***/
