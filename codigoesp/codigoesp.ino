#include <WiFi.h>
#include <HTTPClient.h>
#include <DHT.h>
#include <ArduinoJson.h>

// === Configurações Wi-Fi ===
const char* ssid = "SNAP";
const char* password = "RJ77602b!";

// === URLs do servidor Flask ===
const char* serverDataUrl = "http://192.168.18.9:5000/api/dados";
const char* serverCommandUrl = "http://192.168.18.9:5000/api/comando";

// === Configurações do sensor DHT ===
#define DHTPIN 18
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// === Pinos dos motores (L298N) ===
#define ENA 25
#define IN1 26
#define IN2 27
#define ENB 33
#define IN3 14
#define IN4 12

// === Velocidade dos motores (0 a 255) ===
int velocidadeEsquerda = 230;
int velocidadeDireita = 225;
int velocidadeCurva = 200;  // Aumentado para ajudar a “empurrar” nas curvas

// === Controle modo automático ===
bool modoAuto = false;
unsigned long autoStartTime = 0;
const unsigned long autoDuration = 15000; // 15 segundos

// Timers para enviar dados e buscar comandos
unsigned long lastSendTime = 0;
unsigned long lastCommandTime = 0;
const unsigned long sendInterval = 3000;   // a cada 3 segundos
const unsigned long commandInterval = 500; // a cada 0.5 segundo

void setup() {
  Serial.begin(115200);
  Serial.println("Iniciando...");

  WiFi.begin(ssid, password);
  Serial.print("Conectando ao Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n✅ Wi-Fi conectado!");
  Serial.print("IP Local: ");
  Serial.println(WiFi.localIP());

  dht.begin();

  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  ledcSetup(0, 5000, 8);
  ledcSetup(1, 5000, 8);

  ledcAttachPin(ENA, 0);
  ledcAttachPin(ENB, 1);

  pararMotores();
}

void loop() {
  unsigned long now = millis();

  // Enviar dados periodicamente
  if (now - lastSendTime >= sendInterval) {
    enviarDados();
    lastSendTime = now;
  }

  // Buscar comando periodicamente
  if (now - lastCommandTime >= commandInterval) {
    String comando = buscarComando();
    lastCommandTime = now;

    if (comando == "auto") {
      if (!modoAuto) {
        Serial.println("Modo automático iniciado");
        modoAuto = true;
        autoStartTime = now;
        andarFrente();
      }
    } else {
      // Sai do modo automático se estava ativo
      if (modoAuto) {
        Serial.println("Saindo do modo automático");
        modoAuto = false;
        pararMotores();
      }

      if (comando == "frente") {
        Serial.println("Comando: andar frente");
        andarFrente();
      }
      else if (comando == "tras") {
        Serial.println("Comando: andar trás");
        andarTras();
      }
      else if (comando == "parar") {
        Serial.println("Comando: parar");
        pararMotores();
      }
      else if (comando == "esquerda") {
        Serial.println("Comando: virar à esquerda");
        virarEsquerda();
      }
      else if (comando == "direita") {
        Serial.println("Comando: virar à direita");
        virarDireita();
      }
      else {
        Serial.println("Comando desconhecido, parando motores");
        pararMotores();
      }
    }
  }

  // Se modo automático ativado, verifica se o tempo passou
  if (modoAuto && (now - autoStartTime >= autoDuration)) {
    Serial.println("Modo automático finalizado após 15s");
    pararMotores();
    modoAuto = false;
  }
}

void enviarDados() {
  float temperatura = dht.readTemperature();
  float umidade = dht.readHumidity();

  if (isnan(temperatura) || isnan(umidade)) {
    Serial.println("❌ Erro na leitura do DHT11!");
    return;
  }

  Serial.printf("🌡️ Temperatura: %.1f °C | 💧 Umidade: %.1f %%\n", temperatura, umidade);

  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(serverDataUrl);
    http.addHeader("Content-Type", "application/json");

    String json = "{\"temperatura\":" + String(temperatura, 1) + ",\"umidade\":" + String(umidade, 1) + "}";
    int httpResponseCode = http.POST(json);

    if (httpResponseCode > 0) {
      String resposta = http.getString();
      Serial.print("📡 Resposta do servidor: ");
      Serial.println(resposta);
    } else {
      Serial.print("❌ Erro ao enviar dados: ");
      Serial.println(httpResponseCode);
    }

    http.end();
  } else {
    Serial.println("❌ Wi-Fi desconectado. Não foi possível enviar os dados.");
  }
}

String buscarComando() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(serverCommandUrl);
    int httpCode = http.GET();

    if (httpCode > 0) {
      String resposta = http.getString();
      http.end();

      DynamicJsonDocument doc(256);
      auto error = deserializeJson(doc, resposta);

      if (!error) {
        const char* acao = doc["acao"];
        if (acao != nullptr) {
          return String(acao);
        }
      }
      Serial.println("Erro ao parsear JSON ou comando vazio");
    } else {
      Serial.printf("Erro HTTP: %d\n", httpCode);
      http.end();
    }
  }
  return "auto"; // padrão
}

void andarFrente() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);

  ledcWrite(0, velocidadeEsquerda);
  ledcWrite(1, velocidadeDireita);
}

void andarTras() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);

  ledcWrite(0, velocidadeEsquerda);
  ledcWrite(1, velocidadeDireita);
}

void virarEsquerda() {
  // Motor esquerdo para trás, motor direito para frente — gira “empurrando” para esquerda
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);

  ledcWrite(0, velocidadeCurva);
  ledcWrite(1, velocidadeCurva);
}

void virarDireita() {
  // Motor direito para trás, motor esquerdo para frente — gira “empurrando” para direita
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);

  ledcWrite(0, velocidadeCurva);
  ledcWrite(1, velocidadeCurva);
}

void pararMotores() {
  ledcWrite(0, 0);
  ledcWrite(1, 0);

  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
}
