 #include <WiFi.h>
 #include <PubSubClient.h>
   
  
  #include <DHT.h> 

// Указываем пины для подключения датчиков
#define DHT_PIN 5      // Пин, к которому подключен датчик DHT11
#define SOIL_PIN 34    // Пин, к которому подключен датчик влажности почвы
#define LIGHT_PIN 35   // Пин, к которому подключен фоторезистор
#define FAN_PIN 2   // Пин, к которому подключен фоторезистор



 /////////////////// SETTINGS ///////////////////////////// 
   
  // Wi-Fi 
  const char* ssid = "Galaxy M31922F"; 
  const char* password = "123123123"; 


  // MQTT 
  const char* mqtt_server = "m3.wqtt.ru"; 
  const int mqtt_port = 10273; 
  const char* mqtt_user = "u_BHWYQV"; 
  const char* mqtt_password = "71fnLgKU"; 

// Указываем идентификатор устройства и темы MQTT
  
  //const int sending_period = 5; 

const char* temperature_topic = "test9999/temperature";
const char* humidity_topic = "test9999/humidity";
const char* soil_moisture_topic = "test9999/soil_humidity";
const char* light_intensity_topic = "test9999/light_intensity";

const char* fan_control_topic = "test9999/fan_control";
const char* led_control_topic = "test9999/led_control";

// Создаем объекты для работы с датчиками
DHT dht(DHT_PIN, DHT11);

// Создаем объект для работы с Wi-Fi
WiFiClient espClient;
PubSubClient client(espClient);

// Функция для подключения к Wi-Fi сети
void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

// Функция для подключения к MQTT брокеру
//void reconnect() {
//  while (!client.connected()) {
//    Serial.print("Attempting MQTT connection...");
//    if (client.connect(device_id)) {
//      Serial.println("connected");
//      client.subscribe(fan_control_topic);
//      client.subscribe(led_control_topic);
//    } else {
//      Serial.print("failed, rc=");
//      Serial.print(client.state());
//      Serial.println(" try again in 5 seconds");
//      delay(5000);
//    }
//  }
//}

void reconnect() { 
    while (!client.connected()) { 
      Serial.print("Attempting MQTT connection..."); 
      String clientId = "ESP32-" + WiFi.macAddress(); 
      if (client.connect(clientId.c_str(), mqtt_user, mqtt_password) ) { 
        Serial.println("connected"); 
      client.subscribe(fan_control_topic);
      client.subscribe(led_control_topic);

      } else { 
        Serial.print("failed, rc="); 
        Serial.print(client.state()); 
        Serial.println(" try again in 5 seconds"); 
        delay(5000); 
      } 
    } 
  } 

// Функция для отправки данных на MQTT брокер
void send_sensor_data() {
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();
  int soil_moisture = analogRead(SOIL_PIN);
  int light_intensity = analogRead(LIGHT_PIN);

  // Отправляем данные о температуре, влажности, влажности почвы и освещенности
  client.publish(temperature_topic, String(temperature).c_str());
  client.publish(humidity_topic, String(humidity).c_str());
  client.publish(soil_moisture_topic, String(soil_moisture).c_str());
  client.publish(light_intensity_topic, String(light_intensity).c_str());
}

// Функция, которая вызывается при получении сообщения от MQTT брокера
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Проверяем условия для управления исполнительными устройствами

  
  int value = payloadStr.toInt();
  if (strcmp(topic, humidity_topic) == 0 && value > 80) {
    digitalWrite(FAN_PIN, HIGH);
    Serial.println("FAN turned ON due to light condition");
  } 
  else {
    digitalWrite(FAN_PIN, LOW);
    Serial.println("FAN turned OFF");
  }
}  
}

void setup() {
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  dht.begin();
  pinMode(SOIL_PIN, INPUT);
  pinMode(LIGHT_PIN, INPUT);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  send_sensor_data();
  delay(10000); // Отправляем данные каждые 10 секунд
}
