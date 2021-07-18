
#include <WiFi.h>
//#include "/Applications/Arduino.app/Contents/Java/libraries/WiFi.h"
#include <WiFiClientSecure.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include "esp_camera.h"
#include <UniversalTelegramBot.h>
//#include <TelegramCertificate.h>
#include <ArduinoJson.h>
#include <Wire.h>

const char* ssid = "###########";
const char* password = "###########";
String chatId = "############";
String BOTtoken = "#############";

bool sendPhoto = false;

WiFiClientSecure clientTCP;

UniversalTelegramBot bot(BOTtoken, clientTCP);

//CAMERA_MODEL_AI_THINKER
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27

#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

#define ONE_WIRE_BUS 13

#define RELAY_1 14
#define RELAY_2 15

#define FLASH_LED_PIN 4
bool flashState = LOW;

#define BLINK_LED 12

int botRequestDelay = 1000;
long lastTimeBotRan;

void handleNewMessages(int numNewMessages);
String sendPhotoTelegram();

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
float Celcius = 0;
float Farenheit = 0;

String getReadings(){
  sensors.requestTemperatures(); 
  Celcius = sensors.getTempCByIndex(0);
  Farenheit = sensors.getTempFByIndex(0);
  String message = "Temperature: \n" + String(Celcius) + " ºC \n" + String(Farenheit) + " ºF \n";
  return message;
}

void setup() {
  // put your setup code here, to run once:
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
  Serial.begin(115200);

  pinMode(FLASH_LED_PIN, OUTPUT);
  digitalWrite(FLASH_LED_PIN, flashState);

  pinMode(RELAY_1, OUTPUT);
  pinMode(RELAY_2, OUTPUT);

  pinMode(BLINK_LED, OUTPUT);

  sensors.begin();

  WiFi.mode(WIFI_STA);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  clientTCP.setCACert(TELEGRAM_CERTIFICATE_ROOT); //add root certificate for api.telegram.org
  while (WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(500);
  }
  Serial.println("");
  Serial.println("WiFi Connected");
  Serial.println();
  Serial.print("ESP32 CAM IP Address: ");
  Serial.println(WiFi.localIP());
  
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

  //init with high specs to pre-allocate larger buffers
  if(psramFound()){
    config.frame_size = FRAMESIZE_UXGA;
    config.jpeg_quality = 10;  //0-63 lower number means higher quality
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;  //0-63 lower number means higher quality
    config.fb_count = 1;
  }
  
  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    delay(1000);
    ESP.restart();
  }

  // Drop down frame size for higher initial frame rate
  sensor_t * s = esp_camera_sensor_get();
  s->set_framesize(s, FRAMESIZE_CIF);  // UXGA|SXGA|XGA|SVGA|VGA|CIF|QVGA|HQVGA|QQVGA

}

