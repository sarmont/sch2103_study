#include <ESP8266WiFi.h>
                   
#include <Wire.h>
#include <LPS331.h>
#include <WiFiClient.h>
#include "DHT.h" // датчик влажности
//MQ135
#include <TroykaMQ.h>
//экран
#include <LiquidCrystal_I2C.h>
// работа с wifi
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h> 


LiquidCrystal_I2C lcd(0x3F,16,2); //создаем объект экрана lcd 

// барометр + температура
LPS331 barometer;
IPAddress apIP(192, 168, 0, 10);
#define DHTPIN 2     // dht11 пин
#define PIN_MQ135  A0

// датчик качества воздуха
MQ135 mq135(PIN_MQ135);

#define DHTTYPE DHT11   // DHT 11

DHT dht(DHTPIN, DHTTYPE); //создаем объект датчика dht11

const char* host = "meteo.krivaksin.ru";

int buff = 0;

int led = 13;

String Hostname; //имя нашего устройства 
#define debug true // вывод отладочных сообщений

float t;  // TEMPERATURE VAR
float h;  // HUMIDITY VAR
float p;  //DAVLENIE

String data;

          // функция начального подключения к wifi
          void wifimanstart() {
            WiFiManager wifiManager;
            wifiManager.setDebugOutput(debug);
            wifiManager.setMinimumSignalQuality();
            if (!wifiManager.autoConnect("ESP8266")) {
            if (debug) Serial.println("failed to connect and hit timeout");
              delay(3000);
              //reset and try again, or maybe put it to deep sleep
              ESP.reset();
              delay(5000); }
          if (debug) Serial.println("connected...");
          }

void setup() {
   Serial.begin(115200);
   pinMode(led, OUTPUT);
 

          
  
   Hostname = "ESP"+WiFi.macAddress(); //формируем имя устройства
  Hostname.replace(":",""); // удаляем двоеточия 
   Wire.begin();
//проверка барометра
  if (!barometer.init())
  {
    Serial.println("Failed to autodetect pressure sensor!");
    while (1);
  }
  barometer.enableDefault();
 
// газ
 // указываем значение на чистом воздухе
   
mq135.calibrate(100);
 
//lcd
  lcd.init();      // In ESP8266-01, SDA=0, SCL=2               
  lcd.backlight();

   WiFi.hostname(Hostname);
  //запускаем функцию подключения к wifi
  wifimanstart();
  Serial.println(WiFi.localIP()); 
  Serial.println(WiFi.macAddress()); 
  Serial.print("ID: "); 
  Serial.println(Hostname); // выдаем в компорт айпишник железки и ее id. этот id используется при регистрации датчика на сайте народного мониторинга, так же выводим на дисплей
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(WiFi.localIP());
  lcd.setCursor(0, 1);
  lcd.print(Hostname);
  delay(15000);
}

void loop(){
 delay(2000);
                   

 // создаём переменную и присваиваем ей значения абсолютного давления
            p = barometer.readPressureMillibars();
 // создаём переменную и присваиваем ей температуру окружающей среды
            t = barometer.readTemperatureC();
            t = t - 2;

// считываем значение каче5ства воздуха в переменную
            int gas =  mq135.readCO2();
   

//влажность
            h = dht.readHumidity();

// условие включения либо внешнего устройства, либо  светодиода на корпусе устройства
if(t>=27)
{
  digitalWrite(led,HIGH); //зажигаем светодиод на корпусе устройства
} else  {
  digitalWrite(led,LOW);  // //гасим светодиод на корпусе устройства
}
  
// проверка.
  if (isnan(h)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
   if (isnan(p)) {
    Serial.println("Failed to read from pressure sensor!");
    return;
  }
   if (isnan(t)) {
    Serial.println("Failed to read from temperature sensor!");
    return;
  }
    Serial.println(h);
    Serial.print("Davlenie: ");
    Serial.print(p*0.75006375541921);
    Serial.print("Temperature: ");
    Serial.print(t);
    Serial.print("Gas: ");
    Serial.print(gas);
    //////////////////
    lcd.home();                // At column=0, row=0
    lcd.print("T:");
    lcd.print(t);
    lcd.print("|D:"); 
    lcd.print(p*0.750062);  
    lcd.setCursor(0, 1);
    lcd.print("H:");
    lcd.print(h);
    lcd.print("|PPM:");
    lcd.print(gas);
/////////////// 
WiFiClient client;
if (client.connect(host,80)) { 
    Serial.println("good conection...");
    //Составляем строку запроса
    String data = "/add.php?";
    data += "temp1=";
    data += String(t);
    data += "&hum1=";
    data += String(h);
    data += "&hum2=";
    data += String(p*0.75006375541921);
    data += "&hum3=";
    data += String(gas);
 // Выводим переданную ссылку запроса в порт
  Serial.print("Getting Query: ");
  Serial.println(host + data);
// отправляем GET запрос на сервер
  client.print(String("GET ") + data + " HTTP/1.1\r\n" + "Host: meteo.krivaksin.ru" + "\r\n" + "Connection: close\r\n\r\n");
  delay(3000);
  }
  if (client.connected()) { 
    client.stop();  // DISCONNECT FROM THE SERVER
  }

 delay(1000*15);
}


