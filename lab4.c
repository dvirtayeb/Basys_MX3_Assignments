// Includes
#include <stdio.h>
#include <stdlib.h>
#include <xc.h>
#include <sys/attribs.h>
#include <string.h>
#include <p32xxxx.h>

// Configs
#pragma config JTAGEN = OFF
#pragma config FWDTEN = OFF
#pragma config FNOSC = FRCPLL
#pragma config FSOSCEN = OFF
#pragma config POSCMOD = EC
#pragma config OSCIOFNC = ON
#pragma config FPBDIV = DIV_1
#pragma config FPLLIDIV = DIV_2
#pragma config FPLLMUL = MUL_20
#pragma config FPLLODIV = DIV_1

// Defines
typedef unsigned char BYTE;

#define BUZZER_INTERVAL 200
#define SLOW_TIME_INTERVAL 200000
#define QUICK_TIME_INTERVAL 100000
#define SPEAKER LATBbits.LATB14


// Switches
#define SW0 PORTFbits.RF3
#define SW1 PORTFbits.RF5
#define SW2 PORTFbits.RF4
#define SW3 PORTDbits.RD15
#define SW4 PORTDbits.RD14
#define SW5 PORTBbits.RB11
#define SW6 PORTBbits.RB10
#define SW7 PORTBbits.RB9

// LCD
#define DISPRS TRISBbits.TRISB15
#define DISPRW TRISBbits.TRISB5
#define DISPEN TRISBbits.TRISB4
#define DB0 PORTEbits.RE0
#define DB1 PORTEbits.RE1
#define DB2 PORTEbits.RE2
#define DB3 PORTEbits.RE3
#define DB4 PORTEbits.RE4
#define DB5 PORTEbits.RE5
#define DB6 PORTEbits.RE6
#define DB7 PORTEbits.RE7
#define FIRST_ROW_LEFT_BOUNDARY 0x80
#define FIRST_ROW_RIGHT_BOUNDARY 0x80 + 16
#define SECOND_ROW_LEFT_BOUNDARY 0xC0
#define SECOND_ROW_RIGHT_BOUNDARY 0xC0 + 16

// Keyboard
#define JA1 PORTCbits.RC2
#define JA2 PORTCbits.RC1
#define JA3 PORTCbits.RC4
#define JA4 PORTGbits.RG6
#define JA7 TRISCbits.TRISC3
#define JA8 TRISGbits.TRISG7
#define JA9 TRISGbits.TRISG8
#define JA10 TRISGbits.TRISG9
#define JB1 PORTDbits.RD9
#define JB2 PORTDbits.RD11
#define JB3 PORTDbits.RD10
#define JB4 PORTDbits.RD8
#define JB7 PORTCbits.RC14
#define JB8 PORTDbits.RD0
#define JB9 PORTDbits.RD1
#define JB10 PORTCbits.RC13

// Variables
char row0Strs[][16] = {"Mode 0:", "Mode 1:", "Mode 2:", "Mode 5:", "Mode 6:", "Mode 7:"};
char row1ModeStrs[][16] = {"Counter ", "Shift ", "Swing "};
char row1DirectionStrs[][16] = {"Up ", "Down ", "Left ", "Right "};
char row1SpeedStrs[][16] = {"Slow", "Fast"};
char row1SpecialStrs[][16] = {"Halt", "Beep Mode", "Exit"};

// Declerations
void setup();
void setupSwitches();
void setupLCD();
void setupKeyboard();

// Ex01
void activateBinaryCounter(BYTE* binCounter, int isBackward);
void activateShift(BYTE* shiftCounter, int isBackward);
void activateFan(BYTE* fanCounter, int isBackward);
void checkPriority(BYTE* binCounter, BYTE* shiftCounter, BYTE* fanCounter,
        BYTE* currentMode, unsigned int* currentSpeed, int* modeDirection,
        BYTE* row1Offset, BYTE* row1Direction);

// Ex02
void sendLCDPulse();
void printToLCD(char str[], BYTE line, BYTE* offset, BYTE* row1Direction);
void clearLCD();
void busy(void);

