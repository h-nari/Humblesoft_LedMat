#include <SD.h>

#include <Adafruit_GFX.h>       // https://github.com/adafruit/Adafruit-GFX-Library
#include <Fontx.h>              // https://github.com/h-nari/Fontx
#include <Humblesoft_GFX.h>     // https://github.com/h-nari/Humblesoft_GFX
#include <Humblesoft_LedMat.h>  // https://github.com/h-nari/Humblesoft_LedMat
#include <MoviePlayer.h>        // included in Humblesoft_LedMat 

uint8_t imgBuf[1024*24];
MoviePlayer moviePlayer;

void setup(void)
{
  Serial.begin(115200);
  delay(100);
  Serial.println("\n\nReset:");
  
  Serial.println("Initialize");
  LedMat.begin(LMMT64x32s16,1,1);
  LedMat.setImgBuf(imgBuf, sizeof(imgBuf));
  LedMat.setBright(10);    
  LedMat.setPlane(8);
  
  LedMat.setTextWrap(true);
  LedMat.clear();
  LedMat.printf("%dx%d\n",LedMat.width(),LedMat.height());
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

void loop(void){
  LedMat.checkSubcon();

  if(!moviePlayer.update()){
    LedMat.clear();
    LedMat.display();
    delay(1000);
    if(!moviePlayer.begin("/movie.hlm"))
      LedMat.printf("Error");
  }
}


/*** Local variables: ***/
/*** tab-width:2      ***/
/*** truncate-lines:t ***/
/*** End:             ***/
