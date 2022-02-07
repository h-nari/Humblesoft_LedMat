#include <Arduino.h>
#include <Humblesoft_LedMat.h>

#define PERIOD 2000  // [ms]

static unsigned long tPrev;

void light_sensor_init(void) { tPrev = millis(); }

void light_sensor_update(void) {
  if (millis() - tPrev > PERIOD) {
    int b = 10;
    tPrev += PERIOD;
    int val = analogRead(A0);
    if (val < 5)
      b = 1;
    else if (val < 20)
      b = 3;
    else if (val < 50)
      b = 5;
    else
      b = 10;
    LedMat.setBright(b);

    // Serial.printf("TOUT: %4d  bright:%d\n", val, b);
  }
}