// Ex03
int getCurrentMode(int keyValue, BYTE* currentMode);
int getKeyPressed();
int in_y(int a);

void main() {
    unsigned int currentSpeed = SLOW_TIME_INTERVAL;
    BYTE currentMode;
    // State variables
    int pressedKey = 0, modeDirection = 0, isHalted = 0;
    BYTE binCounter = 0x00;
    BYTE shiftCounter = 0x00;
    BYTE fanCounter = 0x18;

    BYTE row1Offset = 0; // Relative to 0xC0
    BYTE row1Direction = 1;

    setup();

    do {
        pressedKey = getCurrentMode(getKeyPressed(), &currentMode);
        if (pressedKey == 0x05) { // halt the program
            printToLCD(row0Strs[3], 0, 0, 0);
            printToLCD(row1SpecialStrs[0], 1, &row1Offset, &row1Direction);
            continue;
        }
        checkPriority(&binCounter, &shiftCounter, &fanCounter, &currentMode, 
                &currentSpeed, &modeDirection, &row1Offset, &row1Direction);
        modeDirection = pressedKey == 0x03 ? !modeDirection : modeDirection;
        if (pressedKey == 0x04) // speed / slow
            currentSpeed = currentSpeed == SLOW_TIME_INTERVAL ? QUICK_TIME_INTERVAL : SLOW_TIME_INTERVAL;

        if (pressedKey == 0x06) { // beep
            clearLCD();
            printToLCD(row0Strs[4], 0, 0, 0);
            printToLCD(row1SpecialStrs[1], 1, &row1Offset, &row1Direction);
            for (int i = 0; i < 500; i++) // 500 wavelengths to make sound
            {
                SPEAKER ^= 1;
                for (int j = 0; j < 500; j++);
            }
        }
        for (int j = 0; j < currentSpeed; j++);


        clearLCD();

    } while (pressedKey != 0x07);

    printToLCD(row0Strs[5], 0, 0, 0);
    printToLCD(row1SpecialStrs[2], 1, &row1Offset, &row1Direction);
}

void setup() {
    setupSwitches();
    setupLCD();
    setupKeyboard();
}

void setupSwitches() {
    PORTA = 0x00;
    TRISA &= 0xff00;

    TRISFbits.TRISF3 = 1;
    TRISFbits.TRISF5 = 1;
    TRISFbits.TRISF4 = 1;
    TRISDbits.TRISD15 = 1;
    TRISDbits.TRISD14 = 1;
    TRISBbits.TRISB14 = 0;
    ANSELBbits.ANSB14 = 0;
    TRISBbits.TRISB11 = 1;
    ANSELBbits.ANSB11 = 0;
    TRISBbits.TRISB10 = 1;
    ANSELBbits.ANSB10 = 0;
    TRISBbits.TRISB9 = 1;
    ANSELBbits.ANSB9 = 0;
}

void setupLCD() {
    char control[] = {0x38, 0x38, 0x38, 0x0e, 0x06, 0x01};

    TRISBbits.TRISB15 = 0; // RB15 (DISP_RS) set as an output
    ANSELBbits.ANSB15 = 0; // disable analog functionality on RB15 (DISP_RS)
    TRISDbits.TRISD5 = 0; // RD5 (DISP_RW) set as an output
    TRISDbits.TRISD4 = 0; // RD4 (DISP_EN) set as an output  
    TRISE &= 0xff00;
    ANSELEbits.ANSE2 = 0;
    ANSELEbits.ANSE4 = 0;
    ANSELEbits.ANSE5 = 0;
    ANSELEbits.ANSE6 = 0;
    PORTBbits.RB15 = 0; //rs=0
    PORTDbits.RD5 = 0; //w=0
    ANSELEbits.ANSE7 = 0;

    // Prepare control functions and send them to the LCD controller and waits for it to finish
    for (int i = 0; i < sizeof (control); i++) {

        PORTE = control[i];
        sendLCDPulse();
    }
}

