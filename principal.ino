
const byte start_end_Flag = 0x7E;

//endereços - 4bits
String p_addr = "0000";
String s1_addr = "0001";
String s2_addr = "0010";

//parâmetros do ctrl
int NS1 = 0;//TERÃO 2 FRAMES NA FILA
int ACK1 = 0;
int NS2 = 0;//TERÃO 2 FRAMES NA FILA
int ACK2 = 0;

//buffers de dados
String buffer1[2] = {"101", "1010"};
String ult_recebido1;
String buffer2[2] = {"101000", "10100"};
String ult_recebido2;

//divisor do crc
String DIV = "1000100000010001";


String crcGen(String entrada, String DIV)
{
    int div_totais = entrada.length();
    String crc;

    for (int i = 0; i < (DIV.length() - 1); i++) entrada+= '0';
    for (int i = 0; i < div_totais; i++) for (int j = 0; j < DIV.length(); j++) entrada[i + j] = (entrada[i + j] != DIV[j]);

    for (int i = (entrada.length() - 1); i < DIV.length(); i++) crc += entrada[i];

    return crc;
}

int strToNbr(String dado)
{
  int out = 0;
  int pow2 = 1;
  for (int i = 0; i < dado.length(); i++) 
  {
    pow2 = 1;
    
    for (int j = dado.length(); j > 0; j--)
    {
      pow2 *= 2;
    }

    out += pow2;
  }
  if (dado[dado.length() - 1]) out += 1;

  return out;
}

int RX0_TX1 = 0;

void setup()
{
  Serial.begin(9600);
  RX0_TX1 = 1;
}

int byte_to_send;
String bts_clone;
int CRC;
bool alternar = 0; //0: buffer 1 manda|| 1: buffer 2
void loop()
{
  if (RX0_TX1)
  {
    
    Serial.write(start_end_Flag);

    //ADDRESS---------------------

    


    Serial.write(byte_to_send);//<--ADDRESS


    //CONTROLL---------------------

    if (alternar)//buffer 1
    {
      bts_clone += '0';
      bts_clone += String(NS1, BIN);
      bts_clone += '0';
      bts_clone += String(ACK1, BIN);
    }

    Serial.write(byte_to_send);//<--CONTROLL

    
    
    //DADOS---------------------

    //NS par envia [0], NS ímpar
    if (alternar == 0)
    {
      if (NS1/2)
      {
        byte_to_send = strToNbr(buffer1[0]);
        bts_clone = buffer1[0];
      }
      else
      { 
        byte_to_send = strToNbr(buffer1[1]);
        bts_clone = buffer1[1];
      }
      NS1++;
    } 
    else 
    {
      if (NS2/2)
      {
        byte_to_send = strToNbr(buffer2[0]);
        bts_clone = buffer2[0];
      }
      else
      {
        byte_to_send = strToNbr(buffer2[1]);
        bts_clone = buffer2[1];
      }
      NS2++;
    }
    alternar = !alternar;

    Serial.write(byte_to_send);//<--DADOS

    
    //CRC---------------------
    byte_to_send = strToNbr(crcGen(bts_clone, DIV));

    Serial.write(byte_to_send); //<--CRC

    Serial.write(start_end_Flag);
    RX0_TX1 = 0;
  }
  else
  {

  }
}