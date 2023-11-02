bool enviandoDadosMagazine = false;

void receberDadosMagazine();
void enviarDadosMagazine();
// envia a mensagem como <{----{----{----> (todos de uma vez)
// < = início da mensagem
// { = inicio da próxima prateleira
// - = tipo da peça:
//   M = metálica
//   P = preta
//   V = vermalha
//   - ou qualquer outro = vazio
// > = fim da mensagem

// envia a mensagem como <{1[1(-> (uma posição de cada vez)
// < = início da mensagem
// { = índice da prateleira
// [ = índice da posição
// ( = tipo da peça:
//   M = metálica
//   P = preta
//   V = vermalha
//   - ou qualquer outro = vazio
// > = fim da mensagem

//char msg[] = "<{----{----{---->";

char magazine[3][5] = {
  "VVVV",
  "----",
  "MMMM"
};

double tempo_atual = -15000;

void setup() {
  pinMode(13, INPUT_PULLUP);
  pinMode(12, INPUT_PULLUP);
  pinMode(11, INPUT_PULLUP);
  pinMode(10, INPUT_PULLUP);
  pinMode(9, INPUT_PULLUP);
  pinMode(8, INPUT_PULLUP);
  Serial.begin(9600);
}

void loop() {

  /*if(millis() > (tempo_atual + 15000)){
    enviarDadosMagazine(0,3);
    tempo_atual = millis();
    }*/

  receberDadosMagazine();

  /*magazine[0] = digitalRead(13);
    magazine[1] = digitalRead(12);
    magazine[2] = digitalRead(11);
    magazine[3] = digitalRead(10);
    magazine[4] = digitalRead(9);
    magazine[5] = digitalRead(8);
  */
}

void atualizaMagazine() {
}

void atualizaMagazine(int prat, int pos, char valor) {
  magazine[prat][pos] = valor;
}

//Gera uma mensagem com as informações da magazine toda e envia para o Esp32
void enviarDadosMagazine() {
  enviandoDadosMagazine = true;
  String msg = String();

  msg = "<";
  for (int prat = 0; prat < 3; prat++) {
    for (int pos = -1; pos < 4; pos++) {
      if (pos != -1) {
        msg += magazine[prat][pos];
      } else {
        msg += '{';
      }
    }
  }

  msg += '>';
  Serial.println(msg);
  enviandoDadosMagazine = false;
}

//Gera uma mensagem apenas as informações da magazine que mudaram e envia para o Esp32
void enviarDadosMagazine(byte prateleira, byte posicao) {
  enviandoDadosMagazine = true;
  /*String msg = String();
    msg = "<";
    msg += prateleira;
    msg += posicao;
    msg += magazine[prateleira][posicao];
    msg += '>';
    Serial.println(msg);*/

  Serial.print('<');
  Serial.print('{');
  Serial.print(prateleira);
  Serial.print('[');
  Serial.print(posicao);
  Serial.print('(');
  Serial.print(magazine[prateleira][posicao]);
  Serial.println('>');

  enviandoDadosMagazine = false;
}


