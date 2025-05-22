#include "DHT.h"

#define DHTPIN 4     // ajuste para o seu pino
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(115200);
  dht.begin();
  Serial.println("Teste sensor DHT11");
}

void loop() {
  delay(2000);

  float t = dht.readTemperature();
  float h = dht.readHumidity();

  if (isnan(t) || isnan(h)) {
    Serial.println("Falha na leitura do sensor DHT11");
    return;
  }

  Serial.print("Temperatura: ");
  Serial.print(t);
  Serial.print(" °C\t");

  Serial.print("Umidade: ");
  Serial.print(h);
  Serial.println(" %");
}