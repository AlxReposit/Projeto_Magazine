#include <Arduino.h>
#include <WiFi.h>
#include <Firebase_ESP_Client.h>

// Provide the token generation process info.
#include <addons/TokenHelper.h>

// Provide the RTDB payload printing info and other helper functions.
#include <addons/RTDBHelper.h>

/* 1. Define the WiFi credentials */
#define WIFI_SSID "Galaxy a."
#define WIFI_PASSWORD "123123000"

/** 2. Define the API key
 *
 * The API key (required) can be obtained since you created the project and set up
 * the Authentication in Firebase console. Then you will get the API key from
 * Firebase project Web API key in Project settings, on General tab should show the
 * Web API Key.
 *
 * You may need to enable the Identity provider at https://console.cloud.google.com/customer-identity/providers
 * Select your project, click at ENABLE IDENTITY PLATFORM button.
 * The API key also available by click at the link APPLICATION SETUP DETAILS.
 *
 */
#define API_KEY "AIzaSyCS2IckjSds29v8UUomI1aYt5mPhFlWB9Y"

//Informações para Autenticação
/* 3. Define the user Email and password that already registerd or added in your project */
#define AUTH_EMAIL "esp32@magazine.com"
#define AUTH_PASSWORD "TW9uaXRvck1hZ2F6aW5lRXNw"

/* 4. If work with RTDB, define the RTDB URL */
#define DATABASE_URL "https://teste-app-monitor-default-rtdb.firebaseio.com"

#define Tx2 17             // Comunicação Serial
#define Rx2 16             // nos pinos G16 e G17
#define pinoNovosDados 15  // recebe sinal do arduino para ler o serial e receber novos dados

/** 5. Define the database secret (optional)
 *
 * This database secret needed only for this example to modify the database rules
 *
 * If you edit the database rules yourself, this is not required.
 #define DATABASE_SECRET "DATABASE_SECRET"
 */

/* 6. Define the Firebase Data object */
FirebaseData fbdo;

/* 7. Define the FirebaseAuth data for authentication data */
FirebaseAuth auth;

/* 8. Define the FirebaseConfig data for config data */
FirebaseConfig config;

bool signupOK = false;

// recebe a mensagem como <{----{----{---->
// < = início da mensagem
// { = inicio da próxima prateleira
// - = tipo da peça:
//   m = metálica
//   p = preta
//   v = vermalha
//   - ou qualquer outro = vazio
// > = fim da mensagem

char charDadosRecebidos[32];
bool novosDados = false;
char magazine[3][5] = {
  "----",
  "----",
  "----"
};
char magazineRecebida[3][5] = {
  "----",
  "----",
  "----"
};

void conectarWifi();
void conectarFirebase();
void receberDadosMagazine();
void enviarDadosRTDB();
void setMagazineRTDB(int, int, String);

void conectarWifi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.print("Conectando ao WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }

  Serial.println();
  Serial.print("Conectado ao IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();
}

void conectarFirebase() {
  // Assign the api key
  config.api_key = API_KEY;

  // Assign the user sign in credentials
  auth.user.email = AUTH_EMAIL;
  auth.user.password = AUTH_PASSWORD;

  // Assign the RTDB URL
  config.database_url = DATABASE_URL;

  /*
  Firebase.reconnectNetwork(true);

  // Since v4.4.x, BearSSL engine was used, the SSL buffer need to be set.
  // Large data transmission may require larger RX buffer, otherwise connection issue or data read time out can be occurred.
  fbdo.setBSSLBufferSize(4096 / * Rx buffer size in bytes from 512 - 16384 * /, 1024 / * Tx buffer size in bytes from 512 - 16384 * /);

  fbdo.setResponseSize(4096);
*/
  // Sign up
  if (Firebase.signUp(&config, &auth, "", "")) {
    Serial.println("OK");
    signupOK = true;
  } else {
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }

  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback;  // see addons/TokenHelper.h

  // Assign the maximum retry of token generation
  config.max_token_generation_retry = 5;

  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback;  // see addons/TokenHelper.h

  // To refresh the token 5 minutes before expired
  config.signer.preRefreshSeconds = 5 * 60;

  // Use refresh token and force token to refresh immediately
  // Refresh token is required for id token refreshing.
  Firebase.setIdToken(&config, "" /* set id token to empty to refresh token immediately */, 3600 /* expiry time */, "<Refresh Token>");

  // Or refresh token manually
  // Firebase.refreshToken(&config);

  /* Initialize the library with the Firebase authen and config */
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
}

void setup() {
  //Serial.begin(115200, SERIAL_8N1, Rx0, Tx0);
  Serial.begin(115200);                       // Serial PC
  Serial2.begin(9600, SERIAL_8N1, Rx2, Tx2);  // Serial Arduino

  conectarWifi();
  conectarFirebase();

  // Getting the user UID might take a few seconds
  Serial.println("Getting User UID");
  while ((auth.token.uid) == "") {
    Serial.print('.');
    delay(1000);
  }

  // Print user UID
  uid = auth.token.uid.c_str();
  Serial.print("User UID: ");
  Serial.print(uid);

  //enviarDadosRTDB();
}

void loop() {
  receberDadosMagazine();
}
/*
void verificarNovosDados() {
  if (digitalRead(pinoNovosDados) == LOW) {
    novosDados == true;
  }

  if (novosDados == true) {
    receberDadosMagazine();
  }
}*/
/*
void receberDadosTodaMagazine() {
  static char inicioLeitura = '<';
  static char fimLeitura = '>';
  static char marcadorPrateleira = '{';
  static bool leituraEmProgresso = false;
  static int pos = 0;
  static int prateleira = 0;
  char leituraSerialArduino;

  if (Serial2.available() > 0 /* && novosDados == true* /) {
    leituraSerialArduino = Serial2.read();

    if (leituraEmProgresso == true) {
      if (leituraSerialArduino != fimLeitura) {

        if (leituraSerialArduino == marcadorPrateleira) {
          magazineRecebida[prateleira][pos] = '\0';
          prateleira++;
          pos = 0;
        } else {
          magazineRecebida[prateleira][pos] = leituraSerialArduino;
          pos++;
        }

      } else if (leituraSerialArduino == fimLeitura) {
        //chegou no fim da leitura
        //magazine[prateleira][pos] = '\0';
        leituraEmProgresso = false;
        pos = 0;
        novosDados = false;

        atualizaMagazineRTDB();
      }
    } else if (leituraSerialArduino == inicioLeitura) {
      leituraEmProgresso = true;
    }
  }
}*/

void receberDadosMagazine() {
  //recebe a mensagem como: <{0[0(V> <{prateleira[posição(tipo_peça>
  static char inicioLeitura = '<';
  static char fimLeitura = '>';
  static char marcadorPrateleira = '{';
  static char marcadorPosicao = '[';
  static char marcadorPeca = '(';
  static byte setorizacao = 0;
  static bool leituraEmProgresso = false;
  static byte pos = 0;
  static byte prat = 0;
  char leituraSerialArduino;
  static String peca = String();

  if (Serial2.available() > 0 /* && novosDados == true*/) {
    leituraSerialArduino = Serial2.read();

    if (leituraEmProgresso == true) {
      if (leituraSerialArduino != fimLeitura) {

        if (true) {
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

            switch (magazine[prat][pos]) {
              case 'V':
                peca = "vermelho";
                break;
              case 'P':
                peca = "preto";
                break;
              case 'M':
                peca = "metal";
                break;
              default:
                peca = "vazio";
                break;
            }

            setMagazineRTDB(prat, pos, peca);
            setorizacao = 0;
          }
        }
      } else if (leituraSerialArduino == fimLeitura) {
        //chegou no fim da leitura
        leituraEmProgresso = false;
        pos = 0;
      }
    } else if (leituraSerialArduino == inicioLeitura) {
      leituraEmProgresso = true;
    }
  }
}

