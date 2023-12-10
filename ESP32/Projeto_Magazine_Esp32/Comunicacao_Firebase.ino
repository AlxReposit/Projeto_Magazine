//Funções de Callback quando ocorre um evento de Wifi
void WiFiConectadoRoteador(WiFiEvent_t event, WiFiEventInfo_t info){
  Serial.println("Conectado ao Roteador");
}

void WiFiConectadoIP(WiFiEvent_t event, WiFiEventInfo_t info){
  Serial.print("Conectado ao IP: ");
  Serial.println(WiFi.localIP());
  reconectarFirebase = true;
}

void WiFiDesconectado(WiFiEvent_t event, WiFiEventInfo_t info){
  Serial.println("Desconectado do Wifi");
  Serial.print("Motivo: ");
  Serial.println(info.wifi_sta_disconnected.reason);
  Serial.println("Tentando Reconexão");
  //WiFi.reconnect();
  ESP.restart();
}

//Inicia Conexão ao WiFi
void conectarWifi() {

  WiFi.onEvent(WiFiConectadoRoteador, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_CONNECTED);
  WiFi.onEvent(WiFiConectadoIP, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_GOT_IP);
  WiFi.onEvent(WiFiDesconectado, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_DISCONNECTED);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.print("Conectando ao WiFi");
  // while (WiFi.status() != WL_CONNECTED) {
  //   Serial.print(".");
  //   delay(300);
  // }

  // Serial.println();
  // Serial.print("Conectado ao IP: ");
  // Serial.println(WiFi.localIP());
  // Serial.println();
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
  config.token_status_callback = tokenStatusCallback;  // see addons/TokenHelper.h

  // Comment or pass false value when WiFi reconnection will control by your code or third party library e.g. WiFiManager
  Firebase.reconnectNetwork(true);

  // Since v4.4.x, BearSSL engine was used, the SSL buffer need to be set.
  // Large data transmission may require larger RX buffer, otherwise connection issue or data read time out can be occurred.
  fbdo.setBSSLBufferSize(2048 /*Rx buffer size in bytes from 512 - 16384 */, 1024 /*Tx buffer size in bytes from 512 - 16384 */);
  stream_start.setBSSLBufferSize(2048, 1024);
  stream_stop.setBSSLBufferSize(2048, 1024);
  stream_configs.setBSSLBufferSize(2048, 1024);

  Firebase.begin(&config, &auth);

  stream_start.keepAlive(5, 5, 1);
  stream_stop.keepAlive(5, 5, 1);
  stream_configs.keepAlive(5, 5, 1);

  //Iniciar conexão Stream para monitorar mudanças em Start, Stop e Configs
  if (!Firebase.RTDB.beginStream(&stream_start, "stream/start")) {
    Serial.println(stream_start.errorReason());
  }

  if (!Firebase.RTDB.beginStream(&stream_stop, "stream/stop")) {
    Serial.println(stream_stop.errorReason());
  }

  if (!Firebase.RTDB.beginStream(&stream_configs, "stream/novas_configs")) {
    Serial.println(stream_configs.errorReason());
  }
}

void streamFirebase() {
  if (Firebase.ready()) {
    if (!Firebase.RTDB.readStream(&stream_start)) {
      Serial.println(stream_start.errorReason());
    }
    if (!Firebase.RTDB.readStream(&stream_stop)) {
      Serial.println(stream_stop.errorReason());
    }
    if (!Firebase.RTDB.readStream(&stream_configs)) {
      Serial.println(stream_configs.errorReason());
    }

    if (stream_start.streamTimeout()) {
      Serial.println("Stream timeout, resume streaming...");
      Serial.println();
      if (!stream_start.httpConnected())
        Serial.printf("error code: %d, reason: %s\n\n", stream_start.httpCode(), stream_start.errorReason().c_str());
    }
    if (stream_stop.streamTimeout()) {
      Serial.println("Stream timeout, resume streaming...");
      Serial.println();
      if (!stream_stop.httpConnected())
        Serial.printf("error code: %d, reason: %s\n\n", stream_stop.httpCode(), stream_stop.errorReason().c_str());
    }
    if (stream_configs.streamTimeout()) {
      Serial.println("Stream timeout, resume streaming...");
      Serial.println();
      if (!stream_configs.httpConnected())
        Serial.printf("error code: %d, reason: %s\n\n", stream_configs.httpCode(), stream_configs.errorReason().c_str());
    }

    if (stream_start.streamAvailable()) {
      Serial.printf("stream path, %s\nevent path, %s\ndata type, %s\nevent type, %s\n\n",
                    stream_start.streamPath().c_str(),
                    stream_start.dataPath().c_str(),
                    stream_start.dataType().c_str(),
                    stream_start.eventType().c_str());
      printResult(stream_start);  // see addons/RTDBHelper.h
      Serial.print("Valor Start: ");
      Serial.println(stream_start.to<bool>());
      Serial.printf("Received stream payload size: %d (Max. %d)\n\n", stream_start.payloadLength(), stream_start.maxPayloadLength());
      Serial.println("- - - - - - - - - -");

      if (stream_start.to<bool>()) {
        start = true;
        enviarStartStopArduino(true);
      }
    }

    if (stream_stop.streamAvailable()) {
      Serial.printf("stream path, %s\nevent path, %s\ndata type, %s\nevent type, %s\n\n",
                    stream_stop.streamPath().c_str(),
                    stream_stop.dataPath().c_str(),
                    stream_stop.dataType().c_str(),
                    stream_stop.eventType().c_str());
      printResult(stream_stop);  // see addons/RTDBHelper.h
      Serial.print("Valor Stop: ");
      Serial.println(stream_stop.to<bool>());
      Serial.printf("Received stream payload size: %d (Max. %d)\n\n", stream_stop.payloadLength(), stream_stop.maxPayloadLength());
      Serial.println("- - - - - - - - - -");

      if (stream_stop.to<bool>()) {
        stop = true;
        enviarStartStopArduino(false);
      }
    }

    if (stream_configs.streamAvailable()) {
      Serial.printf("stream path, %s\nevent path, %s\ndata type, %s\nevent type, %s\n\n",
                    stream_configs.streamPath().c_str(),
                    stream_configs.dataPath().c_str(),
                    stream_configs.dataType().c_str(),
                    stream_configs.eventType().c_str());
      printResult(stream_configs);  // see addons/RTDBHelper.h
      Serial.print("Valor Configs: ");
      Serial.println(stream_configs.to<bool>());
      Serial.printf("Received stream payload size: %d (Max. %d)\n\n", stream_configs.payloadLength(), stream_configs.maxPayloadLength());
      Serial.println("- - - - - - - - - -");

      if (stream_configs.to<bool>()) {
        receberDadosConfigsRTDB();
        novosDadosConfigs = true;
        //enviarDadosConfigsArduino();
      }
    }
  }
}


