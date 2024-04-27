#pragma once

#include "sent/BaseSent.h"

enum class SlowType{
    Short,
    Enhanced12,
    Enhanced16
};

typedef void(*SlowChannelCallback)(SlowType, uint8_t, uint16_t);

class SlowChannel:ISentFrameHandler{
    public:
        SlowChannel(BaseSent &sent)
            : _sent(&sent)
            , _callback(nullptr)
        { }

        void begin(SlowChannelCallback callback){
            _callback = callback;
            _sent->registerHandler(this);
        }

        void onFrame(BaseSent *sender, SentFrame &frame) override {
            (void) sender;
            
            data2 = data2 << 1 | bitRead(frame[0], 2);
            data3 = data3 << 1 | bitRead(frame[0], 3);

            uint8_t id;
            uint16_t data;
            if((data3 & 0b1111111111111111) == 0b1000000000000000){
                id = (data2 >> 12) & 0b1111;
                data = (data2 >> 4) & 0b11111111;
                (*_callback)(SlowType::Short, id, data);
            } else if((data3 & 0b111111110000100001) == 0b111111000000000000){
                id = ((data3 >> 2) & 0b11110000) | ((data3 >> 1) & 0b1111);
                data = data2 & 0b111111111111;
                (*_callback)(SlowType::Enhanced12, id, data);
            } else if((data3 & 0b111111110000100001) == 0b111111010000000000){
                id = (data3 >> 6) & 0b1111;
                data = ((data3 << 11) & 0b1111000000000000) | (data2 & 0b111111111111);
                (*_callback)(SlowType::Enhanced16, id, data);
            }
        }

        void onError(BaseSent*, SentError) override {
            data3 = 0;
        }
        
    private:
        BaseSent *_sent;
        SlowChannelCallback _callback;

        uint32_t data2;
        uint32_t data3;
};
