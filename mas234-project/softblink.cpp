/*
 * GccApplication1.cpp
 *
 * Created: 16.10.2020 09:31:07
 * Author : Inter
 */

#define F_CPU 1000000

#include <avr/io.h>

#define __DELAY_BACKWARD_COMPATIBLE__
#include <util/delay.h>


void level(int on) {
        int tot = 100; // 100%
        int off = tot - on; // 90 %, on via input argument

        PORTC |= 1 << PC5;
        _delay_us(on*100);
        PORTC &= ~(1 << PC5);
        _delay_us(off*100);

}

int main(void){

        DDRC = 0x00;
        PORTC = 0x00;

        DDRC |= 1 << PC5;

        // Increase and decrease level of light. pulse width modulation: pseudo-pwm  on/off loop.

    while (1) {
                for (int i=0; i < 100; i++) {
                        level(i);
                        _delay_ms(5);
                }
                for (int i=100; i > 0; i--) {
                        level(i);
                        _delay_ms(5);
                }
    }
}
