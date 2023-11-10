//#include "CRC_lib.cpp"
//#include "frame_utilities.cpp"
#include <Arduino.h>


using namespace std;   

const byte startFlag = 0x7E;
const byte addressPS1 = B00010010;
const byte control = B00011000;
const byte data = B10101010;
const byte CRC = B00110011;
const byte endFlag = 0x7E;

bool RX0_TX1; //1 = tx, 0 = rx
int BufferFila = 0;
int fila; 

void setup()
{
    RX0_TX1 = 1; //ativado modo tx
    
}



void loop()
{
    if (RX0_TX1) // modo tx
    {
        //código para enviar dados

        
        if (BufferFila < fila) RX0_TX1 = 0;
    }
    else //modo rx
    {
        //código para pegar dados

        RX0_TX1 = 1;
    }

    
    delay(200);
}