void setupKeyboard() {

    TRISA &= 0xff00; //led
    TRISCbits.TRISC2 = 0; //RC2
    TRISCbits.TRISC1 = 0; //RC1
    TRISCbits.TRISC4 = 0; //RC4            
    TRISGbits.TRISG6 = 0; //RG6
    ANSELGbits.ANSG6 = 0; //???????
    TRISCbits.TRISC3 = 1; //RC3
    CNPUCbits.CNPUC3; // init pull up - nagad
    TRISGbits.TRISG7 = 1; //RG7
    ANSELGbits.ANSG7 = 0;
    CNPUGbits.CNPUG7; // init pull up - nagad
    TRISGbits.TRISG8 = 1; //RG8
    ANSELGbits.ANSG8 = 0; //???????
    CNPUGbits.CNPUG8; // init pull up - nagad
    TRISGbits.TRISG9 = 1; //RG9
    ANSELGbits.ANSG9 = 0; //???????
    CNPUGbits.CNPUG9; // init pull up - nagad
}

// Ex01

void activateBinaryCounter(BYTE * binCounter, int isBackward) {

    PORTA = isBackward ? (*binCounter)-- : (*binCounter)++;
}

void activateShift(BYTE * shiftCounter, int isBackward) {
    if (isBackward) {
        *shiftCounter = *shiftCounter >> 1;
        if (*shiftCounter == 0)
            *shiftCounter = 128;
    } else {
        *shiftCounter = *shiftCounter << 1;

        if (*shiftCounter == 0)
            *shiftCounter = 1;
    }
    PORTA = *shiftCounter;
}

void activateFan(BYTE * fanCounter, int isBackward) {
    BYTE high = *fanCounter & 0xf0;
    BYTE low = *fanCounter & 0x0f;

    if (isBackward) {
        high >>= 1;
        if (high == 0x08)
            high = 0x80;
        low <<= 1;
        if (low == 0x10)
            low = 0x01;
    } else {
        high <<= 1;
        if (high == 0x00)
            high = 0x10;
        low >>= 1;

        if (low == 0x00)
            low = 0x08;
    }
    *fanCounter = high + low;
    PORTA = *fanCounter;
}

void checkPriority(BYTE* binCounter, BYTE* shiftCounter, BYTE* fanCounter,
        BYTE* currentMode, unsigned int* currentSpeed, int* modeDirection,
        BYTE* row1Offset, BYTE* row1Direction) {
    char description[80] = "";

    if (*currentMode == 0x02) {
        strcat(description, row1ModeStrs[2]);
        strcat(description, *modeDirection ? row1DirectionStrs[1] : row1DirectionStrs[0]);
        strcat(description, *currentSpeed == SLOW_TIME_INTERVAL ? row1SpeedStrs[0] : row1SpeedStrs[1]);

        printToLCD(row0Strs[2], 0, 0, 0);
        printToLCD(description, 1, row1Offset, row1Direction);

        return activateFan(fanCounter, *modeDirection);
    }
    if (*currentMode == 0x01) {
        strcat(description, row1ModeStrs[1]);
        strcat(description, *modeDirection ? row1DirectionStrs[1] : row1DirectionStrs[0]);
        strcat(description, *currentSpeed == SLOW_TIME_INTERVAL ? row1SpeedStrs[0] : row1SpeedStrs[1]);

        printToLCD(row0Strs[1], 0, 0, 0);
        printToLCD(description, 1, row1Offset, row1Direction);

        return activateShift(shiftCounter, *modeDirection);
    }
    if (*currentMode == 0x00) {
        strcat(description, row1ModeStrs[0]);
        strcat(description, *modeDirection ? row1DirectionStrs[1] : row1DirectionStrs[0]);
        strcat(description, *currentSpeed == SLOW_TIME_INTERVAL ? row1SpeedStrs[0] : row1SpeedStrs[1]);

        printToLCD(row0Strs[0], 0, 0, 0);
        printToLCD(description, 1, row1Offset, row1Direction);

        return activateBinaryCounter(binCounter, *modeDirection);
    }
}

// Ex02

