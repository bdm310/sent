#pragma once

#include "FspTimer.h"
#include "BaseSent.h"

#include <variant.h>


static const auto CHANNEL_COUNT = 14;
static R_GPT0_Type * channel2GPT[CHANNEL_COUNT] = {
    R_GPT0, R_GPT1, R_GPT2, R_GPT3, R_GPT4, 
    R_GPT5, R_GPT6, R_GPT7, R_GPT8, R_GPT9,
    R_GPT10, R_GPT11, R_GPT12, R_GPT13
};

void isr_GPT(timer_callback_args_t *p_args);

class SentTimerGPT:public BaseSent{
    public:
        SentTimerGPT(uint8_t tick_time = 3, bool padding = false, unsigned int pin = 4)
            : BaseSent(tick_time, padding, sentBuffer)
            , _pin{pin}
        {}

        void begin(SentCallback callback = nullptr) override {
            bool ok = true;

            ok = ok && _pin < PINS_COUNT;

            if(!ok){
                return;
            }

            auto cfg = getPinCfgs(_pin, PIN_CFG_REQ_PWM);
            ok = ok && IS_PIN_GPT_PWM(cfg[0]);

            auto channel = GET_CHANNEL(cfg[0]);
            ok = ok && channel < CHANNEL_COUNT;
        
            auto hasChannelA = IS_PWM_ON_A(cfg[0]);
            auto hasChannelB = IS_PWM_ON_B(cfg[0]);

            ok = ok && (hasChannelA || hasChannelB) && (hasChannelA != hasChannelB);

            if(!ok){
                return;
            }

            timer.force_use_of_pwm_reserved_timer();
            ok = ok && timer.begin(TIMER_MODE_PERIODIC, GPT_TIMER, channel, 0x10000, 0, TIMER_SOURCE_DIV_4, isr_GPT, this);

            if(hasChannelA){
                ok = ok && timer.setup_capture_a_irq();
            } else {
                ok = ok && timer.setup_capture_b_irq();
            }
            ok = ok && timer.open();

            if(!ok){
                return;
            }

            auto rpin = g_pin_cfg[_pin].pin;
            uint32_t pcfg = IOPORT_CFG_PORT_DIRECTION_INPUT | IOPORT_CFG_PULLUP_ENABLE | IOPORT_CFG_PERIPHERAL_PIN | IOPORT_PERIPHERAL_GPT1;
            R_IOPORT_PinCfg(NULL, rpin, pcfg);

            auto gpt = channel2GPT[channel];
            if(IS_PWM_ON_A(cfg[0])){
                gpt->GTICASR_b.ASCAFBL = 0b1;
                gpt->GTICASR_b.ASCAFBH = 0b1;
            } else {
                gpt->GTICBSR_b.BSCBFAL = 0b1;
                gpt->GTICBSR_b.BSCBRAH = 0b1;
            }

            updateLut(timer.get_freq_hz());

            BaseSent::begin(callback);
            timer.start();
        }

        void doISR(timer_callback_args_t *p_args){
            if(p_args->event == TIMER_EVENT_CAPTURE_A || p_args->event == TIMER_EVENT_CAPTURE_B){
                sentBuffer.write(p_args->capture);
            }
        }

    private:
        FspTimer timer{};
        SentBuffer sentBuffer;
        unsigned int _pin;
};

void isr_GPT(timer_callback_args_t *p_args) {
    SentTimerGPT *sentTimer = const_cast<SentTimerGPT*>(static_cast<const SentTimerGPT*>(p_args->p_context));
    if(sentTimer){
        sentTimer->doISR(p_args);
    }
}
