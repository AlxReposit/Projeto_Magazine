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

// Firebase Data object para a função Stream
FirebaseData stream;

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

volatile bool dataChanged = false;


// - - - - - - - - - - - - - - - - - - -
//Variáveis do Processo
// - - - - - - - - - - - - - - - - - - -

//Tipo de Peça
bool Hab_V = 1;
bool Hab_P = 1;
bool Hab_M = 1;

//Tipo de Peça em cada Prateleira
byte Prat_V = 0;
byte Prat_P = 1;
byte Prat_M = 2;

//Operação
//'c' = Encher Magazine, Colocar Peças
//'r' = Esvaziar Magazine, Retirar Peças
char Operacao = 'c';

//Ordem de Retidada e Posicionamento de Peças
//'s' = Sequência
//'a' = Alternado
char Ordem = 's';

//Número de Ciclos de movimentos que o robô vai realizar
//entre 1 e 100 => ciclos definidos pelo número
//150  => sem ciclos (apenas o movimento de colocar ou de remover peças)
//200 => ciclos infinitos
//Máximo de ciclos: 100
byte n_ciclos = 0;

//------------------------------------------

void conectarWifi();
void conectarFirebase();
void receberDadosMagazine();
void receberDadosConfigsRTDB();
void enviarDadosConfigsRTDB();
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


 // ----------------------------------------------------------------------------
  // Assign the callback function for the long running token generation task 
  config.token_status_callback = tokenStatusCallback; // see addons/TokenHelper.h

  // Comment or pass false value when WiFi reconnection will control by your code or third party library e.g. WiFiManager
  Firebase.reconnectNetwork(true);

  // Since v4.4.x, BearSSL engine was used, the SSL buffer need to be set.
  // Large data transmission may require larger RX buffer, otherwise connection issue or data read time out can be occurred.
  fbdo.setBSSLBufferSize(2048 / * Rx buffer size in bytes from 512 - 16384 * /, 1024 / * Tx buffer size in bytes from 512 - 16384 * /);
  stream.setBSSLBufferSize(2048 / * Rx buffer size in bytes from 512 - 16384 * /, 1024 / * Tx buffer size in bytes from 512 - 16384 * /);

  Firebase.begin(&config, &auth);

  //Iniciar conexão Stream para monitorar mudanças em Start, Stop e Configs 
  //if (!Firebase.RTDB.beginStream(&stream, "/test/stream/data"))
  if (!Firebase.RTDB.stream(&stream, "/test/stream/data"))
    Serial.printf("sream begin error, %s\n\n", stream.errorReason().c_str());

  Firebase.RTDB.setStreamCallback(&stream, streamCallback, streamTimeoutCallback);
  // ---------------------------------------------------------------------------------
 
/*
  // Sign up
  if (Firebase.signUp(&config, &auth, "", "")) {
    Serial.println("OK");
    signupOK = true;
  } else {
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }

  // Assign the maximum retry of token generation
  config.max_token_generation_retry = 5;

  // Assign the callback function for the long running token generation task
  config.token_status_callback = tokenStatusCallback;  // see addons/TokenHelper.h

  // To refresh the token 5 minutes before expired
  config.signer.preRefreshSeconds = 5 * 60;

  // Use refresh token and force token to refresh immediately
  // Refresh token is required for id token refreshing.
  Firebase.setIdToken(&config, "" / * set id token to empty to refresh token immediately * /, 3600 / * expiry time * /, "<Refresh Token>");

  // Or refresh token manually
  // Firebase.refreshToken(&config);

  // Initialize the library with the Firebase authen and config 
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
  */
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


// - - - - TESTAR - - - - 

void receberDadosConfigsRTDB(){

}


struct configs {
  bool Hab_V;
  bool Hab_P;
  bool Hab_M;
  byte Prat_V;
  byte Prat_P;
  byte Prat_M;
  char Operacao;
  char Ordem;
  byte n_ciclos;
};

void enviarDadosConfigsRTDB(){
 //atualiza o banco de dados com as novas configurações do processo
  static String path = String();
  path = "configs";

  configs configAtualizada = {
    Hab_V,
    Hab_P,
    Hab_M,
    Prat_V,
    Prat_P,
    Prat_M,
    Operacao,
    Ordem,
    n_ciclos
  }

  if (Firebase.ready() && signupOK) {
    if (Firebase.RTDB.set(&fbdo, path, configAtualizada)) {
      Serial.println("PASSED");
      Serial.println("PATH: " + fbdo.dataPath());
      Serial.println("TYPE: " + fbdo.dataType());
    } else {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }
  }
}



// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
// Stream para monitorar mudanças 
//rtdb: colocar start, stop e novas_configs em um node "stream"
//monitorar mudanças em stream e atualizar start e stop, se mudar novas_configs, atualizar as vars com vase em "configs"
//Colcoar antes do setup

// - - - - - - - - - Callback - - - - - - - - - - 
void streamCallback(FirebaseStream data)
{
  Serial.printf("sream path, %s\nevent path, %s\ndata type, %s\nevent type, %s\n\n",
                data.streamPath().c_str(),
                data.dataPath().c_str(),
                data.dataType().c_str(),
                data.eventType().c_str());
  printResult(data); // see addons/RTDBHelper.h
  Serial.println();

  // This is the size of stream payload received (current and max value)
  // Max payload size is the payload size under the stream path since the stream connected
  // and read once and will not update until stream reconnection takes place.
  // This max value will be zero as no payload received in case of ESP8266 which
  // BearSSL reserved Rx buffer size is less than the actual stream payload.
  Serial.printf("Received stream payload size: %d (Max. %d)\n\n", data.payloadLength(), data.maxPayloadLength());

  // Due to limited of stack memory, do not perform any task that used large memory here especially starting connect to server.
  // Just set this flag and check it status later.
  dataChanged = true;
}

