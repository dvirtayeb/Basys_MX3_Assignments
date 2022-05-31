#include <xc.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/attribs.h>
#include <string.h>
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
#define SECOND_ROW_LEFT_BOUNDARY 0xC0
#define SECOND_ROW_RIGHT_BOUNDARY 0xC0+16
#define FIRST_ROW_LEFT_BOUNDARY 0x80
#define FIRST_ROW_RIGHT_BOUNDARY 0x80+16

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
    } while (PORTEbits.RE7);
    PORTDbits.RD5 = RD;
    PORTBbits.RB15 = RS;
    TRISE = STATUS_TRISE;
}

void runLCDAction() {
    PORTDbits.RD4 = 1;
    PORTDbits.RD4 = 0;
    busy();
}

void jumpLCDTo(int offset) {
    int tempRS = PORTBbits.RB15;
    PORTBbits.RB15 = 0;
    PORTE = 0x80 + offset;
    runLCDAction();
    PORTBbits.RB15 = tempRS;
}

void setupLCD() {
    char control[] = {0x38, 0x38, 0x38, 0xe, 0x6, 0x1};
    for (int i = 0; i < 6; i++) {
        PORTE = control[i];
        runLCDAction();
    }
    PORTBbits.RB15 = 1; //rs = 1
}

void init() {
    TRISA &= 0xff00;
    PORTA = 0;
    TRISFbits.TRISF3 = 1; // RF3 (SW0) configured as input
    TRISFbits.TRISF5 = 1;
    TRISFbits.TRISF4 = 1;
    TRISDbits.TRISD15 = 1;
    TRISDbits.TRISD14 = 1;
    TRISBbits.TRISB11 = 1;
    ANSELBbits.ANSB11 = 0;
    TRISBbits.TRISB10 = 1;
    ANSELBbits.ANSB10 = 0;
    TRISBbits.TRISB9 = 1;
    ANSELBbits.ANSB9 = 0;
    ANSELBbits.ANSB14 = 0;
    TRISBbits.TRISB14 = 0;
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
    TRISCbits.TRISC2 = 0; //RC2            
    TRISCbits.TRISC1 = 0; //RC1           
    TRISCbits.TRISC4 = 0; //RC4            
    TRISGbits.TRISG6 = 0; //RG6
    ANSELGbits.ANSG6 = 0; //???????
    TRISCbits.TRISC3 = 1; //RC3
    CNPUCbits.CNPUC3;
    TRISGbits.TRISG7 = 1; //RG7
    ANSELGbits.ANSG7 = 0;
    CNPUGbits.CNPUG7;
    TRISGbits.TRISG8 = 1; //RG8
    ANSELGbits.ANSG8 = 0; //???????
    CNPUGbits.CNPUG8; //?????
    TRISGbits.TRISG9 = 1; //RG9
    ANSELGbits.ANSG9 = 0; //???????
    CNPUGbits.CNPUG9; //????? 
    setupLCD();
}

void writeSecondRow(char ch) {
    jumpLCDTo(SECOND_ROW_LEFT_BOUNDARY - FIRST_ROW_LEFT_BOUNDARY + 4);
    const char* text = "Mode ";
    const char endText = '-';
    for (int i = 0; i < strlen(text); i++) {
        PORTE = text[i];
        runLCDAction();
    }
    PORTE = ch;
    runLCDAction();
    PORTE = endText;
    runLCDAction();
}

char calculateChar(int num) {
    if (num < 10) {
        return '0' + num;
    } else {
        return '7' + num;
    }
}
void __ISR(_TIMER_4_VECTOR, ipl2auto) Timer4SR(void);

void __ISR(_TIMER_4_VECTOR, ipl2) Timer4SR(void) {

    PORTA++;
    IFS0bits.T4IF = 0;
}

void main() {
    init();
    PR4 = 0xffff; //             set period register, generates one interrupt every 1 ms
    TMR4 = 0; //             initialize count to 0    
    T4CONbits.TCKPS0 = 1; //            1:256 prescale value
    T4CONbits.TCKPS1 = 1;
    T4CONbits.TCKPS2 = 1;
    T4CONbits.TGATE = 0; //             not gated input (the default)
    T4CONbits.TCS = 0; //             PCBLK input (the default)
    T4CONbits.ON = 1; //             turn on Timer1
    IPC4bits.T4IP = 2; //             priority
    IPC4bits.T4IS = 0; //             subpriority
    IFS0bits.T4IF = 0; //             clear interrupt flag
    IEC0bits.T4IE = 1;
    INTCONbits.MVEC = 1; //vector interrupt
    IPTMR = 0; //INTERRUPT PROXIMITY TIMER REGISTER
    asm("ei"); //on interrupt

    while (1);
}