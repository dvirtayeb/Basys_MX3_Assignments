#include <xc.h>
#pragma config JTAGEN = OFF
#pragma config FWDTEN = OFF
#pragma config FNOSC =FRCPLL
#pragma config FSOSCEN = OFF
#pragma config POSCMOD = EC
#pragma config OSCIOFNC = ON
#pragma config FPBDIV = DIV_1
#pragma config FPLLIDIV = DIV_2
#pragma config FPLLMUL = MUL_20
#pragma config FPLLODIV = DIV_1
// define times
#define TIME_INTERVAL 640000

#define QUICK_TIME_INTERVAL 100000
// define Switches
#define SW0 PORTFbits.RF3 // hexadecimal counter in the LEDs
#define SW1 PORTFbits.RF5 // shift 2 in the LEDs
#define SW2 PORTFbits.RF4 // a fan in the LEDs
#define SW3 PORTDbits.RD15 // reverse the LEDs
#define SW4 PORTDbits.RD14 // speed change in the LEDs
#define SW5 PORTBbits.RB11 // stopping the LEDs
#define SW6 PORTBbits.RB10 // buzzing noise
#define SW7 PORTBbits.RB9 // exiting the program

#define BUZZER_INTERVAL 200
void TimeDelay();
void activateBuzzer();
void count() {

    if (SW0 && !SW3) {
        for (int i = 0; i < 256 && SW0 && !SW1 && !SW2 && !SW3; i++) {
            PORTA = i; //PORTA shows the decimal number as binary
            TimeDelay();
            if (SW7 && SW1 && SW2) // Terminate
            {
                PORTA = 0;
                return;
            }
        }
    } else if (SW0 && SW3) {
        for (int i = 256; i > 0 && SW0 && !SW1 && !SW2 && SW3; i--) {
            PORTA = i; //PORTA shows the decimal number as binary
            TimeDelay();
            if (SW7 && SW1 && SW2) // Terminate
            {
                PORTA = 0;
                return;
            }
        }
    } else {
        return;
    }
}

void justMoveYoni(int currentState, int counter) {
    if (SW1 && !SW3) { // move front
        currentState = 1;
        counter = 1;
        int i;

        for (i = 0; i < 9 && SW1 && !SW3; i++) {
            PORTA = counter;
            counter *= 2;
            TimeDelay();
            if (SW7 && SW2) // Terminate
            {
                PORTA = 0;
                return;
            }
        }
    } else if (SW1 && SW3) { // move back
        currentState = 1;
        counter = 256;
        int i;

        for (i = 9; i > 0 && SW1 && SW3; i--) {
            PORTA = counter;
            counter /= 2;
            TimeDelay();
            if (SW7 && SW2) // Terminate
            {
                PORTA = 0;
                return;
            }
        }
    } else {
        return;
    }
}

void TimeDelay() {
    while (SW5) {
        if (SW7) // Terminate
        {
            PORTA = 0;
            return;
        }
        continue;
    }
    if (SW4) {
        for (int j = 0; j < TIME_INTERVAL; j++) {
        }
    } else {
        for (int j = 0; j < QUICK_TIME_INTERVAL; j++) {
        }
    }
}

void menifa(int counter, int currentState) {

    currentState = 2;
    if (SW2 && !SW3) {
        counter = 16;
        int i;
        int j = 8;
        for (i = 0; i < 9 && SW2; i++) { // i = left, j = right

            PORTA = counter + j;
            counter *= 2;
            j /= 2;
            TimeDelay();
            if (SW7) // Terminate
            {
                PORTA = 0;
                return;
            }
        }
    } else if (SW2 && SW3) {
        counter = 1;
        int i;
        int j = 128;
        for (i = 0; i < 5 && SW2; i++) { // counter = right, j = left
            PORTA = counter + j;
            counter *= 2;
            j /= 2;
            TimeDelay();
            if (SW7) // Terminate
            {
                PORTA = 0;
                return;
            }
        }
    } else {
        return;
    }
}

void playSound() {
    uint32_t currentSpeed = TIME_INTERVAL;
    currentSpeed = SW4 ? QUICK_TIME_INTERVAL : TIME_INTERVAL;
    for (int j = 0; j < currentSpeed; j++){
        if (!(j % BUZZER_INTERVAL) && SW6){
            activateBuzzer();
        }
    }
}

void activateBuzzer() {
    LATBINV = 1 << 14;
}

void main() {
    // init the leds - output
    TRISA &= 0xff00;
    //    // initialize each switch to be input
    TRISFbits.TRISF3 = 1;
    TRISFbits.TRISF5 = 1;
    TRISFbits.TRISF4 = 1;
    TRISDbits.TRISD15 = 1;
    TRISDbits.TRISD14 = 1;
    TRISBbits.TRISB11 = 1;
    TRISBbits.TRISB10 = 1;
    TRISBbits.TRISB9 = 1;
    ANSELBbits.ANSB9 = 0;
    ANSELBbits.ANSB11 = 0;
    ANSELBbits.ANSB10 = 0;
    // Sound:
    TRISBbits.TRISB14 = 0;
    ANSELBbits.ANSB14 = 0;
    //    RPB14R = 0x0C;


    // State
    int currentState = -1;
    while (1) {
        // priority
        if (SW2)
            currentState = 2;
        else if ((!SW2) && SW1)
            currentState = 1;
        else
            currentState = 0;
        int counter = 1;

        switch (currentState) {
            case 0:
                count();
                break;

            case 1:
                justMoveYoni(currentState, counter);
                break;

            case 2:
                menifa(counter, currentState);
                break;

            default:
                continue;
        }
        if (SW7) {
            PORTA = 0;
            return;
        }
        playSound();
    }
}