void loop() {
  // put your main code here, to run repeatedly:
  if (sendPhoto){
    Serial.println("Capturing Photo");
    sendPhotoTelegram();
    sendPhoto = false;
    if (flashState == HIGH)
      flashState = !flashState;
    digitalWrite(FLASH_LED_PIN, flashState);
  }

  if (millis() > lastTimeBotRan + botRequestDelay){
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    while (numNewMessages){
      Serial.println("got response");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    lastTimeBotRan = millis();
  }
}

String sendPhotoTelegram(){
  const char* myDomain = "api.telegram.org";
  String getAll = "";
  String getBody = "";

  camera_fb_t * fb = NULL;
  fb = esp_camera_fb_get();  
  if(!fb) {
    Serial.println("Camera capture failed");
    delay(1000);
    ESP.restart();
    return "Camera capture failed";
  }  
  
  Serial.println("Connect to " + String(myDomain));

  if (clientTCP.connect(myDomain, 443)) {
    Serial.println("Connection successful");
    
    String head = "--ProjectJarvis\r\nContent-Disposition: form-data; name=\"chat_id\"; \r\n\r\n" + chatId + "\r\n--ProjectJarvis\r\nContent-Disposition: form-data; name=\"photo\"; filename=\"esp32-cam.jpg\"\r\nContent-Type: image/jpeg\r\n\r\n";
    String tail = "\r\n--ProjectJarvis--\r\n";

    uint16_t imageLen = fb->len;
    uint16_t extraLen = head.length() + tail.length();
    uint16_t totalLen = imageLen + extraLen;
  
    clientTCP.println("POST /bot"+BOTtoken+"/sendPhoto HTTP/1.1");
    clientTCP.println("Host: " + String(myDomain));
    clientTCP.println("Content-Length: " + String(totalLen));
    clientTCP.println("Content-Type: multipart/form-data; boundary=ProjectJarvis");
    clientTCP.println();
    clientTCP.print(head);
  
    uint8_t *fbBuf = fb->buf;
    size_t fbLen = fb->len;
    for (size_t n=0;n<fbLen;n=n+1024) {
      if (n+1024<fbLen) {
        clientTCP.write(fbBuf, 1024);
        fbBuf += 1024;
      }
      else if (fbLen%1024>0) {
        size_t remainder = fbLen%1024;
        clientTCP.write(fbBuf, remainder);
      }
    }  
    
    clientTCP.print(tail);
    
    esp_camera_fb_return(fb);
    
    int waitTime = 10000;   // timeout 10 seconds
    long startTimer = millis();
    boolean state = false;
    
    while ((startTimer + waitTime) > millis()){
      Serial.print(".");
      delay(100);      
      while (clientTCP.available()) {
        char c = clientTCP.read();
        if (state==true) getBody += String(c);        
        if (c == '\n') {
          if (getAll.length()==0) state=true; 
          getAll = "";
        } 
        else if (c != '\r')
          getAll += String(c);
        startTimer = millis();
      }
      if (getBody.length()>0) break;
    }
    clientTCP.stop();
    Serial.println(getBody);
  }
  else {
    getBody="Connected to api.telegram.org failed.";
    Serial.println("Connected to api.telegram.org failed.");
  }
  return getBody;
}

void handleNewMessages(int numNewMessages){
  Serial.print("Handle New Messages: ");
  Serial.println(numNewMessages);

  for (int i = 0; i < numNewMessages; i++){
    // Chat id of the requester
    String chat_id = String(bot.messages[i].chat_id);
    if (chat_id != chatId){
      bot.sendMessage(chat_id, "Unauthorized user", "");
      continue;
    }
    
    // Print the received message
    String text = bot.messages[i].text;
    Serial.println(text);

    String fromName = bot.messages[i].from_name;

    if (text == "/blink"){
      for (int i=0; i<5; i++){
        digitalWrite(BLINK_LED, HIGH);
        delay(500);
        digitalWrite(BLINK_LED, LOW);
        delay(500);
      }
      String mssg = "Just blinked LED 5 times!";
      bot.sendMessage(chatId, mssg, "");
    }
    if (text == "/flashclick") {
      flashState = !flashState;
      digitalWrite(FLASH_LED_PIN, flashState);
      sendPhoto = true;
      Serial.println("New flash-photo request");
    }
    if (text == "/capture") {
      sendPhoto = true;
      Serial.println("New photo request");
    }
    if (text == "/LightUp"){
      digitalWrite(RELAY_1, HIGH);
      Serial.println("Relay 1 -> High");
      bot.sendMessage(chatId, "Lamp turned on!", "Markdown");
    }
    if (text == "/LightOut"){
      digitalWrite(RELAY_1, LOW);
      Serial.println("Relay 1 -> Low");
      bot.sendMessage(chatId, "Lamp turned off!", "Markdown");
    }
    if (text == "/FanOn"){
      digitalWrite(RELAY_2, HIGH);
      Serial.println("Relay 2 -> High");
      bot.sendMessage(chatId, "Fan turned on!", "Markdown");
    }
    if (text == "/FanOff"){
      digitalWrite(RELAY_2, LOW);
      Serial.println("Relay 2 -> Low");
      bot.sendMessage(chatId, "Fan turned off!", "Markdown");
    }
    if (text == "/getstatus"){
      String conn_stat;
      if (WiFi.status() == WL_CONNECTED)
        conn_stat = "System Online! \nConnected to WiFi Network: "+String(ssid);
      else conn_stat = "system Offline!";
      bot.sendMessage(chatId, conn_stat, "Markdown");
    }
    if (text == "/readtemp"){
      Serial.println("Temperature Request");
      String readings = getReadings();
      Serial.println(readings);
      bot.sendMessage(chatId, readings, "");
    }
    if ((text == "/wake_up") || (text == "/start")){
      String welcome = "Hi! I'm Jarvis, your virtual butler.\n";
      welcome += "Here at your service with the following commands:\n";
      welcome += "/blink : _toggle on-board LED 5 times_\n";
      welcome += "/getstatus : _check connection status_\n"; 
      welcome += "/capture : _takes a new photo_\n";
      welcome += "/flashclick : _toggle flash LED while taking a new photo_\n";
      welcome += "/readtemp : _request latest temperature sensor reading_\n";
      welcome += "/LightUp : _turn on lamp_\n";
      welcome += "/LightOut : _turn off lamp_\n";
      welcome += "/FanOn : _turn on fan_\n";
      welcome += "/FanOff : _turn off fan_\n\n";
      welcome += "Enter your command.\n";
      bot.sendMessage(chatId, welcome, "Markdown");
    }
  }
}
