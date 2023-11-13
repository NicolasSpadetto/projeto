
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
String buffer[2] = {"110000", "1100111"};
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
  delay(1000);

  if (RX0_TX1) //fala---------------------------------------------------------
  {
    if (CRC == 0)
    {
      
      Serial.write(start_end_Flag);


      //ADDRESS---------------------
      
      bts_clone = p_addr;
      bts_clone += my_addr;
      
      com_byte = strToNbr(bts_clone);
      
      
      Serial.write(com_byte);//<--ADDRESS

      //CONTROLL---------------------
    
      bts_clone = '0';
      bts_clone += NbrToBitS(NS, 3);
      bts_clone += '0';
      bts_clone += NbrToBitS(ACK, 3);
      com_byte = strToNbr(bts_clone);
      
      


      Serial.write(com_byte);//<--CONTROLL

      //DADOS---------------------

      //NS par envia [0], NS ímpar

      if ((NS % 2) == 0)
      {
        com_byte = strToNbr(buffer[0]);
        bts_clone = buffer[0];
        
      }
      else
      { 
        com_byte = strToNbr(buffer[1]);
        bts_clone = buffer[1];
        
      }
      NS++;
      
        
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
    
  }
  else if(Serial.available() > 0)//ouve--------------------------------------------
  {    
    delay(2000);
    //Serial.println("meu deus foi");
    com_byte = Serial.read();

    if (com_byte == start_end_Flag)
    {
      //ADDRESS---------------------
      //Serial.println("meu deus foi");

      delay(1000);
      com_byte = Serial.read();
      delay(1000);

      AUX1 = NbrToBitS(com_byte, 8);      
      
      bts_clone = AUX1[0];
      bts_clone += AUX1[1];
      bts_clone += AUX1[2];
      bts_clone += AUX1[3];



      if (bts_clone == my_addr)// tem q trocar caso mude o secundário ------------------------------------------------------
      {
        

        AUX1.remove(0, 4);//tira o destinatário

        ult_recebido = AUX1; // pega o remetente e salva-----------
              
        //CONTROLL---------------------
        com_byte = Serial.read();
        bts_clone = NbrToBitS(com_byte, 8);
        delay(1000);

        AUX1 = bts_clone[1];
        AUX1 += bts_clone[2];
        AUX1 += bts_clone[3];

        AUX = strToNbr(AUX1);// segura o NS da mensagem
        
        ACK = AUX + 1;//incrementa ack1------------------------
        

        //pega o ack põe em AUX
        AUX1 = bts_clone[bts_clone.length() - 3];
        AUX1 += bts_clone[bts_clone.length() - 2];
        AUX1 += bts_clone[bts_clone.length() - 1];
        AUX = strToNbr(AUX1);
        
        
        if (NS == AUX) //compara o ack da msg com NS do ultimo frame enviado--------------------
        {

          //DADOS --------------------------
          do
          {
            com_byte = Serial.read();
            ult_recebido = NbrToBitS(com_byte);
            delay(1000);
          }
          while (com_byte != start_end_Flag);

          ult_recebido.remove((ult_recebido.length() - 8), 8); //arranca a flag final

          AUX1 = ult_recebido;
          AUX1.remove(0, 4); //arranca endereço
            
          //CRC ---------------------------------------------------------------------------------------
          CRC = (!(crcCmp(AUX1, DIV)));// mantém 0 quando ta td certo, 1 quando deu ruim

          if (CRC != 0) ACK--;

        }
        else //caso do ult frame, que este mandou, não tenha sido mandado corretamente----------------------------------
        {
          Serial.println("ack ruim");
          //realiza a opr normalmente porém, voltando uma instancia de NS e sem armazenar nada
          //DADOS --------------------------
          NS = AUX; //volta um NS

          do
          {
            com_byte = Serial.read();
            delay(1000);
          }
          while (com_byte != start_end_Flag);

          ult_recebido.remove((ult_recebido.length() - 8), 8); //arranca a flag final
            
        }

        
        delay(1500);
        RX0_TX1 = !RX0_TX1;//volta ao modo falar---------------------------------------- 
        Serial.println(NS);
        Serial.println(ACK);     
      }
    }

  }
  
}