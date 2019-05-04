/**
 * Program to turn led on via push button which should turn off after given time.
 *
 * @author Igor Martens
 * @since 04.05.2019
 */

#define F_CPU 128000UL

#define MILLI_SECONDS_PER_TICK 2048UL
#define DISABLE_AFTER_MILLISECONDS_LEFT 5400000UL

#include <avr/io.h>
#include <avr/interrupt.h>

bool isPushed = false;
bool isReleased = true;

bool isActive = false; // Are LEDs active
volatile uint32_t milliSecondsLeft = 0; // How much milliseconds is left since last reset

ISR(TIM0_OVF_vect) {
    if (isActive) {
        milliSecondsLeft += MILLI_SECONDS_PER_TICK;
    }
}

void setup() {
    DDRB |= (1 << DDB0);  // LED on / off port
    DDRB &= ~(1 << DDB1);  // Input port for button
    PORTB |= (1 << PB1); // Define pull-up and set high

    TCCR0B |= (1 << CS02)|(1 << CS00); // set prescaler to 1024 (CLK=128000Hz/1024/256) interrupt every 2,048s = 2018 milliseconds
    TIMSK0 |= (1 << TOIE0); // enable Timer Overflow interrupt

    sei(); // Enable global interrupts
}

void activateLEDs() {
    isActive = true;
    milliSecondsLeft = 0;
    PORTB |= (1 << PB0);
}

void deactivateLEDs() {
    isActive = false;
    PORTB &= ~(1 << PB0);
}

void updateButtonStatus() {
    if (!isPushed) {
        if ((PINB & (1<<PINB1)) == 0) {
            isPushed = true;
        }

    } else if (!isReleased) {
        if ((PINB & (1<<PINB1)) > 0) {
            isReleased = true;
        }
    }
}

bool isButtonClicked() {
    if (isPushed && isReleased) {
        isPushed = false;
        isReleased = false;
        return true;
    }

    return false;
}

int main(void) {
    setup();

    while(1) {
        updateButtonStatus();
        if (isButtonClicked()) {
          if (isActive) {
              deactivateLEDs();
          } else {
              activateLEDs();
          }
        } else {
            if (isActive && milliSecondsLeft > DISABLE_AFTER_MILLISECONDS_LEFT) {
                deactivateLEDs();
            }
        }
    }

    return 0;
}
