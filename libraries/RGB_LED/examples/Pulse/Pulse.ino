#include <RGB_LED.h>

RGB_LED rgb;
uint8_t color[3] = {255,255,0};

void setup(){
    rgb.bounceColor(color, 1000);
}

void loop(){
    rgb.render();
}
