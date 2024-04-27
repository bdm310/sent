
// Connect the data pin to any PWM pin
const auto pinSent = 5;

#include <SentTimerGPT.h>

SentTimerGPT sent(3, true, pinSent);

void callback(SentFrame &frame) {
  char buf[15];
  if (Serial.availableForWrite() > sizeof(buf)) {
    snprintf(buf, sizeof(buf)
             , "%X%X%X%X%X%X%X%X\n"
             , frame[0]
             , frame[1]
             , frame[2]
             , frame[3]
             , frame[4]
             , frame[5]
             , frame[6]
             , frame[7]
            );
    Serial.print(buf);
  }
}

void setup() {
  Serial.begin(115200);
  while (!Serial) {}
  Serial.println("Starting...");
  sent.begin(&callback);
}

void loop() {
  sent.update();
}