/*void loopFirebase() {
  //receberDadosMagazine();
  streamFirebase();
}*/


//
void enviarDadosMagazineRTDB(const byte prateleira, const byte posicao, const char *novoValor) {
  //atualiza o banco de dados apenas na posição recebida
  char tempNum[] = "-";
  itoa(prateleira, tempNum, 10);

  char path[24];
  strcpy(path, "magazine/prateleira_");
  strcat(path, tempNum);
  strcat(path, "/");
  itoa(posicao, tempNum, 10);
  strcat(path, tempNum);

  Serial.print("Caminho: ");
  Serial.println(path);

  Serial.print("Novo Valor: ");
  Serial.println(novoValor);

  if (Firebase.ready()) {
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


// - - - - TESTAR - - - -
// ok
void receberDadosConfigsRTDB() {
  if (Firebase.RTDB.get(&fbdo, "configs")) {

    FirebaseJson json = fbdo.to<FirebaseJson>();
    FirebaseJsonData result;
    char *valor;

    json.get(result, "hab_v");
    if (result.success) Hab_V = (result.to<bool>());

    json.get(result, "hab_p");
    if (result.success) Hab_P = (result.to<bool>());

    json.get(result, "hab_m");
    if (result.success) Hab_M = (result.to<bool>());

    json.get(result, "prat_0");
    if (result.success) strcpy(Prat_0, result.to<const char *>());

    json.get(result, "prat_1");
    if (result.success) strcpy(Prat_1, result.to<const char *>());

    json.get(result, "prat_2");
    if (result.success) strcpy(Prat_2, result.to<const char *>());

    conversaoPrateleiras();

    json.get(result, "tipo_mov");
    if (result.success){
      if(strcmp(result.to<const char *>(), "remover") == 0){
        Tipo_Movimento[0] = 'r';
      } else {
        Tipo_Movimento[0] = 'c';
      }
    }

    json.get(result, "tipo_seq");
    if (result.success){
      if(strcmp(result.to<const char *>(), "aleatorio") == 0){
        Tipo_Sequencia[0] = 'a';
      } else {
        Tipo_Sequencia[0] = 's';
      }
    }

    json.get(result, "ciclos");
    if (result.success) n_ciclos = (result.to<int>());
  } else {
    Serial.println(fbdo.errorReason());
  }

  if (!Firebase.RTDB.setBool(&fbdo, "stream/novas_configs", false)) {
    Serial.println(fbdo.errorReason());
  }
}

//
void enviarDadosConfigsRTDB() {
  FirebaseJson configs;

  configs.add("hab_v", Hab_V);
  configs.add("hab_p", Hab_P);
  configs.add("hab_m", Hab_M);
  configs.add("prat_0", Prat_0);
  configs.add("prat_1", Prat_1);
  configs.add("prat_2", Prat_2);
  configs.add("tipo_movimento", Tipo_Movimento);
  configs.add("tipo_sequencia", Tipo_Sequencia);
  configs.add("ciclos", n_ciclos);

  if (Firebase.ready()) {
    if (Firebase.RTDB.updateNode(&fbdo, "configs", &configs)) {
      Serial.println(fbdo.dataPath());
      Serial.println(fbdo.dataType());
      Serial.println(fbdo.to<String>());
    } else {
      Serial.println(fbdo.errorReason());
    }
  }
}

//
void enviarDadosProcessoRTDB() {
  FirebaseJson estado_processo;

  estado_processo.add("processo_em_andamento", processo_em_andamento);
  estado_processo.add("robo_em_movimento", robo_em_movimento);
  estado_processo.add("ciclo_atual", ciclo_atual);

  Serial.print("processo_em_andamento ");
  Serial.println(processo_em_andamento);
  Serial.print("robo_em_movimento ");
  Serial.println(robo_em_movimento);
  Serial.print("ciclo_atual ");
  Serial.println(ciclo_atual);

  

  if (Firebase.ready()) {
    if (Firebase.RTDB.updateNode(&fbdo, "estado_processo", &estado_processo)) {
      Serial.println(fbdo.dataPath());
      Serial.println(fbdo.dataType());
      Serial.println(fbdo.to<String>());
    } else {
      Serial.println(fbdo.errorReason());
    }
  }
}