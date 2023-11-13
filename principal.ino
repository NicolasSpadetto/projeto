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
    int DIV_mag = DIV.length();
    String crc = "";

    for (int i = 0; i < (DIV_mag - 1); i++) entrada += '0';
 
    for (int i = 0; i < div_totais; i++) for (int j = 0; j < DIV_mag; j++) 
    {
      if (entrada[i + j] != DIV[j]) entrada[i + j] = '1';
      else entrada[i + j] = '0';
    }

    for (int i = (div_totais - 1); i < entrada.length(); i++) crc += entrada[i];

    return crc;
}

int strToNbr(String dado)
{
  int out = 0;
  int pow2;
  int mag = dado.length();
  
  for (int i = 0; i < mag; i++) 
  {
    pow2 = 0;
    
    if (dado[i] == '1') 
    {
      pow2 = 1;
      for (int j = 0; j < (mag - i - 1); j++) pow2 *= 2;
    }
    
    out += pow2;    
  }
  
  return out;
}

String NbrToBitS(int entrada, int tamanho = 4)
{
  String ent_clone = String(entrada, BIN);
  String out = "";

  for (int i = 0; i < tamanho; i++) out += '0';

  int dif = tamanho - ent_clone.length();

  for (int i = (ent_clone.length() - 1); i >= 0; i--) out[i + dif] = ent_clone[i];

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

String askFrame(String ult_frm, String s1_addr, int my_NS, int my_ACK)// PEDE UM FRAME NOVO---------------------
{
  String ask_frame = "";
  
  //montando o frame: addrENV addrREC iFRAME nsENV 0 ackENV data(fixo) CRC(fixo)
  ask_frame += s1_addr;
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
int CRC = 0;// 0 não pede outro frame, 1 pede pra s1, 2 pede pra s2


void setup()
{
  Serial.begin(9600);
  RX0_TX1 = 1; //secundários --> RX0_TX1 = 0
  AUX = 0;
  AUX1 = "";
}

void loop()
{
  delay(1000);
  if (RX0_TX1) //fala---------------------------------------------------------
  {
    if (CRC == 0)
    {
      Serial.write(start_end_Flag);
      
      //ADDRESS---------------------

      if (!alternar)//buffer 1
      {
        bts_clone = s1_addr;
        bts_clone += p_addr;

      }
      else //buffer 2
      {
        bts_clone = s2_addr;
        bts_clone += p_addr;
        
      }
      

      com_byte = strToNbr(bts_clone);
      
      Serial.write(com_byte);//<--ADDRESS

      //CONTROLL---------------------

      if (!alternar)//buffer 1
      {
        bts_clone = '0';
        bts_clone += NbrToBitS(NS1, 3);
        bts_clone += '0';
        bts_clone += NbrToBitS(ACK1, 3);
      }
      else // buffer 2
      {
        bts_clone = '0';
        bts_clone += NbrToBitS(NS2, 3);
        bts_clone += '0';
        bts_clone += NbrToBitS(ACK2, 3);
      }
      
      com_byte = strToNbr(bts_clone);

      
      Serial.write(com_byte);//<--CONTROLL

      //DADOS---------------------

      //NS1 par envia [0], NS1 ímpar
      if (alternar == 0)
      {
        if ((NS1 % 2) == 0)
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
        if ((NS2 % 2) == 0)
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
      
      
      Serial.write(com_byte);//<--DADOS
            
      //CRC---------------------
      com_byte = strToNbr(crcGen(bts_clone, DIV));

            
      Serial.write(com_byte); //<--CRC

      Serial.write(start_end_Flag);
      
      alternar = !alternar;
    }
    else if (CRC == 1) //pede para o s1
    {
      Serial.write(start_end_Flag);
      
      bts_clone = askFrame(ult_recebido1, p_addr, NS1, ACK1);
      
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
    else //pede para s2
    {
      Serial.write(start_end_Flag);
      bts_clone = askFrame(ult_recebido2, p_addr, NS2, ACK2);
      
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
    
  }
  else if(Serial.available() > 0)//ouve--------------------------------------------
  {    
    delay(3000);
    
    
    com_byte = Serial.read();

    if (com_byte == start_end_Flag)
    {
      delay(1000);

      //ADDRESS---------------------
      
      com_byte = Serial.read();

      delay(1000);

      AUX1 = NbrToBitS(com_byte, 8);
      
      bts_clone = AUX1[0];
      bts_clone += AUX1[1];
      bts_clone += AUX1[2];
      bts_clone += AUX1[3];


      if (bts_clone == p_addr)
      {
        AUX1.remove(0, 4);//tira o destinatário

        if (AUX1[0] == '1') ult_recebido1 = AUX1; // pega o remetente e salva-----------
        else if (AUX1[1] == '1') ult_recebido2 = AUX1;
        
        //Seleção de onde que veio a mensagem----------------------
        if (AUX1 == s1_addr) // S1--------------------------------------------------------
        {
          //CONTROLL---------------------
          com_byte = Serial.read();
          bts_clone = NbrToBitS(com_byte, 8);
          delay(1000);

          AUX1 = bts_clone[1];
          AUX1 += bts_clone[2];
          AUX1 += bts_clone[3];

          AUX = strToNbr(AUX1);// segura o NS da mensagem

          ACK1 = strToNbr(AUX1) + 1;//incrementa ack1------------------------

                    
          //pega o ack põe em AUX
          AUX1 = bts_clone[bts_clone.length() - 3];
          AUX1 += bts_clone[bts_clone.length() - 2];
          AUX1 += bts_clone[bts_clone.length() - 1];
          AUX = strToNbr(AUX1);



          if (AUX == NS1) //compara o ack da msg com NS1 do ultimo frame enviado--------------------
          {
            //DADOS --------------------------
            do
            {
              com_byte = Serial.read();
              ult_recebido1 = NbrToBitS(com_byte);
              delay(500);
            }
            while (com_byte != start_end_Flag);

            ult_recebido1.remove((ult_recebido1.length() - 8), 8); //arranca a flag final

            AUX1 = ult_recebido1;
            AUX1.remove(0, 4); //arranca endereço
            //CRC ---------------------------------------------------------------------------------------
            CRC = (!(crcCmp(AUX1, DIV)));// mantém 0 quando ta td certo, 1 quando deu ruim

            
            

            if (CRC != 0) ACK1--;
          
          }
          else //caso do ult frame, que este mandou, não tenha sido mandado corretamente----------------------------------
          {
            alternar = !alternar; // para poder responder o mesmo secundário
            //realiza a opr normalmente porém, voltando uma instancia de NS e sem armazenar nada
            //DADOS --------------------------
            NS1 = AUX; //volta um NS

            do
            {
              com_byte = Serial.read();
              delay(500);
            }
            while (com_byte != start_end_Flag);

            ult_recebido1.remove((ult_recebido1.length() - 8), 8); //arranca a flag final
            
          }

        }
        else if (AUX1 == s2_addr)//S2--------------------------------------------------------
        {
          //CONTROLL---------------------
          com_byte = Serial.read();
          bts_clone = NbrToBitS(com_byte, 8);
          delay(1000);

          AUX1 = bts_clone[1];
          AUX1 += bts_clone[2];
          AUX1 += bts_clone[3];

          AUX = strToNbr(AUX1);// segura o NS da mensagem

          ACK2 = AUX + 1;//incrementa ack1------------------------
          
          //pega o ack põe em AUX--------------------
          AUX1 = bts_clone[bts_clone.length() - 3];
          AUX1 += bts_clone[bts_clone.length() - 2];
          AUX1 += bts_clone[bts_clone.length() - 1];
          AUX = strToNbr(AUX1);

          if (AUX == NS2) //compara o ack da msg com NS1 do ultimo frame enviado-----------
          {
            
            //DADOS --------------------------
            do
            {
              com_byte = Serial.read();
              ult_recebido2 = NbrToBitS(com_byte, 8);
              delay(1000);
            }
            while (com_byte != start_end_Flag);

            ult_recebido2.remove((ult_recebido2.length() - 8), 8); //arranca a flag final

            AUX1 = ult_recebido2;
            AUX1.remove(0, 4); //arranca endereço

            //CRC ---------------------------------------------------------------------------------------
            CRC = (!(crcCmp(AUX1, DIV))) * 2;// mantém 0 quando ta td certo, 2 quando deu ruim
            
            if (CRC != 0) ACK1--;

          }
          else //caso do ult frame, que este mandou, não tenha sido mandado corretamente----------------------------------
          {
            alternar = !alternar;
            //realiza a opr normalmente porém, voltando uma instancia de NS e sem armazenar nada
            //DADOS --------------------------
            NS2 = AUX; //volta um NS

            do
            {
              com_byte = Serial.read();
              delay(1000);
            }
            while (com_byte != start_end_Flag);

            ult_recebido2.remove((ult_recebido2.length() - 8), 8); //arranca a flag final
            

          }
        }
        delay(1500);
        RX0_TX1 = !RX0_TX1;//volta ao modo falar----------------------------------------
        
        Serial.println(NS1);        
        Serial.println(ACK1);
        //Serial.println(NS2);
        //Serial.println(ACK2);
      }
    }
  }
  
}