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

#include "RGB_LED.h"

#define DEFAULT_R_PIN 5
#define DEFAULT_G_PIN 3
#define DEFAULT_B_PIN 6

RGB_LED::RGB_LED(){
    uint16_t max_luminosity[3] = {800, 1200, 500};
    init(max_luminosity);
}

RGB_LED::RGB_LED(uint16_t max_luminosity[3]){
    init(max_luminosity);
}

void RGB_LED::init(uint16_t max_luminosity[3]){
    uint16_t min_overall = min(max_luminosity[0], max_luminosity[1]);
    min_overall = min(min_overall, max_luminosity[2]);
    for(uint8_t ii = 0; ii < 3; ii++){
        bright[ii] = (uint16_t) ((((1.0 * min_overall) / max_luminosity[ii]) * 256.0) + 0.5);
    }        

    setPins(DEFAULT_R_PIN, DEFAULT_G_PIN, DEFAULT_B_PIN);
}

// convert the requested color to a duty cycle by normalizing to the brightness
void RGB_LED::setColor(uint8_t color[3]){
    uint8_t duty_cycle[3];
    for(uint8_t ii = 0; ii < 3; ii++){
        uint16_t d_cycle = map(color[ii], 0, 255, 0, bright[ii]);
        if(d_cycle > 255){
            d_cycle = 255;
        }
        duty_cycle[ii] = (uint8_t) d_cycle;
    }
    setDutyCycle(duty_cycle);
}

void RGB_LED::setDutyCycle(uint8_t duty_cycle[3]){
    for(uint8_t ii = 0; ii < 3; ii++){
        analogWrite(rgb_pins[ii], duty_cycle[ii]);
    }
}

// set up the fade animation
void RGB_LED::fade(uint8_t from_color[3], uint8_t to_color[3], int32_t period_ms){
    setColor(from_color);
    for(uint8_t ii = 0; ii < 3; ii++){
        color_start[ii] = from_color[ii];
        color_end[ii] = to_color[ii];
        color_step[ii] = ((int16_t) color_end[ii]) - ((int16_t) color_start[ii]);
    }    
    is_bouncy = false;
    bounce_limit = 0;
    animation_start = millis();
    animation_period = period_ms;
}

void RGB_LED::bounceColor(uint8_t color[3], int32_t period_ms){
    uint8_t black[3] = {0,0,0};
    setColor(black);
    for(uint8_t ii = 0; ii < 3; ii++){
        color_start[ii] = 0;
        color_end[ii] = color[ii];
        color_step[ii] = ((int16_t) color_end[ii]) - ((int16_t) color_start[ii]);
    }    
    is_bouncy = true;
    bounce_limit = 0;
    animation_start = millis();
    animation_period = period_ms / 2;
}

void RGB_LED::bounceColorN(uint8_t color[3], int32_t period_ms, uint8_t n){
    bounceColor(color, period_ms);
    bounce_limit = n;
    bounce_counter = 0;
}

void RGB_LED::setPins(uint8_t r, uint8_t g, uint8_t b){
    rgb_pins[0] = r;
    rgb_pins[1] = g;
    rgb_pins[2] = b;
    
    for(uint8_t ii = 0; ii < 3; ii++){
        pinMode(rgb_pins[ii], OUTPUT);
        analogWrite(rgb_pins[ii], 0);
    }
}

// this function must be called in the loop with adequate frequency to
// achieve the required level of animation smoothness
void RGB_LED::render(){
    uint8_t black[3] = {0,0,0};
    uint32_t now = millis();
    uint32_t elapsed_time = now - animation_start;
    uint16_t percent_elapsed = (uint16_t) ((100.0 * elapsed_time) / animation_period);
    if(percent_elapsed > 100) percent_elapsed = 100;
    uint8_t color[3];
    for(uint8_t ii = 0; ii < 3; ii++){
        color[ii] = color_start[ii] + map(percent_elapsed, 0, 100, 0, color_step[ii]);               
    }   

    setColor(color);

    // calculate whether the end color has been reached
    boolean end_color_reached = true;
    for(uint8_t ii = 0; ii < 3; ii++){
        if(color_end[ii] != color[ii]){
            end_color_reached = false;
        }
    }

    // for bouncy behavior set up the next animation
    if(is_bouncy){ 
        if(end_color_reached){                
            // switch the start and end colors
            if(bounce_limit != 0){
                bounce_counter++;
                if(bounce_counter / 2 == bounce_limit){
                    setColor(black);
                    stop_animation();
                    return;                
                }
            }           

            for(uint8_t ii = 0; ii < 3; ii++){                
                color[ii] = color_start[ii];
                color_start[ii] = color_end[ii];
                color_end[ii] = color[ii];
                color_step[ii] = ((int16_t) color_end[ii]) - ((int16_t) color_start[ii]);
                animation_start = millis();
            }            
        }
    }            
}

boolean RGB_LED::animation_complete(){
    if(millis() - animation_start > animation_period){
        return true;
    }
    return false;
}

void RGB_LED::stop_animation(){
    for(uint8_t ii = 0; ii < 3; ii++){
        color_step[ii]   = 0;
        color_start[ii]  = 0; // lights out
        color_end[ii]    = 0; // lights out
    }
    is_bouncy = false;
    animation_start = 0;   
}

