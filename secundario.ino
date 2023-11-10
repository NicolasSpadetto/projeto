const byte start_end_Flag = 0x7E;

//endereços - 4bits----------------------------------
String p_addr = "0000";
String my_addr = "0001"; //para outros secundários aumentar endereço
//String s2_addr = "0010";

//parâmetros do ctrl----------------------------------
int NS = 0;//TERÃO 2 FRAMES NA FILA
int ACK = 0;

/*
int NS2 = 0;//TERÃO 2 FRAMES NA FILA
int ACK2 = 0;
*/

//buffers de dados--------------------------------
String buffer[2] = {"1100", "1101"};
String ult_recebido;

/*
String buffer2[2] = {"101000", "10100"};
String ult_recebido2;
*/

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

bool crcCmp(String ult_frm, String DIV)
{
  String dados = "";
  String crc_rec = "";
  for (int i = 4; i < (ult_frm.length() - 16); i++)  dados += ult_frm[i];
  for (int i = ult_frm.length() - 16; i < ult_frm.length(); i++) crc_rec += ult_frm[i];

  if (crcGen(dados, DIV) == crc_rec) return 1;
  else return 0;
}

String askFrame(String ult_frm, String my_addr, int my_NS, int my_ACK)// PEDE UM FRAME NOVO---------------------
{
  String ask_frame = "";
  
  //montando o frame: addrENV addrREC iFRAME nsENV 0 ackENV data(fixo) CRC(fixo)
  ask_frame += my_addr;
  //8b
  ask_frame += ult_frm[0];
  ask_frame += ult_frm[1];
  ask_frame += ult_frm[2];
  ask_frame += ult_frm[3];
  //8b
  ask_frame += '0';
  ask_frame += NbrToBitS(my_NS, 3);
  ask_frame += '0';
  ask_frame += NbrToBitS(my_ACK, 3);
  
  //ele não ira usar nem data nem crc
  
  return ask_frame;

}

bool RX0_TX1 = 0;

int com_byte;
String bts_clone;
int AUX;
String AUX1;
bool alternar = 0; //0: buffer 1 manda|| 1: buffer 2
int CRC = 0;// 0 não pede outro frame, 1 pede pra o principal


void setup()
{
  Serial.begin(9600);
  RX0_TX1 = 0; //secundários --> RX0_TX1 = 0
  AUX = 0;
  AUX1 = "";
}

void loop()
{
  bts_clone = "";

  if (RX0_TX1) //fala---------------------------------------------------------
  {
    if (CRC == 0)
    {
      Serial.write(start_end_Flag);


      //ADDRESS---------------------
    
      bts_clone += my_addr;
      bts_clone += p_addr;
      com_byte = strToNbr(bts_clone);
      bts_clone = "";

      Serial.write(com_byte);//<--ADDRESS

      //CONTROLL---------------------
    
      bts_clone += '0';
      bts_clone += NbrToBitS(NS, 3);
      bts_clone += '0';
      bts_clone += NbrToBitS(ACK, 3);
      com_byte = strToNbr(bts_clone);
      bts_clone = "";

      Serial.write(com_byte);//<--CONTROLL

      //DADOS---------------------

      //NS par envia [0], NS ímpar

      if (NS/2)
      {
        com_byte = strToNbr(buffer[0]);
        bts_clone = buffer[0];
      }
      else
      { 
        com_byte = strToNbr(buffer[1]);
        bts_clone = buffer[1];
      }
        
      //CRC---------------------
      com_byte = strToNbr(crcGen(bts_clone, DIV));

      Serial.write(com_byte); //<--CRC

      Serial.write(start_end_Flag);
      
    }
    else //pede para o principal
    {
      Serial.write(start_end_Flag);
      
      bts_clone = askFrame(ult_recebido, my_addr, NS, ACK);
      
      //endereco
      AUX1 = bts_clone[0];
      AUX1 += bts_clone[1];
      AUX1 += bts_clone[2];
      AUX1 += bts_clone[3];
      AUX1 += bts_clone[4];
      AUX1 += bts_clone[5];
      AUX1 += bts_clone[6];
      AUX1 += bts_clone[7];

      Serial.write(strToNbr(AUX1));

      //control
      bts_clone.remove(0, 8);

      Serial.write(strToNbr(bts_clone));

      Serial.write(start_end_Flag);
      CRC = 0;
    }
    
    RX0_TX1 = !RX0_TX1;
    bts_clone = "";
  }
  else if((Serial.available() > 0) && (!RX0_TX1))//ouve--------------------------------------------
  {    
    com_byte = Serial.read();

    if (com_byte == start_end_Flag)
    {
      delay(1000);

      //ADDRESS---------------------

      com_byte = Serial.read();
      delay(1000);
      if (bitRead(com_byte, 4) == strToNbr(my_addr))// tem q trocar caso mude o secundário ------------------------------------------------------
      {
        bts_clone = NbrToBitS(com_byte, 8);

        bts_clone.remove(0, 4);//tira o destinatário

        ult_recebido = bts_clone; // pega o remetente e salva-----------
        
        
        
        
        
        //CONTROLL---------------------
        com_byte = Serial.read();
        bts_clone = NbrToBitS(com_byte, 8);
        delay(1000);

        //pega o ack põe em AUX
        AUX1 = bts_clone[bts_clone.length() - 3];
        AUX1 += bts_clone[bts_clone.length() - 2];
        AUX1 += bts_clone[bts_clone.length() - 1];
        AUX = strToNbr(AUX1);

        if (AUX > NS) //compara o ack da msg com ns do ultimo frame enviado--------------------
        {
          NS = AUX; //move a fila do buffer------------------

          //AUX1 pega o NS da msg--------------------------
          AUX1 = bts_clone[1];
          AUX1 += bts_clone[2];
          AUX1 += bts_clone[3];

          ACK = strToNbr(AUX1) + 1;//incrementa ack1------------------------

          //DADOS --------------------------
          do
          {
            com_byte = Serial.read();
            ult_recebido = NbrToBitS(com_byte);
            delay(1000);
          }
          while (com_byte != start_end_Flag);

            ult_recebido.remove((ult_recebido.length() - 8), 8); //arranca a flag final


            //CRC ---------------------------------------------------------------------------------------
            CRC = (!(crcCmp(ult_recebido, DIV)));// mantém 0 quando ta td certo, 1 quando deu ruim
            if (CRC != 0) ACK--;
        }
        else //caso o frame anterior não tenha sido mandado corretamente------------------------------
        {
          //realiza a opr normalmente porém, incrementando apenas o ack e sem armazenar nada
          //DADOS --------------------------
          NS = AUX; //move a fila do buffer------------------
          
          //AUX1 pega o NS da msg--------------------------
          AUX1 = bts_clone[1];
          AUX1 += bts_clone[2];
          AUX1 += bts_clone[3];
          ACK = strToNbr(AUX1) + 1;

          do
          {
            com_byte = Serial.read();
            delay(1000);
          }
          while (com_byte != start_end_Flag);

          ult_recebido.remove((ult_recebido.length() - 8), 8); //arranca a flag final
            
        }

        
        delay(1000);
        RX0_TX1 = !RX0_TX1;//volta ao modo falar----------------------------------------
      }
    }
  }
  
}