void streamTimeoutCallback(bool timeout)
{
  if (timeout)
    Serial.println("stream timed out, resuming...\n");

  if (!stream.httpConnected())
    Serial.printf("error code: %d, reason: %s\n\n", stream.httpCode(), stream.errorReason().c_str());
}

//colocar em loop()
void novosDados(){
 if (dataChanged)
  {
    dataChanged = false;
    // When stream data is available, do anything here...

   Serial.println();
  }
}



// - - - - - - - - - Sem Callback - - - - - - - - - - 
void iniciaStream(){
// In setup(), set the streaming path to "/test/data" and begin stream connection.
if (!Firebase.RTDB.stream(&fbdo, "/stream"))
{
  Serial.println(fbdo.errorReason());
}
}

void monitorarStream(){
// Place this in loop()
if (!Firebase.RTDB.readStream(&fbdo))
{
  Serial.println(fbdo.errorReason());
}

if (fbdo.streamTimeout())
{
  Serial.println("Stream timeout, resume streaming...");
  Serial.println();
}

if (fbdo.streamAvailable())
{
  if (fbdo.dataTypeEnum() == firebase_rtdb_data_type_integer)
    Serial.println(fbdo.to<int>());
  else if (fbdo.dataTypeEnum() == firebase_rtdb_data_type_float)
    Serial.println(fbdo.to<float>(), 5);
  else if (fbdo.dataTypeEnum() == firebase_rtdb_data_type_double)
    printf("%.9lf\n", fbdo.to<double>());
  else if (fbdo.dataTypeEnum() == firebase_rtdb_data_type_boolean)
    Serial.println(fbdo.to<bool>() ? "true" : "false");
  else if (fbdo.dataTypeEnum() == firebase_rtdb_data_type_string)
    Serial.println(fbdo.to<String>());
  else if (fbdo.dataTypeEnum() == firebase_rtdb_data_type_json)
  {
      FirebaseJson *json = fbdo.to<FirebaseJson *>();
      Serial.println(json->raw());
  }
  else if (fbdo.dataTypeEnum() == firebase_rtdb_data_type_array)
  {
      FirebaseJsonArray *arr = fbdo.to<FirebaseJsonArray *>();
      Serial.println(arr->raw());
  }
}
}

// For authentication except for legacy token, Firebase.ready() should be called repeatedly 
// in loop() to handle authentication tasks.
/*
void loop()
{
  if (Firebase.ready())
  {
    // Firebase is ready to use now.
    
  }
}*/






// - - - - - exemplo com callback - - - - - -

/*
// In setup(), set the streaming path to "/test/data" and begin stream connection.

if (!Firebase.RTDB.Stream(&fbdo, "/test/data", dataCallback))
{
  // Could not begin stream connection, then print out the error detail.
  Serial.println(fbdo.errorReason());
}

  
  // Global function that handles stream data
void dataCallback(FirebaseData &data)
{

  if (data.isStream())
  {

    if (data.callbackEventType() == firebase_callback_event_response_keepalive)
    {
      Serial.println("Stream keep alive event");
    }
    else if (data.callbackEventType() == firebase_callback_event_response_auth_revoked)
    {
      Serial.println("Stream auth revoked event");
    }
    else if (data.callbackEventType() == firebase_callback_event_response_timed_out)
    {
      Serial.println("Stream timed out");

      if (!stream.httpConnected())
        Serial_Printf("error code: %d, reason: %s\n\n", data.httpCode(), data.errorReason().c_str());
    }
    else if (data.callbackEventType() == firebase_callback_event_response_error)
       Serial.printf("Stream error, %s", data.errorReason());
    else if (data.callbackEventType() == firebase_callback_event_response_received)
    {
      Serial.println("Stream payload received");

      Serial_Printf("sream path, %s\nevent path, %s\ndata type, %s\nevent type, %s\n\n",
                    data.streamPath().c_str(),
                    data.dataPath().c_str(),
                    data.dataType().c_str(),
                    data.eventType().c_str());

      if (data.dataTypeEnum() == firebase_rtdb_data_type_integer)
        Serial.println(data.to<int>());
      else if (data.dataTypeEnum() == firebase_rtdb_data_type_float)
        Serial.println(data.to<float>(), 5);
      else if (data.dataTypeEnum() == firebase_rtdb_data_type_double)
        printf("%.9lf\n", data.to<double>());
      else if (data.dataTypeEnum() == firebase_rtdb_data_type_boolean)
        Serial.println(data.to<bool>() ? "true" : "false");
      else if (data.dataTypeEnum() == firebase_rtdb_data_type_string)
        Serial.println(data.to<String>());
      else if (data.dataTypeEnum() == firebase_rtdb_data_type_json)
      {
        FirebaseJson *json = data.to<FirebaseJson *>();
        Serial.println(json->raw());
      }
      else if (data.dataTypeEnum() == firebase_rtdb_data_type_array)
      {
        FirebaseJsonArray *arr = data.to<FirebaseJsonArray *>();
        Serial.println(arr->raw());
      }

      Serial.println();
    }
  }

}

// For authentication except for legacy token, Firebase.ready() should be called repeatedly 
// in loop() to handle authentication tasks.

void loop()
{
  if (Firebase.ready())
  {
    // Firebase is ready to use now.

  }
}
*/
