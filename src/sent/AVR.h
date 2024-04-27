#pragma once

#include "BaseSent.h"

class AvrSent: public BaseSent{
    public:
        using BaseSent::BaseSent;

        virtual void initTimer(uint8_t divider) = 0;

        void begin(SentCallback callback = nullptr) override {
            BaseSent::begin(callback);

            // Tick_time can be between 3µs and 90µs 
            // Figure out what divider we need for the 
            // 16 bit timer to allow for at least 4 syncs
            uint32_t cycles = F_CPU / 1000000 * _tick_time * _tick_syn * 4;
            uint32_t div = cycles / (UINT16_MAX + 1UL);

            const auto numDivider = 6;
            uint16_t divider[] = {0, 1, 8, 64, 256, 1024};
            int cs1x;
            for(cs1x=1; cs1x<numDivider; cs1x++){
                if(div < divider[cs1x])
                    break;
            }

            if(cs1x >= numDivider){
                onError(SentError::ConfigurationError);
                return;
            }

            auto F_TIMER = F_CPU / divider[cs1x];

            if((F_TIMER / 1000000 * _tick_time) < 4){
                // For this F_CPU and tick_time the divider would not allow
                // for accurate enough measurements with the timer...
                onError(SentError::ConfigurationError);
                return;
            }

            updateLut(F_TIMER);
            initTimer(cs1x);
        }
};
