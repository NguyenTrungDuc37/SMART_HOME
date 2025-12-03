// ============================================
// IMPORT THƯ VIỆN
// ============================================
#include <WiFi.h>
#include <PubSubClient.h>
#include <WiFiClientSecure.h>  
#include <DHT.h>
#include "config.h"
#include "credentials.h"

// ============================================
// ĐỊNH NGHĨA PINS
// ============================================
#define DHT_PIN 4           // DHT22 data pin
#define DHT_TYPE DHT22
#define LED_PIN 2           // LED built-in
#define FAN_PIN 5           // Relay quạt
#define DOOR_PIN 18         // Servo/motor cửa
#define GAS_PIN 34          // Cảm biến MQ2/MQ5 (analog)

// ============================================
// KHỞI TẠO ĐỐI TƯỢNG
// ============================================
WiFiClientSecure espClient;
PubSubClient client(espClient);
DHT dht(DHT_PIN, DHT_TYPE);

// ============================================
// BIẾN TOÀN CỤC
// ============================================
unsigned long lastPublish = 0;
const unsigned long PUBLISH_INTERVAL = 1000;  // Publish mỗi 5 giây

// ============================================
// HÀM KẾT NỐI WIFI
// ============================================
void setupWiFi() {
  Serial.println("\n🌐 Đang kết nối WiFi...");
  Serial.print("   SSID: ");
  Serial.println(WIFI_SSID);
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n✅ WiFi connected!");
    Serial.print("   IP: ");
    Serial.println(WiFi.localIP());
    Serial.print("   Signal: ");
    Serial.print(WiFi.RSSI());
    Serial.println(" dBm\n");
  } else {
    Serial.println("\n❌ WiFi connection failed!");
    ESP.restart();
  }
}

// ============================================
// CALLBACK NHẬN LỆNH TỪ MQTT
// ============================================
void mqttCallback(char* topic, byte* payload, unsigned int length) {
  // Chuyển payload thành String
  String message = "";
  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  
  String topicStr = String(topic);
  Serial.println("📥 MQTT received:");
  Serial.println("   Topic: " + topicStr);
  Serial.println("   Message: " + message);
  
  // ---- ĐIỀU KHIỂN ĐÈN ----
  if (topicStr == "home/light/cmd") {
    if (message == "ON") {
      digitalWrite(LED_PIN, HIGH);
      Serial.println("   💡 LED: ON\n");
    } else if (message == "OFF") {
      digitalWrite(LED_PIN, LOW);
      Serial.println("   💡 LED: OFF\n");
    }
  }
  
  // ---- ĐIỀU KHIỂN QUẠT ----
  else if (topicStr == "home/fan/cmd") {
    if (message == "ON") {
      digitalWrite(FAN_PIN, HIGH);
      Serial.println("   🌀 FAN: ON\n");
    } else if (message == "OFF") {
      digitalWrite(FAN_PIN, LOW);
      Serial.println("   🌀 FAN: OFF\n");
    }
  }
  
  // ---- ĐIỀU KHIỂN CỬA ----
  else if (topicStr == "home/door/cmd") {
    if (message == "OPEN") {
      // TODO: Thêm code điều khiển servo
      // servo.write(90);
      Serial.println("   🚪 DOOR: OPEN\n");
    } else if (message == "CLOSE") {
      // servo.write(0);
      Serial.println("   🚪 DOOR: CLOSE\n");
    }
  }
}

// ============================================
// HÀM KẾT NỐI MQTT
// ============================================
void reconnectMQTT() {
  while (!client.connected()) {
    Serial.print("🔌 Connecting to MQTT Broker...");
    
    // Tạo client ID ngẫu nhiên
    String clientId = "ESP32_" + String(random(0xffff), HEX);
    
    // Kết nối với username/password
    if (client.connect(clientId.c_str(), MQTT_USER, MQTT_PASS)) {
      Serial.println(" ✅ Connected!");
      Serial.println("   Client ID: " + clientId);
      
      // Subscribe các topics điều khiển
      client.subscribe("home/light/cmd");
      client.subscribe("home/fan/cmd");
      client.subscribe("home/door/cmd");
      
      Serial.println("   Subscribed topics:");
      Serial.println("   - home/light/cmd");
      Serial.println("   - home/fan/cmd");
      Serial.println("   - home/door/cmd\n");
      
    } else {
      Serial.print(" ❌ Failed, rc=");
      Serial.println(client.state());
      Serial.println("   Retry in 5 seconds...\n");
      delay(5000);
    }
  }
}

