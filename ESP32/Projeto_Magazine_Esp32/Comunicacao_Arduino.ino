char leituraSerial;
static bool leituraMagazineEmProgresso = false;
static bool leituraProcessoEmProgresso = false;
static const char inicioLeituraMagazine = '<';
static const char inicioLeituraProcesso = '[';

//Observa quando o arduino envia dados por serial - Loop
void observadorComunicacaoArduino() {
  if (Serial2.available() > 0) {
    leituraSerial = Serial2.read();

    if ((!leituraProcessoEmProgresso && leituraSerial == inicioLeituraMagazine) || leituraMagazineEmProgresso) {
      receberDadosMagazine();
      Serial.println("leitura mag");
    }

    if ((!leituraMagazineEmProgresso && leituraSerial == inicioLeituraProcesso) || leituraProcessoEmProgresso) {
      receberDadosProcesso();
    }
  }

  /*if (processo_em_andamento == false && novosDadosConfigs == true) {
    novosDadosConfigs = false;
    enviarDadosConfigsArduino();
  }*/
}

void receberDadosMagazine() {
  //recebe a mensagem como: <0;0;V> <prateleira,posição,tipo_peça>
  //static char inicioLeitura = '<';
  static char fimLeitura = '>';
  static char separador = ';';
  static byte setorizacao = 0;

  static byte pos = 0;
  static byte prat = 0;
  static char peca[10] = "---------";

  if (leituraMagazineEmProgresso == true) {
    if (leituraSerial != fimLeitura) {

      if (true) {
        if (leituraSerial == separador) {
          setorizacao++;
        } else if (setorizacao == 0) {
          prat = (byte)leituraSerial - '0';
        } else if (setorizacao == 1) {
          pos = byte(leituraSerial - '0');
        } else if (setorizacao == 2) {
          magazine[prat][pos] = leituraSerial;
          Serial.print("prateleira: ");
          Serial.println(prat);
          Serial.print("posição: ");
          Serial.println(pos);
          Serial.print("peça: ");
          Serial.println(magazine[prat][pos]);

          switch (magazine[prat][pos]) {
            case 'V':
              strcpy(peca, "vermelho");
              break;
            case 'P':
              strcpy(peca, "preto");
              break;
            case 'M':
              strcpy(peca, "metal");
              break;
            default:
              strcpy(peca, "vazio");
              break;
          }

          Serial.print("envia dados: ");
          Serial.print(prat);
          Serial.print("-");
          Serial.print(pos);
          Serial.print("-");
          Serial.println(peca);

          enviarDadosMagazineRTDB(prat, pos, peca);

          Serial.println("Envio de Dados OK");

          setorizacao = 0;
        }
      }
    } else if (leituraSerial == fimLeitura) {
      //chegou no fim da leitura
      leituraMagazineEmProgresso = false;
      pos = 0;
    }
  } else if (leituraSerial == inicioLeituraMagazine) {
    leituraMagazineEmProgresso = true;
  }
}

// void receberDadosMagazine() {
//   //recebe a mensagem como: <{0[0(V> <{prateleira[posição(tipo_peça>
//   //static char inicioLeitura = '<';
//   static char fimLeitura = '>';
//   static char marcadorPrateleira = '{';
//   static char marcadorPosicao = '[';
//   static char marcadorPeca = '(';
//   static byte setorizacao = 0;

//   static byte pos = 0;
//   static byte prat = 0;
//   static char peca[10] = "---------";

//   if (leituraMagazineEmProgresso == true) {
//     if (leituraSerial != fimLeitura) {

//       if (true) {
//         if (leituraSerial == marcadorPrateleira) {
//           setorizacao = 1;
//         } else if (leituraSerial == marcadorPosicao) {
//           setorizacao = 2;
//         } else if (leituraSerial == marcadorPeca) {
//           setorizacao = 3;
//         } else if (setorizacao == 1) {
//           prat = (byte)leituraSerial - '0';
//         } else if (setorizacao == 2) {
//           pos = byte(leituraSerial - '0');
//         } else if (setorizacao == 3) {
//           magazine[prat][pos] = leituraSerial;
//           Serial.print("prateleira: ");
//           Serial.println(prat);
//           Serial.print("posição: ");
//           Serial.println(pos);
//           Serial.print("peça: ");
//           Serial.println(magazine[prat][pos]);

//           switch (magazine[prat][pos]) {
//             case 'V':
//               strcpy(peca, "vermelho");
//               break;
//             case 'P':
//               strcpy(peca, "preto");
//               break;
//             case 'M':
//               strcpy(peca, "metal");
//               break;
//             default:
//               strcpy(peca, "vazio");
//               break;
//           }

//           Serial.print("envia dados: ");
//           Serial.print(prat);
//           Serial.print("-");
//           Serial.print(pos);
//           Serial.print("-");
//           Serial.println(peca);

//           enviarDadosMagazineRTDB(prat, pos, peca);

//           Serial.println("Envio de Dados OK");

//           setorizacao = 0;
//         }
//       }
//     } else if (leituraSerial == fimLeitura) {
//       //chegou no fim da leitura
//       leituraMagazineEmProgresso = false;
//       pos = 0;
//     }
//   } else if (leituraSerial == inicioLeituraMagazine) {
//     leituraMagazineEmProgresso = true;
//   }
// }

