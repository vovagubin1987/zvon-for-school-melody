

#include <string.h>
#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <ESP8266SSDP.h>
#include <ESP8266HTTPUpdateServer.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266HTTPUpdateServer.h>  //Содержится в пакете
//ESP8266HTTPUpdateServer httpUpdater;
#include <ESP8266httpUpdate.h>  //Содержится в пакете
//File fsUploadFile;
//#include <FS.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <DNSServer.h>
//#include <TimeLib.h>
///#include <NTPClient.h>
#include <Wire.h>
#include <time.h>
#include "StringCommand.h"
#include "set.h"
#include "ntp2.h"
//#include "case"

//#include "json.h"
#include "fs2.h"
WiFiClient client;
#include <WiFiUdp.h>  //Содержится в пакете
WiFiUDP udp;
//
#include <SPI.h>

#define Number_MAX7219 1      // 3 chips = 24 digits
#define MAX7219_CS 15         // pin the MAX7219's CS pin is connected to
#define Default_Brightness 8  // brightness level 0-15
#include "DFRobotDFPlayerMini.h"  //dfplayer mini подключаем

#include "MAX7219_8_Digit_Driver.h"

//dfplayer mini подключаем
#include <SoftwareSerial.h>

// RX - цифровой вывод 10, необходимо соединить с выводом TX дисплея
// TX - цифровой вывод 11, необходимо соединить с выводом RX дисплея
//SoftwareSerial ss(12, 2);  //rx tx  d6 d4
DFRobotDFPlayerMini mp3;



//dfplayer mini подключаем конец

// make an instance of MAX7219_Digit called My_Display and set CS pin
//
MAX7219_8_Digit_Driver My_Display(MAX7219_CS, Number_MAX7219);


//#include "wifi.h"
//#include "web.h"

#include "tikerj.h"


HTTPClient http;
#define MAX_DEVICES 4

#define CLK_PIN 14  // or SCK d5

#define DATA_PIN 13  // or MOSI d7

#define CS_PIN 15  // or SS d8
//#define relePin 0  //d3
#define relePin 12  //d6
#define ESP8266_RX 5  // The ESP8266 pin connected to the TX of the Serial MP3 Player module 12
#define ESP8266_TX 4  // The ESP8266 pin connected to the RX of the Serial MP3 Player module 2

SoftwareSerial ss(ESP8266_RX, ESP8266_TX);  //rx tx  d6 d4

//#define buttonPin1 12 //d6 12
//
//#define buttonPin2 2 //d4 2
//#define buttonPin2 D0 //d4 2
//#define LEDPIN
//int button1State = digitalRead(buttonPin1);
//int button2State = digitalRead(buttonPin2);
int buttong = 0;
//WiFiUDP ntpUDP;
///&mytimeNTPClient timeClient(udp), "ru.pool.ntp.org");
//
time_t now;  // this are the seconds since Epoch (1970) - UTC
//tm tm;                              // the structure tm holds time information in a more convenient way

uint32_t sntp_startup_delay_MS_rfc_not_less_than_60000() {
  return 30000UL;  // 30s
}
uint32_t sntp_update_delay_MS_rfc_not_less_than_15000() {
  return 1 * 60 * 60 * 1000UL;  // 1 hours
}

void setup() {
  
  wifi_status_led_uninstall();  //wifi led to disable

  pinMode(relePin, OUTPUT);
  //pinMode(buttonPin1,INPUT_PULLUP);
  //pinMode(buttonPin2,INPUT_PULLUP);
  digitalWrite(relePin, LOW);
  //
  My_Display.String_To_Buffer("   H1   ", Default_Brightness);
  delay(1000);
  // put your setup code here, to run once:
  Serial.begin(9600);
  //Serial.setDebugOutput(true);
  Serial.println("open");
  chipID = String(ESP.getChipId()) + "-" + String(ESP.getFlashChipId());
  LittleFS.begin();
  HTTP.begin();


  configSetup = readFile(fileConfigS, 4096);
  initWIFI();
  digitalWrite(relePin, LOW);
  WiFi.printDiag(Serial);

  initHTTP();
  initFS();
  ///timeClient.begin();
  ///timeClient.setTimeOffset(10800);
  //Запускаем файловую систему
  Serial.println("Start 4-FS");
  //FS_init();

  Serial.println("Step7-FileConfig");
  //configSetup = readFile("config.json", 4096);
  //Запускаем WIFI
  //WIFIinit();
  //Настраиваем и запускаем HTTP интерфейс
  Serial.println("Start 2-WebServer");
  //HTTP_init();
  //initHTTP();
  //wire init
  Wire.begin(0, 2);  //SDA, SCL(4 5 old)
  Serial.println("Start ssdp");

  initSSDP();
  //тикеры
  f_ntp();  //jn reboot time sync
  t_tablo.attach_ms(1000, f_tablo);
  t_time.attach_ms(1000, f_time);
  t_ntp.attach(3600, f_ntp);
  //t_knopki.attach_ms(500, f_knopki);
  t_zvonok.attach(5, f_zvonok);
  //запуск нтп при загрузке
  Serial.println("Start ntp");

  dnsServer.processNextRequest();
  initNTP();
  f_ntp();
  initUpgrade();
  //получение времени
  f_time();
  setupToInit();
  Serial.println("Start readconf");

  readconf();  //заполняем массивы шаблонов звонков и массив привязки дней к шаблону из файла
  //digitalWrite(relePin, HIGH);

  digitalWrite(relePin, LOW);
  //pinMode(buttonPin1, INPUT_PULLUP);
  //
  //pinMode(buttonPin2, INPUT_PULLUP);
  //pinMode(buttonPin1, INPUT);
  //pinMode(buttonPin2, INPUT);
  My_Display.String_To_Buffer("   AU   ", Default_Brightness);
  ss.begin(9600, SWSERIAL_8N1, /*rx =*/ESP8266_RX, /*tx =*/ESP8266_TX);
 mp3.begin(ss, /*isACK = */true, /*doReset = */true);
 delay(5000);
  if (mp3.begin(ss)){
   Serial.println("OK");

    My_Display.String_To_Buffer("   AU O1  ", Default_Brightness);
  mp3.setTimeOut(700);
   delay(1000);
   My_Display.String_To_Buffer("   AU O2  ", Default_Brightness);
  //----Set volume----
  mp3.volume(29);  //Set volume value (0~30).
   My_Display.String_To_Buffer("   AU 3-29  ", Default_Brightness);
  //----Set different EQ----
  mp3.EQ(DFPLAYER_EQ_NORMAL);
   My_Display.String_To_Buffer("   AU O4  ", Default_Brightness);
  mp3.outputDevice(DFPLAYER_DEVICE_SD);
  } else {
    Serial.println("Connecting to DFPlayer Mini failed!");
  };
  
}

void loop() {
  // put your main code here, to run repeatedly:

  HTTP.handleClient();  // Работа Web сервера
  //yield();
  podzvonka(buttong);
  delay(0);
  //
  podzvonka(times);
  //

  dnsServer.processNextRequest();
  //yield();
  //handleSSDP();
  yield();
  //HTTPWAN.handleClient();
  yield();


  WiFi.setSleepMode(WIFI_NONE_SLEEP);

  //webSocket.loop(); // Работа webSocket
  //dnsServer.processNextRequest(); // Для работы DNS в режиме AP
}
