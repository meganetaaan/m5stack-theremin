//********************************************************************
//*超音波センサを使って距離を表示するプログラム
//********************************************************************
//********************************************************************
//*超音波センサを使って距離を表示するプログラム
//********************************************************************

#include <M5Stack.h>

#define echoPin 21 // Echo Pin
#define trigPin 22 // Trigger Pin

const uint32_t maxDist = 50;
const uint32_t minDist = 5;
const uint32_t maxIdx = 88;
const uint32_t minIdx = 0;

const float KEY_A = 880.0 * 2;

boolean isPlay = false;

float note(float i) {
  return KEY_A * pow(2, (1.0 / 12.0) * (-48.0 + i));
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
}

void loop()
{
  M5.update();
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH); //超音波を出力
  delayMicroseconds(10);       //
  digitalWrite(trigPin, LOW);
  Duration = pulseIn(echoPin, HIGH); //センサからの入力
  if (M5.BtnA.wasPressed())
  {
    isPlay = !isPlay;
  }
  if (Duration > 0)
  {
    Duration = Duration / 2;                   //往復距離を半分にする
    Distance = Duration * 340 * 100 / 1000000; // 音速を340m/sに設定
    float d = floor(maxDist - _max(minDist, min(maxDist, Distance)));
    float f = note(d);
    Serial.print("note:");
    Serial.print(f);
    Serial.println(" Hz");
    if(isPlay)
    {
      M5.Speaker.tone(f);
    }
    else
    {
      M5.Speaker.mute();
    }
  }
  delay(125);
}