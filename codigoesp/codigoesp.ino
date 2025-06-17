#include <WiFi.h>
#include <HTTPClient.h>
#include <DHT.h>

// === Wi-Fi ===
const char* ssid = "SNAP";
const char* password = "RJ77602b!";

// === Servidor Flask ===
const char* serverName = "http://192.168.18.9:5000/api/dados";

// === DHT ===
#define DHTPIN 18
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// === Motores (L298N) ===
#define ENA 25
#define IN1 26
#define IN2 27
#define ENB 33
#define IN3 14
#define IN4 12

void setup() {
  Serial.begin(115200);
  Serial.println("Iniciando...");

  // === Conecta ao Wi-Fi ===
  WiFi.begin(ssid, password);
  Serial.print("Conectando ao Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n✅ Wi-Fi conectado!");
  Serial.print("IP Local: ");
  Serial.println(WiFi.localIP());

  // === Inicializa DHT ===
  dht.begin();
  delay(2000);

  // === Configura motores ===
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  ledcSetup(0, 5000, 8);
  ledcSetup(1, 5000, 8);

  ledcAttachPin(ENA, 0);
  ledcAttachPin(ENB, 1);

  // === Anda para frente ===
  andarFrente();
  Serial.println("🚗 Andando pra frente por 15 segundos...");
  delay(15000);

  pararMotores();
  Serial.println("🛑 Carrinho parado.");
}

void loop() {
  float temperatura = dht.readTemperature();
  float umidade = dht.readHumidity();

  if (isnan(temperatura) || isnan(umidade)) {
    Serial.println("❌ Erro na leitura do DHT11!");
  } else {
    Serial.print("🌡️ Temperatura: ");
    Serial.print(temperatura);
    Serial.println(" °C");

    Serial.print("💧 Umidade: ");
    Serial.print(umidade);
    Serial.println(" %");

    if (WiFi.status() == WL_CONNECTED) {
      HTTPClient http;
      http.begin(serverName);
      http.addHeader("Content-Type", "application/json");

      String json = "{\"temperatura\":" + String(temperatura, 1) +
              ",\"umidade\":" + String(umidade, 1) + "}";

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

  delay(10000); // Faz a leitura e envio a cada 10 segundos
}

// === Função para andar pra frente ===
void andarFrente() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);

  ledcWrite(0, 255);
  ledcWrite(1, 255);
}

// === Função para parar os motores ===
void pararMotores() {
  ledcWrite(0, 0);
  ledcWrite(1, 0);

  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
}