void sendLCDPulse() {

    PORTDbits.RD4 = 1; // Pulse
    PORTDbits.RD4 = 0;
    busy();
}

void printToLCD(char str[], BYTE line, BYTE* offset, BYTE* row1Direction) {
    PORTBbits.RB15 = 0; // RS = 0

    //int Shiftingrow 1
    //    if (line == 1) {
    //        if (*offset <= -(int) strlen(str) / 2)
    //            *row1Direction = 1;
    //        else if (*offset >= 16 - (int) strlen(str) / 2)
    //            *row1Direction = -1;
    //        *(offset) += *row1Direction;
    //    }

    // Moving cursor to the offset we write in
    PORTE = line == 0 ? FIRST_ROW_LEFT_BOUNDARY : SECOND_ROW_LEFT_BOUNDARY + *offset;
    sendLCDPulse();

    PORTBbits.RB15 = 1; // RS = 1
    PORTDbits.RD5 = 0; // Write      

    for (int i = 0; i < strlen(str); i++) {

        PORTE = str[i];
        sendLCDPulse();
    }
}

void clearLCD() {

    PORTBbits.RB15 = 0;
    PORTDbits.RD5 = 0;
    PORTE = 0b1;
    sendLCDPulse();
}

void busy(void) {
    char RD, RS;
    int STATUS_TRISE;
    int portMap;

    RD = PORTDbits.RD5;
    RS = PORTBbits.RB15;
    STATUS_TRISE = TRISE;
    PORTDbits.RD5 = 1; //w/r
    PORTBbits.RB15 = 0; //rs 
    portMap = TRISE;
    portMap |= 0x80;
    TRISE = portMap;
    do {
        PORTDbits.RD4 = 1; //enable=1
        PORTDbits.RD4 = 0; //enable=0
    } while (PORTEbits.RE7); // BF ?????
    PORTDbits.RD5 = RD;
    PORTBbits.RB15 = RS;
    TRISE = STATUS_TRISE;
}

// Ex03

int getCurrentMode(int keyValue, BYTE* currentMode) {
    if (keyValue == 0) {
        *currentMode = 0x00;
        return 0x00;
    }
    if (keyValue == 1) {
        *currentMode = 0x01;
        return 0x01;
    }
    if (keyValue == 2) {
        *currentMode = 0x02;
        return 0x02;
    }
    if (keyValue == 3)
        return 0x03;
    if (keyValue == 4)
        return 0x04;
    if (keyValue == 5)
        return 0x05;
    if (keyValue == 6)
        return 0x06;
    if (keyValue == 7)

        return 0x07;

    return 0xff;
}

int getKeyPressed() {
    const int valueChanger[4] = {14, 13, 11, 7};
    const int valueMatrix[16] = {0xD, 0xC, 0xB, 0xA, 0xE, 9, 6, 3, 0xF, 
    8, 5, 2, 0, 7, 4, 1}; // order by keyboard
    for (int i = 0; i < 4; i++) {
        int x = valueChanger[i];
        PORTCbits.RC2 = (x & 0x1);
        PORTCbits.RC1 = (x & 0x2) >> 1;
        PORTCbits.RC4 = (x & 0x4) >> 2;
        PORTGbits.RG6 = (x & 0x8) >> 3;
        
        int y = (PORTGbits.RG9 << 3) | (PORTGbits.RG8 << 2) | (PORTGbits.RG7 << 1) | PORTCbits.RC3;
        
        for (int j = 0; j < 4; j++) {
            if (y == valueChanger[j]) {

                return valueMatrix[i * 4 + j];
            }
        }
    }
    return -1;
}

//int in_y(int a) {
//    int j = 1, flag = 0;
//
//    if (!PORTCbits.RC3) {
//        flag = 1;
//        j = 1;
//    } else if (!PORTGbits.RG7) {
//        flag = 1;
//        j = 2;
//    } else if (!PORTGbits.RG8) {
//        flag = 1;
//        j = 3;
//    } else if (!PORTGbits.RG9) {
//        flag = 1;
//        j = 4;
//    }
//
//    return flag == 0 ? 0xff : j | (a << 4);
//}