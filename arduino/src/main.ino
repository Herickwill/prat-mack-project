// Código do ESP32 com leitura do sensor TDS e envio via MQTT
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <time.h>

// ======== Wi-Fi Config ========
const char* ssid = "SSID_WIFI";
const char* password = "SENHA_WIFI";

// ======== AWS IoT Config ========
const char* mqtt_server = "a2nrghdakdc223-ats.iot.sa-east-1.amazonaws.com";
const int mqtt_port = 8883;
const char* mqtt_topic = "esp32/tds";

// Certificado do dispositivo (.pem.crt)
const char* certificate_pem_crt = "OMITIDO";

// Chave privada (.pem.key)
const char* private_pem_key = "OMITIDO";

// Amazon Root CA (.pem)
const char* amazon_root_ca = "OMITIDO";

// ======== TDS Config ========
const int tdsPin = 34; // GPIO34 (D34)
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

  WiFi.begin(ssid, password);
  Serial.print("Conectando ao Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi conectado!");
  Serial.println(WiFi.localIP());

  // Configurar NTP (UTC-3 para Brasil)
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
  if (millis() - lastRead > 30000) { // a cada 30 segundos
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

    lastRead = millis();
  }
}
