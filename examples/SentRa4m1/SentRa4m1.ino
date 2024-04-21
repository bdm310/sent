
// Connect the data pin to any PWM pin
const auto pinSent = 5;

#include <SentTimerGPT.h>

SentTimerGPT sent(3, true, pinSent);

void callback(SentFrame &frame) {
  if (Serial.availableForWrite() > 15) {
    for (int c = 0; c < 8; c++) {
      Serial.print(frame[c], HEX);
    }
    Serial.println();
  }
}

void setup() {
  Serial.begin(115200);
  while (!Serial) {}

  sent.begin(&callback);
}

void loop() {
  sent.update();
}
