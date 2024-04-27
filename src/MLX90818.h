#pragma once

#include "sent/BaseSent.h"

typedef void(*MLX90818Callback)(float);

class MLX90818:ISentFrameHandler{
    public:
        MLX90818(BaseSent &sent)
            : _sent(&sent)
            , _callback(nullptr)
        { }

        void begin(MLX90818Callback callback){
            _callback = callback;
            _sent->registerHandler(this);
        }

        void onFrame(BaseSent *sender, SentFrame &frame) override {
            (void) sender;

            uint16_t value = (frame[1] << 8) | (frame[2] << 4) | frame[3];
            float result = ((value - O1) * (P2 - P1) / (O2 - O1)) + P1;

            if(_callback){
                (*_callback)(result);
            }
        }

    private:
        BaseSent *_sent;
        MLX90818Callback _callback;

        const uint16_t O1 =  193;
        const uint16_t O2 = 3896;
        const float P1 = 0.1;
        const float P2 = 4.0;

};
