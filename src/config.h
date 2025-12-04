#ifndef CONFIG_H
#define CONFIG_H

// ============================================
// CẤU HÌNH MQTT BROKER
// ============================================
#define MQTT_BROKER "w0e16d00.ala.asia-southeast1.emqxsl.com"
#define MQTT_PORT 8883  // MQTTS (TLS/SSL)

// ============================================
// TOPICS MQTT
// ============================================
// Topics publish (ESP32 → Server)
#define TOPIC_TEMP "home/temp"
#define TOPIC_HUMIDITY "home/humidity"
#define TOPIC_GAS "home/gas"

// Topics subscribe (Server → ESP32)
#define TOPIC_LIGHT_CMD "home/light/cmd"
#define TOPIC_FAN_CMD "home/fan/cmd"
#define TOPIC_DOOR_CMD "home/door/cmd"

#endif