// ============================================
// HÀM GIẢ LẬP DỮ LIỆU CẢM BIẾN (FAKE DATA)
// ============================================
void publishFakeData() {
  // Nhiệt độ: 20.0 - 35.0°C
  float temp = random(200, 350) / 10.0;
  
  // Độ ẩm: 40.0 - 80.0%
  float humidity = random(400, 800) / 10.0;
  
  // Khí gas: 100 - 500 ppm (thỉnh thoảng vượt ngưỡng 300)
  int gasPPM = random(100, 500);
  
  // Publish lên MQTT
  client.publish("home/temp", String(temp, 1).c_str());
  client.publish("home/humidity", String(humidity, 1).c_str());
  client.publish("home/gas", String(gasPPM).c_str());
  
  // Log ra Serial Monitor
  Serial.println("📤 Published (FAKE DATA):");
  Serial.printf("   🌡️  Nhiệt độ: %.1f°C\n", temp);
  Serial.printf("   💧 Độ ẩm: %.1f%%\n", humidity);
  Serial.printf("   💨 Khí gas: %d ppm", gasPPM);
  
  // Cảnh báo nếu gas cao
  if (gasPPM > 300) {
    Serial.print(" ⚠️  CẢNH BÁO!");
  }
  Serial.println("\n");
}

// ============================================
// HÀM ĐỌC CẢM BIẾN THẬT (REAL DATA)
// ============================================
void publishRealData() {
  // Đọc DHT22
  float temp = dht.readTemperature();
  float humidity = dht.readHumidity();
  
  // Đọc cảm biến gas
  int gasRaw = analogRead(GAS_PIN);
  int gasPPM = map(gasRaw, 0, 4095, 0, 1000);
  
  // Kiểm tra dữ liệu hợp lệ
  if (isnan(temp) || isnan(humidity)) {
    Serial.println("❌ Lỗi đọc DHT22!\n");
    return;
  }
  
  // Publish lên MQTT
  client.publish("home/temp", String(temp, 1).c_str());
  client.publish("home/humidity", String(humidity, 1).c_str());
  client.publish("home/gas", String(gasPPM).c_str());
  
  // Log ra Serial Monitor
  Serial.println("📤 Published (REAL DATA):");
  Serial.printf("   🌡️  Nhiệt độ: %.1f°C\n", temp);
  Serial.printf("   💧 Độ ẩm: %.1f%%\n", humidity);
  Serial.printf("   💨 Khí gas: %d ppm", gasPPM);
  
  if (gasPPM > 300) {
    Serial.print(" ⚠️  CẢNH BÁO!");
  }
  Serial.println("\n");
}

// ============================================
// SETUP
// ============================================
void setup() {
  // Khởi tạo Serial
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n\n==========================================");
  Serial.println("  🏠 UTC SMARTHOME ESP32 - STARTING...");
  Serial.println("==========================================\n");
  
  // Cấu hình GPIO
  pinMode(LED_PIN, OUTPUT);
  pinMode(FAN_PIN, OUTPUT);
  pinMode(DOOR_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  digitalWrite(FAN_PIN, LOW);
  digitalWrite(DOOR_PIN, LOW);
  
  // Khởi tạo DHT22
  dht.begin();
  
  // Kết nối WiFi
  setupWiFi();
    // Cấu hình MQTT
  espClient.setInsecure();  // Bỏ qua SSL cert (dev only)
  client.setServer(MQTT_BROKER, MQTT_PORT);
  client.setCallback(mqttCallback);
  client.setKeepAlive(60);
  
  // Khởi tạo random seed
  randomSeed(analogRead(0));
  
  Serial.println("🎲 CHẾ ĐỘ: FAKE DATA (Giả lập cảm biến)\n");
  Serial.println("==================================================\n");
}


// ============================================
// LOOP
// ============================================
void loop() {
  // Đảm bảo MQTT luôn kết nối
  if (!client.connected()) {
    reconnectMQTT();
  }
  client.loop();
  
  // Publish dữ liệu cảm biến mỗi 5 giây
  if (millis() - lastPublish >= PUBLISH_INTERVAL) {
    lastPublish = millis();
    
    // ⚠️ CHUYỂN ĐỔI GIỮA FAKE/REAL DATA
    publishFakeData();    // ← Dùng dữ liệu giả
    // publishRealData();  // ← Uncomment khi có DHT22 thật
  }
}