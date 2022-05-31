#include <xc.h>
#include <stdio.h>
#include <stdlib.h>

#pragma config JTAGEN = OFF     
#pragma config FWDTEN = OFF
#pragma config FNOSC =	FRCPLL
#pragma config FSOSCEN =	OFF
#pragma config POSCMOD =	EC
#pragma config OSCIOFNC =	ON
#pragma config FPBDIV =     DIV_1
#pragma config FPLLIDIV =	DIV_2
#pragma config FPLLMUL =	MUL_20
#pragma config FPLLODIV =	DIV_1

#define UPPER_LINE 0
#define BUTTOM_LINE 1
#define STOP 0
#define SIZE_LCD 16
#define SIZE_FONT 16
#define SIZE_OBJ 3
#define SIZE_CONTROL 7
#define TIMES_ENTER_LCD 20000
void write_object(int line,int start_idx,int start_idx_object);
//void change_font();// eye&eye <--> wink&eye
void move_object_on_lcd(int* start_idx, int* dir,int* start_idx_object);
void sw1_Line_Selection(int* line);// 0- upLine,1-ButtomLine on lcd
void sw0_Moving(int* move);// 1-move,0-stop
void sw7_Make_Sound();// 1-sound, 0-stop

void busy(void);
void main (void)
{
    TRISFbits.TRISF3 = 1; // RF3 (SW0) configured as input
    TRISFbits.TRISF5 = 1; // RF5 (SW1) configured as input
    TRISBbits.TRISB9 = 1; // RB9 (SW7) configured as input
    ANSELBbits.ANSB9 = 0; // RB9 (SW7) disabled analog
    //SOUND
    TRISBbits.TRISB14 = 0;// MX3 (A_ OUT) 
    ANSELBbits.ANSB14 = 0;
    
    int j,i;
    int line, move,sound;
    int start_idx=0, dir = 1, start_idx_object = 0;
    int counter_lcd = 0;
    while(1){
        sw1_Line_Selection(&line);
        sw0_Moving(& move);
        sw7_Make_Sound(&sound);
        if(sound!=STOP){
            LATBbits.LATB14^=1;
        }
        if(counter_lcd == TIMES_ENTER_LCD){
            counter_lcd = 0;
            if(move != STOP) {
                move_object_on_lcd(&start_idx,&dir,&start_idx_object);
            }
            write_object(line,start_idx,start_idx_object);
        }
        counter_lcd+=1;  
    }
   
}
void sw0_Moving(int* move){// 1-move,0-stop
    if(PORTFbits.RF3){
        *move = !STOP;
    }else{
        *move = STOP;
    }
}

void sw1_Line_Selection(int* line){// 0- upLine,1-ButtomLine on lcd
    if(PORTFbits.RF5){
        *line = BUTTOM_LINE;
    }else{
        *line = UPPER_LINE;
    }
    
}

void sw7_Make_Sound(int* sound){// 1-sound, 0-stop
   
    int i;
    if(PORTBbits.RB9){
        *sound = !STOP;
    }else{
        *sound = STOP; 
    }

}

void move_object_on_lcd(int* start_idx, int* dir,int* start_idx_object)
{
    int i;
    if(*start_idx == SIZE_LCD +1){
        *start_idx = 0;
        *start_idx_object =SIZE_OBJ -1;
        *dir = -1;//direction of writing the object
    }else{
        if(*start_idx_object == 0){
            *dir = 1;
        }
        if(*dir == -1){//dir = -1 -->change the direction of writing form index startIndex-->endOfString
            *start_idx_object-=1;
        }else{//dir = 1 -->change the direction of writing form index 0-->endOfString
            *start_idx +=1;//now the whole object is written, we can start move it forward.
        }
    }
    
 
}

void write_object(int line,int start_idx,int start_idx_object){
    int i = 0;
//    PORTE = 0x40;
    char  CG_eyes[SIZE_FONT]={0x06,0x09,0x09,0x09,0x0f,0x0f,0x06,//eye middle
                              0x20,0x24,0x3b,0x27,0x29,0x20,0x30,0x30};//wink
//    char myPicture[] = {   0x4, 0x4, 0x1d, 0x2, 0x2, 0x1d, 0x4, 0x4, //Done one part
//     		        0x20, 0x3e, 0x21, 0x32, 0x20, 0x2d, 0x32, 0x2c, //Done second part
//		        0x48, 0x48, 0x4f, 0x50, 0x50, 0x4f, 0x48, 0x48}; // Done third part
    char CG_Dollar[1] = {0x24};
    char eyes[SIZE_OBJ] = {2,1,0};// wink, open eye
    TRISBbits.TRISB15 = 0; // RB15 (DISP_RS) set as an output
    ANSELBbits.ANSB15 = 0; // disable analog functionality on RB15 (DISP_RS)
    TRISDbits.TRISD5 = 0; // RD5 (DISP_RW) set as an output
    TRISDbits.TRISD4 = 0; // RD4 (DISP_EN) set as an output
    TRISE&=0xff00;
    ANSELEbits.ANSE2 = 0;
    ANSELEbits.ANSE4 = 0;
    ANSELEbits.ANSE5 = 0;
    ANSELEbits.ANSE6 = 0;
   
    char control[]={0x38,0x38,0x38,0xe,0x6,0x1, 0x80, 0xc0};
   
    PORTBbits.RB15=0;//rs=0 init lcd
    PORTDbits.RD5=0;//w=0 
    ANSELEbits.ANSE7 = 0; 
    for(i=0;i<7;i++)                                                
    {
        PORTE=control[i];
        PORTDbits.RD4=1; 
        PORTDbits.RD4=0;
        busy();
    }
//    PORTE = 0x80;
    if(line == UPPER_LINE){
        eyes[0] = 0;  
        
    }else{
           //Next line
        PORTE = control[7];// clean the line
        PORTDbits.RD4=1;//input the new line to the LCD
        PORTDbits.RD4=0;//output of what we inserted.
        busy();
    }
    
    PORTBbits.RB15=1;//rs=1 
    PORTDbits.RD5=0;//w=0
    //loop for put spaces in th start of the string(to make the string move)
    i = 0;
    while(i<start_idx){
        PORTE = 0x20; //space
        PORTDbits.RD4=1;
        PORTDbits.RD4=0;
        busy();
        i++;
    }
    for(i=start_idx_object;i<1;i++)
    {
        PORTE=CG_Dollar[i];
        PORTDbits.RD4=1;
        PORTDbits.RD4=0;
        busy();
    }
 
}

//make delay
void busy(void)
{
    char RD,RS;
    int STATUS_TRISE;
    int portMap;
    RD=PORTDbits.RD5;
    RS=PORTBbits.RB15;
    STATUS_TRISE=TRISE;
	PORTDbits.RD5 = 1;//w/r
	PORTBbits.RB15 = 0;//rs 
    portMap = TRISE;
	portMap |= 0x80;
	TRISE = portMap;
    do
    {
        PORTDbits.RD4=1;//enable=1
        PORTDbits.RD4=0;//enable=0
    }
    while(PORTEbits.RE7); // BF ?????
    PORTDbits.RD5=RD; 
    PORTBbits.RB15=RS;
    TRISE=STATUS_TRISE;   
}
