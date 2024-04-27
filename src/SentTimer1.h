#pragma once

#include "sent/AVR.h"

#include <Arduino.h>

SentBuffer sentBufferT1;

ISR(TIMER1_CAPT_vect) {
  uint16_t value = ICR1;
  sentBufferT1.write(value);
}

class SentTimer1:public AvrSent{
    public:
        SentTimer1(uint8_t tick_time = 3, bool padding = false)
            : AvrSent(tick_time, padding, sentBufferT1)
        {}

        void initTimer(uint8_t cs1x) override {
            TCCR1A = 0;
            TCCR1B = cs1x;
            TCCR1C = 0;
            TIMSK1 = (1 << ICIE1);
        }
};
