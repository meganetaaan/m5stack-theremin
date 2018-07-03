// https://www.1ft-seabass.jp/memo/2018/05/10/m5stack-meets-nodered-with-mqtt/

#include <WiFi.h>
#include <WiFiClient.h>
#include <PubSubClient.h>
#include <M5Stack.h>
#include <ArduinoJson.h>
#include <Avatar.h>
#include "const.h"

using namespace m5avatar;
 
// MQTTのポート
const int port = 1883;
// デバイスID
char *deviceID = "M5Stack";  // デバイスIDは機器ごとにユニークにします
// メッセージを知らせるトピック
char *pubTopic = "M5Stack/tone";
// メッセージを待つトピック
char *subTopic = "M5Stack/toneSrv";

char s[10];

float notes[8] = {
    261.626,
    293.665,
    329.628,
    349.228,
    391.995,
    440.000,
    493.883,
    523.251
};
 
WiFiClient httpsClient;
PubSubClient mqttClient(httpsClient);
Avatar avatar;
boolean isPub = true;
   
void setupWifi() {
    WiFi.begin(ssid, password);
   
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
 
    mqttClient.setServer(endpoint, port);
    mqttClient.setCallback(mqttCallback);
    connectMQTT();
}
   
void connectMQTT() {
    while (!mqttClient.connected()) {
        if (mqttClient.connect(deviceID)) {
            Serial.println("Connected.");
            int qos = 0;
            mqttClient.subscribe(subTopic, qos);
            Serial.println("Subscribed.");
        } else {
            Serial.print("Failed. Error state=");
            Serial.print(mqttClient.state());
            // Wait 5 seconds before retrying
            delay(5000);
        }
    }
}
   
long messageSentAt = 0;
int count = 0;
char pubMessage[128];
int led,red,green,blue;
float lastFreq = 0;
   
void mqttCallback (char* topic, byte* payload, unsigned int length) {
 
    String str = "";
    Serial.print("Received. topic=");
    Serial.println(topic);
    for (int i = 0; i < length; i++) {
        Serial.print((char)payload[i]);
        str += (char)payload[i];
    }
    Serial.print("\n");
 
    StaticJsonBuffer<200> jsonBuffer;
     
    JsonObject& root = jsonBuffer.parseObject(str);
   
    // パースが成功したか確認。できなきゃ終了
    if (!root.success()) {
      Serial.println("parseObject() failed");
      return;
    }
    delay(300);
}
  
void mqttLoop() {
    if (!mqttClient.connected()) {
        connectMQTT();
    }
    mqttClient.loop();
}

void publishFreq(float f)
{
  sprintf(pubMessage, "{\"frequency\": %f}", f);
  mqttClient.publish(pubTopic, pubMessage);
}

//********************************************************************
//*超音波センサを使って距離を表示するプログラム
//********************************************************************

#include <M5Stack.h>

#define echoPin 21 // Echo Pin
#define trigPin 22 // Trigger Pin

const uint32_t maxDist = 80;
const uint32_t minDist = 5;
const uint32_t maxIdx = 88;
const uint32_t minIdx = 0;

const float KEY_A = 440;

boolean isPlay = false;

float note_(double d) {
  float i = floor(maxDist - _max(minDist, min(maxDist, d))) / 3 + 50;
  return KEY_A * pow(2, (1.0 / 12.0) * (-48.0 + i));
}

float note(double d) {
    d = floor(maxDist - _max(minDist, min(maxDist, d)));
    int i = floor((d - minDist) * 8 / (maxDist - minDist));
    return notes[i];
}

double Duration = 0; //受信した間隔
double Distance = 0; //距離
void setup()
{
  M5.begin();
  pinMode(echoPin, INPUT);
  pinMode(trigPin, OUTPUT);
  M5.Speaker.setVolume(1);
  M5.Speaker.update();
  setupWifi();
  M5.Lcd.drawString("OK", 0, 0);
  avatar.init();
}

void updateEcho()
{
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH); //超音波を出力
  delayMicroseconds(10);       //
  digitalWrite(trigPin, LOW);
}

void loop()
{
  M5.update();

  // 常にチェックして切断されたら復帰できるように
  mqttLoop();

  updateEcho();
  Duration = pulseIn(echoPin, HIGH); //センサからの入力
  if (M5.BtnA.wasPressed())
  {
    isPlay = !isPlay;
  }
  if (M5.BtnB.wasPressed())
  {
      isPub = !isPub;
  }
  if (Duration > 0)
  {
    Duration = Duration / 2;                   //往復距離を半分にする
    Distance = Duration * 340 * 100 / 1000000; // 音速を340m/sに設定
    float f = note(Distance);
    Serial.print("note:");
    Serial.print(f);
    Serial.println(" Hz");
    float m = _max(0.1, (f - 261.626) / (523.251 - 261.626));
    avatar.setMouthOpenRatio(m);
    if (isPlay && lastFreq != f)
    {
        sprintf(s, "%.0fHz", f);
        avatar.setSpeechText(s);
        if (isPub)
        {
            publishFreq(f);
        }
        else
        {
            M5.Speaker.tone(f);
        }
    }
    if (!isPlay)
    {
      avatar.setSpeechText("");
      M5.Speaker.mute();
    }
    lastFreq = f;
  }
  delay(125);
}