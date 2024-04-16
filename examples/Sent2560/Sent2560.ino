
// Connect data pin of sensor to D49 of the Arduino Mega 2560

#include <SentTimer4.h>
#include <MLX90818.h>

SentTimer4 sent(3, true);
MLX90818 mlx(sent);

#define USE_MLX

void callback(SentFrame &frame){
    if(Serial.availableForWrite() > 10){
        for(int c = 0; c < 8; c++){
            Serial.print(frame[c], HEX);
        }
        Serial.println();
    }
}

void mlx_cb(float pressure){
  uint16_t tmp = pressure * 1000;
  uint16_t v = tmp / 1000;
  uint16_t f = tmp % 1000;

  char buffer[10];
  snprintf(buffer, sizeof(buffer), "%d.%03d ", v,f);
  Serial.println(buffer);
}

void setup(){
    Serial.begin(115200);

#ifndef USE_MLX
    sent.begin(&callback);
#else
    sent.begin();
    mlx.begin(&mlx_cb);
#endif
}

void loop(){
    sent.update();
}
