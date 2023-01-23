//
// vim: ts=4 et
// Copyright (c) 2023 Petr Vanek, petr@fotoventus.cz
//
/// @file   beep.h
/// @author Petr Vanek

#pragma once

#include <inttypes.h>
#include "hardware/clocks.h"
#include <hardware/pwm.h>
#include "pico/stdlib.h"

/**
 * @brief pwm-based sound generation 
 *
 */
class Beep
{

public:

    /**
     * @brief Construct a new Beep 
     * 
     * @param pin - pin assigment
     */
    Beep(uint8_t pin) : _pin(pin)
    {
    }

    /**
     * @brief pwm initialization for a given pin
     * 
     * @return true 
     * @return false 
     */
    bool init()
    {
        _slice = pwm_gpio_to_slice_num(_pin);
        gpio_set_function(_pin, GPIO_FUNC_PWM);
        pwm_set_phase_correct(_slice, false);
        pwm_set_clkdiv_int_frac(_slice, clock_get_hz(clk_sys) / 1000000, 0);
        return true;
    }

    /**
     * @brief tone generation of a given frequency
     * 
     * @param freq in Hz
     */
    void beep(uint16_t freq)
    {
        pwm_set_wrap(_slice, (uint)(1000000 / freq));
        pwm_set_enabled(_slice, true);
        pwm_set_gpio_level(_pin, (uint)((1000000 / freq) / 2));
    }

    /**
     * @brief Disable tone generation
     * 
     */
    void off()
    {
        pwm_set_enabled(_slice, false);
      //  gpio_init(_pin);
      //  gpio_set_dir(_pin, true);
      //  gpio_put(_pin, false);
    }

private:
    uint8_t _pin{0};
    uint _slice{0};
};
