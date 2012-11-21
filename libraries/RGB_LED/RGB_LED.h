/* Copyright (C) 2012 by Victor Aprea <victor.aprea@wickeddevice.com>

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.  */

#ifndef _RGB_LED_LIB_H
#define _RGB_LED_LIB_H

#if ARDUINO >= 100
  #include <Arduino.h>  // Arduino 1.0
#else
  #include <WProgram.h> // Arduino 0022
#endif

#include <stdint.h>


class RGB_LED {
 private:
    uint8_t color_start[3];    // start color
    uint8_t color_end[3];      // end color
    int16_t color_step[3];     // how much to step in each channel for the current animation
    uint8_t rgb_pins[3];       // the physically mapped pin numbers for R, G, and B
    int32_t animation_start;   // timestamp of animation start
    int32_t animation_period;  // period of animation
    boolean is_bouncy;         // whether the bouncy behavior is called to be executed
    uint16_t bounce_counter;   // number of bounces so far
    uint8_t bounce_limit;      // number of times to bounce (0 means forever)
 public:
    uint16_t bright[3];         // normalization factors
    RGB_LED();
    RGB_LED(uint16_t max_luminosity[3]);
    void init(uint16_t max_luminosity[3]);
    void setColor(uint8_t color[3]);
    void setDutyCycle(uint8_t duty_cycle[3]);
    void fade(uint8_t from_color[3], uint8_t to_color[3], int32_t period_ms);
    void bounceColor(uint8_t color[3], int32_t period_ms);
    void bounceColorN(uint8_t color[3], int32_t period_ms, uint8_t n);
    void setPins(uint8_t r, uint8_t g, uint8_t b);
    void render();
    boolean animation_complete();
    void stop_animation();
};

#endif /*_EGG_BUS_LIB_H */