/* Para mais prateleiras e posições do que está sendo usado:
void receberDadosMagazine() {
  static char inicioLeitura = '<';
  static char fimLeitura = '>';
  static char marcadorPrateleira = '{';
  static char marcadorPosicao = '[';
  static char marcadorPeca = '(';
  static byte setorizacao = 0;
  static bool leituraMagazineEmProgresso = false;
  static byte pos = 0;
  static byte prat = 0;
  static String pos_c = "";
  static String prat_c = "";
  static char peca = '-';
  char leituraSerial;

  if (Serial.available() > 0) {
    leituraSerial = Serial.read();

    if (leituraMagazineEmProgresso == true) {
      if (leituraSerial != fimLeitura) {

        if (leituraSerial == marcadorPrateleira) {
          setorizacao = 1;
        } else if (leituraSerial == marcadorPosicao) {
          setorizacao = 2;
        } else if (leituraSerial == marcadorPeca) {
          setorizacao = 3;
        } else if (setorizacao == 1) {
          prat_c += leituraSerial;
        } else if (setorizacao == 2) {
          prat = prat_c.toInt();
          pos_c += leituraSerial;
        } else if (setorizacao == 3) {
          pos = pos_c.toInt();
          magazine[prat][pos] = leituraSerial;
          
          Serial.print("prateleira: ");
          Serial.println(prat);
          Serial.print("posição: ");
          Serial.println(pos);
          Serial.print("peça: ");
          Serial.println(magazine[prat][pos]);

          prat_c = "";
          pos_c = "";
        }

      } else if (leituraSerial == fimLeitura) {
        //chegou no fim da leitura
        leituraMagazineEmProgresso = false;
        pos = 0;

      }
    } else if (leituraSerial == inicioLeitura) {
      leituraMagazineEmProgresso = true;
    }
  } else {
  }
}
*/

//Recebe os dados de monitoramento do processo para enviar para o site
void receberDadosProcesso() {
  //recebe a mensagem como: [1,1,100]
  // static const char inicioLeitura = '[';
  static const char fimLeitura = ']';
  static const char separador = ',';
  static byte setorizacao = 0;
  static char str_ciclos[4] = "---";
  static char *ponteiro_ciclos;

  if (leituraProcessoEmProgresso == true) {
    if (leituraSerial != fimLeitura) {
      if (leituraSerial == separador) {
        setorizacao++;
      } else {
        switch (setorizacao) {
          case 0:
            processo_em_andamento = byte(leituraSerial - '0');
            break;
          case 1:
            robo_em_movimento = byte(leituraSerial - '0');
            break;
          case 2:
            if (ponteiro_ciclos) {
              *ponteiro_ciclos = leituraSerial;
              ponteiro_ciclos++;
            }
            break;
        }
      }
    } else {
      //Fim da leitura
      ciclo_atual = atoi(str_ciclos);
      leituraProcessoEmProgresso = false;
      enviarDadosProcessoRTDB();
    }
  } else if (leituraSerial == inicioLeituraProcesso) {
    leituraProcessoEmProgresso = true;
    setorizacao = 0;
    strcpy(str_ciclos, "---");
    ponteiro_ciclos = str_ciclos;
  }
}

//
void conversaoPrateleiras() {
  // char valor[10];
  char *valor;

  for (byte i = 0; i < 3; i++) {
    switch (i) {
      case 0:
        valor = Prat_0;
        break;
      case 1:
        valor = Prat_1;
        break;
      case 2:
        valor = Prat_2;
        break;
    }

    if (strcmp(valor, "v") == 0) {
      Prat_V = i;

    } else if (strcmp(valor, "p") == 0) {
      Prat_P = i;

    } else if (strcmp(valor, "m") == 0) {
      Prat_M = i;
    }
  }

  Serial.println("");
}

//
void enviarDadosConfigsArduino() {
  //Envia mensagem como: [1,1,1,0,1,2,c,s,10]
  //                     [0,0,0,2,0,1,r,a,200]
  Serial2.print('[');
  Serial2.print(Hab_V);
  Serial2.print(',');
  Serial2.print(Hab_P);
  Serial2.print(',');
  Serial2.print(Hab_M);
  Serial2.print(',');
  Serial2.print(Prat_V);
  Serial2.print(',');
  Serial2.print(Prat_P);
  Serial2.print(',');
  Serial2.print(Prat_M);
  Serial2.print(',');
  Serial2.print(Tipo_Movimento);
  Serial2.print(',');
  Serial2.print(Tipo_Sequencia);
  Serial2.print(',');
  Serial2.print(n_ciclos);
  Serial2.print(']');

  /*Serial.print('[');
  Serial.print(Hab_V);
  Serial.print(',');
  Serial.print(Hab_P);
  Serial.print(',');
  Serial.print(Hab_M);
  Serial.print(',');
  Serial.print(Prat_0);
  Serial.print(',');
  Serial.print(Prat_1);
  Serial.print(',');
  Serial.print(Prat_2);
  Serial.print(',');
  Serial.print(Tipo_Movimento);
  Serial.print(',');
  Serial.print(Tipo_Sequencia);
  Serial.print(',');
  Serial.print(n_ciclos);
  Serial.print(']');*/
}

//
void enviarStartStopArduino(bool btnstart) {
  if (btnstart && start) {
    enviarStartArduino();

  } else if (!btnstart && stop) {
    Serial2.print('?');
    Serial.print('?');

    if (Firebase.ready()) {
      if (!Firebase.RTDB.setBool(&fbdo, "stream/stop", false)) {
        Serial.println(fbdo.errorReason());
      }
    }
  }
}


void enviarStartArduino() {
  if (processo_em_andamento == false) {
    //Envia Configurações do Processo
    enviarDadosConfigsArduino();

    //Inicia Processo
    Serial2.print('!');
    Serial.print('!');
  }

  if (Firebase.ready()) {
    if (!Firebase.RTDB.setBool(&fbdo, "stream/start", false)) {
      Serial.println(fbdo.errorReason());
    }
  }
}