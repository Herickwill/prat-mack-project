#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <time.h>

#include "certificado.h"
#include "chave_privada.h"
#include "amazon_root_ca.h"

// ======== Wi-Fi Config ========
const char* ssid = "OMITIDO";
const char* password = "OMITIDO";

// ======== AWS IoT Config ========
const char* mqtt_server = "a2nrghdakdc223-ats.iot.sa-east-1.amazonaws.com";
const int mqtt_port = 8883;
const char* mqtt_topic = "esp32/tds";

// ======== TDS Config ========
const int tdsPin = 34; // GPIO34 (D34)
const int ledPin = 25;// GPIO25 para o Atuador
float vref = 3.3;       // tensão de referência
int adcResolution = 4095; // resolução de 12 bits

WiFiClientSecure net;
PubSubClient client(net);

// ======== AWS Connection ========
void connectAWS() {
  while (!client.connected()) {
    Serial.print("Conectando ao MQTT da AWS...");
    if (client.connect("esp32_tds_client")) {
      Serial.println("Conectado!");
    } else {
      Serial.print("Falhou. Estado: ");
      Serial.println(client.state());
      delay(2000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  analogReadResolution(12);

  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW); // começa desligado

  WiFi.begin(ssid, password);
  Serial.print("Conectando ao Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi conectado!");
  Serial.println(WiFi.localIP());

  configTime(-3 * 3600, 0, "pool.ntp.org", "time.nist.gov");
  Serial.print("Aguardando sincronização NTP");
  time_t now = time(nullptr);
  while (now < 100000) {
    delay(500);
    Serial.print(".");
    now = time(nullptr);
  }
  Serial.println("\nTempo sincronizado!");

  net.setCACert(amazon_root_ca);
  net.setCertificate(certificate_pem_crt);
  net.setPrivateKey(private_pem_key);
  client.setServer(mqtt_server, mqtt_port);
}

void loop() {
  if (!client.connected()) {
    connectAWS();
  }
  client.loop();

  static unsigned long lastRead = 0;
  static bool isPolluted = false;

  if (millis() - lastRead > 30000) {
    int raw = analogRead(tdsPin);
    float voltage = raw * (vref / adcResolution);
    float tdsValue = (133.42 * voltage * voltage * voltage
                    - 255.86 * voltage * voltage
                    + 857.39 * voltage) * 0.5;

    unsigned long timestamp = time(nullptr);

    String payload = "{\"caixa_id\": \"caixa_1\", \"valor_sensor\": ";
    payload += String(tdsValue, 2);
    payload += ", \"timestamp\": ";
    payload += String(timestamp);
    payload += "}";

    Serial.println(payload);
    client.publish(mqtt_topic, payload.c_str());

    // Atualiza estado de poluição
    isPolluted = (tdsValue > 150);

    lastRead = millis();
  }

  // Piscar LED se estiver poluído
  if (isPolluted) {
    digitalWrite(ledPin, HIGH);
    delay(200);
    digitalWrite(ledPin, LOW);
    delay(200);
  } else {
    digitalWrite(ledPin, LOW); // LED desligado se não estiver poluído
  }
}
