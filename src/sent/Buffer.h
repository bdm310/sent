#pragma once

#include "config.h"

#include <Arduino.h>

class SentBuffer{
    public:
        SentBuffer()
            : _buffer_write_pointer(0)
            , _buffer_read_pointer(0)
            , _error(false)
        {}

        bool available(){
            return _buffer_write_pointer != _buffer_read_pointer;
        }

        uint16_t read() {
            while (!available()) {
                // wait for data
            }
            uint16_t ret = _buffer[_buffer_read_pointer];
            auto new_p = _buffer_read_pointer + 1;
            if (new_p >= BUFFER_SIZE) {
                new_p = 0;
            }

            _buffer_read_pointer = new_p;

            return ret;
        }

        void write(uint16_t value){
            _buffer[_buffer_write_pointer] = value;
            _buffer_write_pointer++;
            if (_buffer_write_pointer >= BUFFER_SIZE) {
                _buffer_write_pointer = 0;
            }
            if (_buffer_write_pointer == _buffer_read_pointer) {
                _error = true;
            }
        }

        bool isError(){
            return _error;
        }

        void resetError(){
            _error = false;
        }

    private:
        uint16_t _buffer[BUFFER_SIZE];
        uint8_t _buffer_write_pointer;
        uint8_t _buffer_read_pointer;

        bool _error;
};
