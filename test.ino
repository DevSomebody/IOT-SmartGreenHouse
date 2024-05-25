  #include "WiFi.h"
  #include <PubSubClient.h>
  //#include <iocontrol.h>
   
  
  #include <DHT.h> 
  //#include <Adafruit_Sensor.h> 
  #define pisk 16 
  /////////////////// SETTINGS ///////////////////////////// 
   
  // Wi-Fi 
  const char* ssid = "Galaxy M31922F"; 
  const char* password = "123123123"; 
  // MQTT 
  const char* mqtt_server = "m3.wqtt.ru"; 
  const int mqtt_port = 10273; 
  const char* mqtt_user = "u_BHWYQV"; 
  const char* mqtt_password = "71fhLgKU"; 
   
  // SENSOR 
  const int sending_period = 5; 
  const bool retain_flag = false; 
  const char* temperature_topic = "test9999/temperature"; 
  const char* humidity_topic = "test9999/humidity"; 
  const char* soil_hum = "test9999/soil_hum";
  const char* light = "test9999/light";  
  
  //const int led = 5;
   
  ///////////////////////////////////////////////////////// 
   
   
  WiFiClient espClient; 
  PubSubClient client(espClient); 
  uint32_t tmr1; 
   
  void setup_wifi() { 
   
    delay(10); 
    Serial.println(); 
    Serial.print("Connecting to "); 
    Serial.println(ssid); 
   
    WiFi.mode(WIFI_STA); 
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
   
  void reconnect() { 
    while (!client.connected()) { 
      Serial.print("Attempting MQTT connection..."); 
      String clientId = "ESP32-" + WiFi.macAddress(); 
      if (client.connect(clientId.c_str(), mqtt_user, mqtt_password) ) { 
        Serial.println("connected"); 
        client.subscribe("test9999/humidity");   // subscribe the topics heretest9999/temperature 
        client.subscribe("test9999/temperature");  
      } else { 
        Serial.print("failed, rc="); 
        Serial.print(client.state()); 
        Serial.println(" try again in 5 seconds"); 
        delay(5000); 
      } 
    } 
  } 
  //int Vo;  
  //float R1 = 10000; // значение R1 на модуле  
  //float logR2, R2, T;  
  //float c1 = 0.001129148, c2 = 0.000234125, c3 = 0.0000000876741; //коэффициенты Штейнхарта-Харта для термистора  
  
   DHT dht(5, DHT11); 
  
  
     
  void sendData() { 
     float h = dht.readHumidity();
     float t = dht.readTemperature();
      
     // Считывание значения влажности почвы с аналогового пина 32 и преобразование его в проценты
     float soil_humidity_percent = map(analogRead(32), 0, 4095, 0, 100); 
     
     // Считывание значения интенсивности света с аналогового пина 35 и преобразование его в проценты
     float light_intensity_percent = map(analogRead(35), 0, 4095, 0, 100); 
     
     // Публикация данных в MQTT
     client.publish(humidity_topic, String(h).c_str(), retain_flag);
     client.publish(temperature_topic, String(t).c_str(), retain_flag);
     client.publish(soil_hum, String(soil_humidity_percent).c_str(), retain_flag); // Отправка данных о влажности почвы
     client.publish(light, String(light_intensity_percent).c_str(), retain_flag); // Отправка данных о свете
  }
  
  
   
  void callback(char *topic, byte *payload, unsigned int length) {
    // Обработка полученных сообщений
    String payloadStr = "";
    for (int i = 0; i < length; i++) {
      payloadStr += (char)payload[i];
    }
  
    Serial.print("Message received on topic: ");
    Serial.print(topic);
    Serial.print(", Payload: ");
    Serial.println(payloadStr);
  
    // Проверяем, превышают ли значения на топиках пороги
    int value = payloadStr.toInt();
    if (strcmp(topic, humidity_topic) == 0 && value > 100) {
      digitalWrite(pisk, HIGH);
      Serial.println("LED turned ON due to light condition");
    } else if (strcmp(topic, temperature_topic) == 0 && value > 32) {
      digitalWrite(pisk, HIGH);
      Serial.println("LED turned ON due to temperature condition");
    } else {
      digitalWrite(pisk, LOW);
      Serial.println("LED turned OFF");
    }
  }  
   
  void setup() { 
    dht.begin();  
    Serial.begin(115200); 
   
    setup_wifi(); 
  
    client.setServer(mqtt_server, mqtt_port); 
    client.setCallback(callback); 
  
    pinMode(pisk, OUTPUT); //set up LED
  
  } 
   
  void loop() { 
    if (!client.connected()) { 
      reconnect(); 
    } 
    client.loop(); 
   
    if (millis() - tmr1 >= (sending_period * 100)) { 
      tmr1 = millis(); 
      sendData(); 
    } 
   
    }