/* Para mais prateleiras e posições do que está sendo usado:
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
        }

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
*/
/*
void atualizaMagazineRTDB() {
  //comparar magazine recebida/atualizada com a atual e atualizar no banco de dados apenas as posições que diferem
  //for (int prat = 0; prat < (sizeof(magazineRecebida) / sizeof(magazineRecebida[0])); prat++) {
  //for (int pos = 0; pos < (sizeof(magazineRecebida[prat]) / sizeof(magazineRecebida[prat][0])); prat++) {
  static String tipo_peca = String();

  for (int prat = 0; prat < 3; prat++) {
    for (int pos = 0; pos < 5; pos++) {

      if (magazineRecebida[prat][pos] != magazine[prat][pos]) {
        magazine[prat][pos] = magazineRecebida[prat][pos];

        switch (magazine[prat][pos]) {
          case 'v':
            tipo_peca = "vermelho";
            break;
          case 'p':
            tipo_peca = "preto";
            break;
          case 'm':
            tipo_peca = "metal";
            break;
          default:
            tipo_peca = "vazio";
            break;
        }

        setMagazineRTDB(prat, pos, tipo_peca);
      }
    }
  }
}*/

void setMagazineRTDB(int prateleira, int posicao, String novoValor = String()) {
  //atualiza o banco de dados apenas na posição recebida
  static String path = String();
  path = "magazine/";
  path += "prateleira_";
  path += prateleira;
  path += "/";
  path += posicao;

  if (Firebase.ready() && signupOK) {
    if (Firebase.RTDB.setString(&fbdo, path, novoValor)) {
      Serial.println("PASSED");
      Serial.println("PATH: " + fbdo.dataPath());
      Serial.println("TYPE: " + fbdo.dataType());
    } else {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }
  }
}
/*
void enviarDadosRTDB() {
  //atualiza todos os dados no banco de dados
  static String path_str = String();
  static String path_pos = String();
  static String tipo_peca = String();

  if (Firebase.ready() && signupOK) {
    //Envia dados da magazine recebidos do arduino via Serial
    for (int prateleira = 0; prateleira < 3; prateleira++) {
      path_str = "magazine/prateleira_";
      path_str += (prateleira + 1);
      path_str += "/";

      for (int pos = 0; pos < 4; pos++) {
        path_pos = path_str;
        path_pos += pos;

        switch (magazine[prateleira][pos]) {
          case 'v':
            tipo_peca = "vermelho";
            break;
          case 'p':
            tipo_peca = "preto";
            break;
          case 'm':
            tipo_peca = "metal";
            break;
          default:
            tipo_peca = "vazio";
            break;
        }

        if (Firebase.RTDB.setString(&fbdo, path_pos, tipo_peca)) {
          Serial.println("PASSED");
          Serial.println("PATH: " + fbdo.dataPath());
          Serial.println("TYPE: " + fbdo.dataType());
        } else {
          Serial.println("FAILED");
          Serial.println("REASON: " + fbdo.errorReason());
        }
      }
    }
  }
}*/
