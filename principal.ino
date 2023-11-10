const byte start_end_Flag = 0x7E;

//endereços - 4bits----------------------------------
String p_addr = "0000";
String s1_addr = "0001";
String s2_addr = "0010";

//parâmetros do ctrl----------------------------------
int NS1 = 0;//TERÃO 2 FRAMES NA FILA
int ACK1 = 0;
int NS2 = 0;//TERÃO 2 FRAMES NA FILA
int ACK2 = 0;

//buffers de dados--------------------------------
String buffer1[2] = {"101", "1010"};
String ult_recebido1;
String buffer2[2] = {"101000", "10100"};
String ult_recebido2;

//divisor do crc----------------------------------
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
    
    for (int j = 0; j < (dado.length() - 1); j++)
    {
      pow2 *= 2;
    }

    out += pow2;
  }
  if (dado[dado.length() - 1]) out += 1;

  return out;
}

String NbrToBitS(int entrada, int tamanho = 4)
{
  String ent_clone = String(entrada, BIN);
  String out;

  for (int i = (tamanho - 1); i >= 0; i++) out += '0';

  int dif_tamanho = tamanho - ent_clone.length();

  for (int i = (ent_clone.length() - 1); i >= 0; i++) out[i] = ent_clone[i - dif_tamanho];

  return out;
}

bool RX0_TX1 = 0;

int com_byte;
String bts_clone;
int AUX;
String AUX1;
bool alternar = 0; //0: buffer 1 manda|| 1: buffer 2


void setup()
{
  Serial.begin(9600);
  RX0_TX1 = 1; //secundários --> RX0_TX1 = 0
  AUX = 0;
  AUX1 = "";
}

void loop()
{
  if (RX0_TX1) //fala---------------------------------------------------------
  {
    Serial.write(start_end_Flag);


    //ADDRESS---------------------

    if (alternar)//buffer 1
    {
      bts_clone += s1_addr;
      bts_clone += p_addr;
    }
    else //buffer 2
    {
      bts_clone += s2_addr;
      bts_clone += p_addr;
    }

    com_byte = strToNbr(bts_clone);
    bts_clone = "";

    Serial.write(com_byte);//<--ADDRESS



    //CONTROLL---------------------

    if (alternar)//buffer 1
    {
      bts_clone += '0';
      bts_clone += NbrToBitS(NS1, 3);
      bts_clone += '0';
      bts_clone += NbrToBitS(ACK1, 3);
    }
    else // buffer 2
    {
      bts_clone += '0';
      bts_clone += NbrToBitS(NS2, 3);
      bts_clone += '0';
      bts_clone += NbrToBitS(ACK2, 3);
    }

    com_byte = strToNbr(bts_clone);
    bts_clone = "";
    Serial.write(com_byte);//<--CONTROLL



    //DADOS---------------------

    //NS par envia [0], NS ímpar
    if (alternar == 0)
    {
      if (NS1/2)
      {
        com_byte = strToNbr(buffer1[0]);
        bts_clone = buffer1[0];
      }
      else
      { 
        com_byte = strToNbr(buffer1[1]);
        bts_clone = buffer1[1];
      }
      NS1++;
    } 
    else 
    {
      if (NS2/2)
      {
        com_byte = strToNbr(buffer2[0]);
        bts_clone = buffer2[0];
      }
      else
      {
        com_byte = strToNbr(buffer2[1]);
        bts_clone = buffer2[1];
      }
      NS2++;
    }
    alternar = !alternar;

    Serial.write(com_byte);//<--DADOS


    //CRC---------------------
    com_byte = strToNbr(crcGen(bts_clone, DIV));

    Serial.write(com_byte); //<--CRC

    Serial.write(start_end_Flag);
    RX0_TX1 = 0;
  }
  else if(Serial.available() > 0)//ouve--------------------------------------------
  {    
    com_byte = Serial.read();

    if (com_byte == start_end_Flag)
    {
      delay(500);

      //ADDRESS---------------------

      com_byte = Serial.read();
      delay(500);
      if (bitRead(com_byte, 4) == strToNbr(p_addr))
      {
        bts_clone = NbrToBitS(com_byte, 8);

        bts_clone.remove(0, 4);//tira o destinatário

        if (bitRead(com_byte, 0) == 1) ult_recebido1 = bts_clone; // pega o remetente e salva
        else if (bitRead(com_byte, 1) == 1) ult_recebido2 = bts_clone;
        
        //Seleção de onde que veio a mensagem----------------------
        if (bitRead(com_byte, 0) == 1) // S1--------------------------------------------------------
        {
          //CONTROLL---------------------
          com_byte = Serial.read();
          bts_clone = NbrToBitS(com_byte, 8);
          delay(500);

          //pega o ack põe em AUX
          AUX1 = bts_clone[bts_clone.length() - 3];
          AUX1 += bts_clone[bts_clone.length() - 2];
          AUX1 += bts_clone[bts_clone.length() - 1];
          AUX = strToNbr(AUX1);

          if (AUX > NS1) //compara o ack da msg com ns do ultimo frame enviado--------------------
          {
            NS1 = AUX; //move a fila do buffer------------------

            //AUX1 pega o NS da msg--------------------------
            AUX1 = bts_clone[1];
            AUX1 += bts_clone[2];
            AUX1 += bts_clone[3];

            ACK1 = AUX + 1;//incrementa ack1------------------------

            //DADOS --------------------------
            do
            {
              com_byte = Serial.read();
              ult_recebido1 = NbrToBitS(com_byte);
              delay(500);
            }
            while (com_byte != start_end_Flag);

            ult_recebido2.remove((ult_recebido2.length() - 8), 8); //arranca a flag final
          }
          else //caso o frame anterior não tenha sido mandado corretamente------------------------------
          {

          }
        }
        else if (bitRead(com_byte, 1) == 1)//S2--------------------------------------------------------
        {
          //CONTROLL---------------------
          com_byte = Serial.read();
          bts_clone = NbrToBitS(com_byte, 8);
          delay(500);

          //pega o ack põe em AUX--------------------
          AUX1 = bts_clone[bts_clone.length() - 3];
          AUX1 += bts_clone[bts_clone.length() - 2];
          AUX1 += bts_clone[bts_clone.length() - 1];
          AUX = strToNbr(AUX1);

          if (AUX > NS1) //compara o ack da msg com ns do ultimo frame enviado-----------
          {
            NS2 = AUX; //move a fila do buffer--------------------

            //AUX1 pega o NS da msg-----------------
            AUX1 = bts_clone[1];
            AUX1 += bts_clone[2];
            AUX1 += bts_clone[3];

            ACK2 = AUX + 1;//incrementa ack1-------------------------
            
            do
            {
              com_byte = Serial.read();
              ult_recebido2 = NbrToBitS(com_byte);
              delay(500);
            }
            while (com_byte != start_end_Flag);

            ult_recebido2.remove((ult_recebido2.length() - 8), 8); //arranca a flag final
          }
          else //caso o frame anterior não tenha sido mandado corretamente----------------------------------
          {

          }
        }
      }
    }
  }
  
}