void receberDadosMagazine() {
  static char inicioLeitura = '<';
  static char fimLeitura = '>';
  static char marcadorPrateleira = '{';
  static char marcadorPosicao = '[';
  static char marcadorPeca = '(';
  static byte setorizacao = 0;
  static bool leituraEmProgresso = false;
  static byte pos = 0;
  static byte prat = 0;
  static String pos_c = "";
  static String prat_c = "";
  static char peca = '-';
  char leituraSerialArduino;

  if (Serial.available() > 0) {
    leituraSerialArduino = Serial.read();

    if (leituraEmProgresso == true) {
      if (leituraSerialArduino != fimLeitura) {

        if (leituraSerialArduino == marcadorPrateleira) {
          setorizacao = 1;
        } else if (leituraSerialArduino == marcadorPosicao) {
          setorizacao = 2;
        } else if (leituraSerialArduino == marcadorPeca) {
          setorizacao = 3;
        } else if (setorizacao == 1) {
          prat_c += leituraSerialArduino;
        } else if (setorizacao == 2) {
          prat = prat_c.toInt();
          pos_c += leituraSerialArduino;
        } else if (setorizacao == 3) {
          pos = pos_c.toInt();
          magazine[prat][pos] = leituraSerialArduino;
          
          Serial.print("prateleira: ");
          Serial.println(prat);
          Serial.print("posição: ");
          Serial.println(pos);
          Serial.print("peça: ");
          Serial.println(magazine[prat][pos]);

          prat_c = "";
          pos_c = "";
        } /*
        {
          if (leituraSerialArduino == marcadorPrateleira) {
            setorizacao = 1;
          } else if (leituraSerialArduino == marcadorPosicao) {
            setorizacao = 2;
          } else if (leituraSerialArduino == marcadorPeca) {
            setorizacao = 3;
          } else if (setorizacao == 1) {
            prat = (byte)leituraSerialArduino - '0';
          } else if (setorizacao == 2) {
            pos = byte(leituraSerialArduino - '0');
          } else if (setorizacao == 3) {
            magazine[prat][pos] = leituraSerialArduino;
            Serial.print("prateleira: ");
            Serial.println(prat);
            Serial.print("posição: ");
            Serial.println(pos);
            Serial.print("peça: ");
            Serial.println(magazine[prat][pos]);
          }
        }<{23[34(M>*/

      } else if (leituraSerialArduino == fimLeitura) {
        //chegou no fim da leitura
        leituraEmProgresso = false;
        pos = 0;

      }
    } else if (leituraSerialArduino == inicioLeitura) {
      leituraEmProgresso = true;
    }
  } else {
  }
}





//---------------------------------------------- FAZER DEPOIS ----------------------------------------------

/*

  //Verifica se o Esp32 mandou sinal na estrada do arduino sinalizando que há dados a serem lidos no serial
  void listenerReceberDadosEsp32() {
  }

  //Lê os dados que o Esp32 enviou ao arduino
  //Atualiza as informações de configuração do processo

  //<v1>
  //<p0>
  //<m1>

  //<V1>
  //<P2>
  //<M3>

  //<pe> <pr>
  //<oa> <os>
  //<c4>


  bool Hab_V = 1;
  bool Hab_P = 1;
  bool Hab_M = 1;

  byte Prat_V = 1;
  byte Prat_P = 2;
  byte Prat_M = 3;

  //Operação
  //'e' = Encher Magazine, Colocar Peças
  //'r' = Esvaziar Magazine, Retirar Peças
  char Operacao = 'e';

  //Ordem de Retidada e Posicionamento de Peças
  //'s' = Sequência
  //'a' = Alternado
  char Ordem = 'a';

  int ciclo_atual = 0;

  void receberDadosEsp32() {
  static char inicioLeitura = '<';
  static char fimLeitura = '>';
  static char marcadorPrateleira = '{';
  static bool leituraEmProgresso = false;
  static int pos = 0;
  static int prateleira = 0;
  char leituraSerial;

  if (Serial.available() > 0 && enviandoDadosMagazine == false) {
    leituraSerial = Serial.read();


    if (leituraEmProgresso == true) {
      if (leituraSerial != fimLeitura) {

        if (leituraSerial == marcadorPrateleira) {
          magazineRecebida[prateleira][pos] = '\0';
          prateleira++;
          pos = 0;
        } else {
          magazineRecebida[prateleira][pos] = leituraSerial;
          pos++;
        }

      } else if (leituraSerial == fimLeitura) {
        //chegou no fim da leitura
        //magazine[prateleira][pos] = '\0';
        leituraEmProgresso = false;
        pos = 0;
        novosDados = false;

        atualizaMagazineRTDB();
      }
    } else if (leituraSerial == inicioLeitura) {
      leituraEmProgresso = true;
    }
  }
  }

*/