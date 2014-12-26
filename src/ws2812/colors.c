#include "ws2812/colors.h"
#include <stdint.h>

RGB_T convertHSV2RGB(const HSV_T* hsv){
    RGB_T rgb;

    //Winkel im Farbkeis 0 - 360 in 1 Grad Schritten
    //h = (englisch hue) Farbwert
    //1 Grad Schrittweite, 4.25 Steigung pro Schritt bei 60 Grad
    if(hsv->h<61){
        rgb.red = 255;
        rgb.blue = 0;
        rgb.green = (425 * hsv->h) / 100;
    }else if(hsv->h < 121){
        rgb.green = 255;
        rgb.blue = 0;
        rgb.red = 255 - ((425 * (hsv->h-60))/100);
    }else if(hsv->h < 181){
        rgb.red = 0;
        rgb.green = 255;
        rgb.blue = (425 * (hsv->h-120))/100;
    }else if(hsv->h < 241){
        rgb.red = 0;
        rgb.blue = 255;
        rgb.green = 255 - ((425 * (hsv->h-180))/100);
    }else if(hsv->h < 301){
        rgb.green = 0;
        rgb.blue = 255;
        rgb.red = (425 * (hsv->h-240))/100;
    }else if(hsv->h< 360){
        rgb.red = 255;
        rgb.green = 0;
        rgb.blue = 255 - ((425 * (hsv->h-300))/100);
    }

    //Berechnung der Farbsättigung
    //s = (englisch saturation) Farbsättigung
    uint8_t s = 100 - hsv->s; //Kehrwert berechnen
    uint8_t diff = ((255 - rgb.red) * s)/100;
    rgb.red = rgb.red + diff;
    diff = ((255 - rgb.green) * s)/100;
    rgb.green = rgb.green + diff;
    diff = ((255 - rgb.blue) * s)/100;
    rgb.blue = rgb.blue + diff;

    //Berechnung der Dunkelstufe
    //v = (englisch value) Wert Dunkelstufe einfacher Dreisatz 0..100%
    rgb.red = (rgb.red * hsv->v)/100;
    rgb.green = (rgb.green * hsv->v)/100;
    rgb.blue = (rgb.blue * hsv->v)/100;

    return rgb;
}
