#pragma once

#include "sent/AVR.h"

#include <Arduino.h>

SentBuffer sentBufferT4;

ISR(TIMER4_CAPT_vect) {
  uint16_t value = ICR4;
  sentBufferT4.write(value);
}

class SentTimer4:public AvrSent{
    public:
        SentTimer4(uint8_t tick_time = 3, bool padding = false)
            : AvrSent(tick_time, padding, sentBufferT4)
        {}

        void initTimer(uint8_t cs1x) override {
            TCCR4A = 0;
            TCCR4B = cs1x;
            TCCR4C = 0;
            TIMSK4 = (1 << ICIE4);
        }
};
