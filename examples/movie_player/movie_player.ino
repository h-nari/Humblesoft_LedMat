#include <SD.h>

#include <Adafruit_GFX.h>				// https://github.com/adafruit/Adafruit-GFX-Library
#include <Fontx.h>							// https://github.com/h-nari/Fontx
#include <Humblesoft_GFX.h>			// https://github.com/h-nari/Humblesoft_GFX
#include <Humblesoft_LedMat.h>	// https://github.com/h-nari/Humblesoft_LedMat
#include <MoviePlayer.h>			  // included in Humblesoft_LedMat 

uint8_t imgBuf[1024*8];
MoviePlayer moviePlayer;

void error_printf(const char *fmt, ...)
{
	va_list ap;
	char buf[80];

	va_start(ap, fmt);
	vsnprintf(buf,sizeof buf,fmt, ap);
	va_end(ap);
	Serial.print("Error:");
	Serial.println(buf);
}

void setup(void)
{
  Serial.begin(115200);
  delay(100);
  Serial.println("\n\nReset:");
	
  Serial.println("Initialize");
  LedMat.begin(LMMT64x32s16,1,2);
	LedMat.setImgBuf(imgBuf, sizeof(imgBuf));
	LedMat.setBright(10);
	
	LedMat.setTextWrap(true);
	LedMat.clear();
	LedMat.println("64x32");
	LedMat.setTextColor("red");
	LedMat.print("R");
	LedMat.setTextColor("green");
	LedMat.print("G");
	LedMat.setTextColor("blue");
	LedMat.println("B");
	LedMat.display();
}

int  cnt;
File dir;

void set_next_file()
{
	if(!dir){
		dir = SD.open("/");
		if(!dir)
			LedMat.error("open SD failed");
		dir.rewindDirectory();
		cnt = 0;
	}
	else {
		File f = dir.openNextFile();
		if(f){
			Serial.printf("file:%s\n",f.name());
			String filename = f.name();
			if(filename.endsWith(".HLM")){
				if(moviePlayer.begin(f.name()))
					cnt++;
			}
			f.close();
		}
		else {
			dir.close();
			if(!cnt)
				LedMat.error("no movie file found.");
		}
	}
}

void loop(void){
	LedMat.checkSubcon();		/* 電源電圧降下によるサブCPUリセットの監視と再設定 */
	if(!moviePlayer.update())
		set_next_file();
}


/*** Local variables: ***/
/*** tab-width:2      ***/
/*** truncate-lines:t ***/
/*** End:             ***/
