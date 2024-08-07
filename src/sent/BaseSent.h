#pragma once

#include <Arduino.h>
#include "crc4.hpp"
#include "config.h"
#include "Buffer.h"

using SentFrame = uint8_t[8];
typedef void(*SentCallback)(SentFrame&);

enum class SentError:uint8_t{
    SyncError,
    CrcError,
    NibbleError,
    OverflowError,
    ConfigurationError
};

class BaseSent;

class ISentFrameHandler{
    public:
        virtual void onFrame(BaseSent*, SentFrame&) = 0;
        virtual void onError(BaseSent*, SentError) {}
};

class BaseSent{
    public:
        BaseSent(uint8_t tick_time, bool padding, SentBuffer &buffer)
            : _state(State::sync)
            , _lastValue(0)
            , _buffer(buffer)
            , _frameIdx(0)
            , _padding(padding)
            , _tick_time(tick_time)
        {
            for(int x = 0; x < MAX_SENT_HANDLER; x++){
                _handler[x] = nullptr;
            }
        }

        bool registerHandler(ISentFrameHandler *handler){
            for (int x = 0; x < MAX_SENT_HANDLER; x++){
                if(_handler[x] == nullptr){
                    _handler[x] = handler;
                    return true;
                }
            }
            return false;
        }

        virtual void begin(SentCallback callback = nullptr){
            _callback = callback;
        }

        void update(){
            while(_buffer.available()){
                if (_buffer.isError()){
                    onError(SentError::OverflowError);
                    _buffer.resetError();
                    _lastValue = _buffer.read();
                    _state = State::sync;
                    continue;
                }

                uint16_t tmp = _buffer.read();
                uint16_t dx = tmp - _lastValue;
                _lastValue = tmp;

                switch(_state){
                    case State::sync:
                        if((_cycl_syn_min <= dx) && (dx <= _cycl_syn_max)){
                            _state = State::data;
                            _frameIdx = 0;
                            _crc.start();
                        } else {
                            onError(SentError::SyncError);
                        }
                        break;
                    case State::data:
                        dx = (dx - _cycl_offset) >> _lookup_shift;
                        if (dx > LOOKUP_SIZE) {
                            onError(SentError::NibbleError);
                            _state = State::sync;
                        } else {
                            auto nibble = _cycl_lookup[dx];
                            if(nibble < 0){
                                onError(SentError::NibbleError);
                                _state = State::sync;
                                break;
                            }
                            _frame[_frameIdx] = nibble;
                            if(_frameIdx>0 && _frameIdx<7){
                                _crc.update4(nibble);
                            }
                            if(_frameIdx == 7){
                                auto crcOk = _crc.finish() == nibble;
                                if(crcOk) {
                                    onFrame();
                                } else {
                                    onError(SentError::CrcError);
                                    _state = State::sync;
                                }
                            }
                            _frameIdx++;
                            if(_frameIdx >= 8){
                                if(_padding){
                                    _state = State::padding;
                                } else {
                                    _state = State::sync;
                                }
                            }
                        }
                        break;
                    case State::padding:
                        _state = State::sync;
                        break;
                }
            }
        }

        void onError(SentError error){
            for(int x = 0; x < MAX_SENT_HANDLER; x++){
                if(_handler[x]){
                    _handler[x]->onError(this, error);
                }
            }
        }

        void onFrame(){
            for(int x = 0; x < MAX_SENT_HANDLER; x++){
                if(_handler[x] != nullptr){
                    _handler[x]->onFrame(this, _frame);
                }
            }
            if(_callback){
                (*_callback)(_frame);
            }
        }

    protected:
        void updateLut(uint64_t f_timer){
            _cycl_tick = f_timer / 1000000 * _tick_time; // CPU cycles per tick

            // Allow +/- 20% difference
            uint16_t _cycl_syn = _cycl_tick * _tick_syn;
            _cycl_syn_min = _cycl_syn * 4UL / 5;
            _cycl_syn_max = _cycl_syn * 6UL / 5;

            _cycl_offset = _cycl_tick * (_tick_offest * 2UL - 1) / 2;

            uint16_t lookupDiv = 16 * _cycl_tick / LOOKUP_SIZE;
            for(_lookup_shift=0; _lookup_shift<16; _lookup_shift++){
                if (lookupDiv < (1U <<_lookup_shift))
                    break;
            }
            if(_lookup_shift >= 16){
                onError(SentError::ConfigurationError);
                return;
            }

            for (uint32_t c = 0; c < LOOKUP_SIZE; c++) {
                int8_t value = c * (1 << _lookup_shift) / _cycl_tick;
                if ( value >= 16) {
                    value = -1;
                }
                _cycl_lookup[c] = value;
            }
        }

    private:
        enum class State{
            sync,
            data,
            padding
        };
        State _state;

        int8_t _cycl_lookup[LOOKUP_SIZE];
        uint8_t _lookup_shift;

        uint16_t _lastValue;
        SentBuffer& _buffer;
        uint8_t _frameIdx;
        bool _padding;
        SentFrame _frame;
        SentCallback _callback;
        Crc4Sent _crc;
        
        uint16_t _cycl_tick;
        uint16_t _cycl_syn_min;
        uint16_t _cycl_syn_max;
        uint16_t _cycl_offset;

    protected:
        const uint8_t _tick_syn{56};
        const uint8_t _tick_offest{12};

        uint8_t _tick_time;
        ISentFrameHandler* _handler[MAX_SENT_HANDLER];
};
