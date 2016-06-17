// Genuino101 Example sketch
// Written by Shiro Kiuchi, public domain
#include <math.h>
#include <ArduinoJson.h>
#include <DHT.h>
#include <Servo.h>

// Light Sensor
#define LIGHTPIN 0     // 光センサー アナログピン番号
#define LEDPIN 13      // LED デジタル出力ピン番号
#define SRVPIN 9       // サーボモータPWM デジタル出力ピン番号
#define DHTPIN 2       // DHTセンサーデジタル入力ピン番号 
#define DHTTYPE DHT11  // DHT 11 DHTセンサーのタイプ DHT11/DHT21/DHT22

int ServoPos = 0;      // サーボモータのポジション 0 - 180
float humiValue = 0;   // 湿度データ
float tempValue = 0;   // 温度データ（摂氏）
float ftempValue = 0;  // 温度データ（華氏）
int LightValue = 0;    // 光センサー値

#define MAX_RCV_BUFF 200           // UART受信バッファのサイズ
char rcvBuffer[MAX_RCV_BUFF+1];    // UART受信バッファ
int rcvCounter = 0;                // UAERT受信バッファのカウンタ

unsigned long IntervalMillis;      // センサー値処理のループインターバル（mSec)
unsigned long PreviousMillis;      // 前回のループ時の時間(mSec)

Servo myservo;                     // サーボモータドライバのインスタンス
DHT dht(DHTPIN, DHTTYPE);          // DHTドライバ（湿度、温度）のインスタンス

// Arduino IDE環境でのイニシャル処理
void setup() {
  Serial.begin(9600);
  while (!Serial) {
   ; // wait for serial port to connect. Needed for native USB port only
  }

  pinMode(LEDPIN,OUTPUT);          // 動作確認用LEDの設定 GPIO出力設定
  
  Serial.println("Genuino101 Sample program!");

  dht.begin();                     // DHT（湿度・温度)ライブラリのイニシャライズ

  myservo.attach(SRVPIN);          // サーボモータライブラリのイニシャライズ

  // 周期起動のためのタイマー値初期化
  IntervalMillis = 2000UL;         // 周期起動の時間を２秒(2000mSec)に設定
  PreviousMillis = millis();       // この時点での経過時間millis()で初期化
}

// Arduino IDE環境でのループ処理
void loop() {
  // 周期起動のための判定
  unsigned long CurrentMillis = millis();  // この時点での経過時間millis()を取得
  if ((CurrentMillis - PreviousMillis) > IntervalMillis) {
    // 前回の起動(PreviousMillis)時間から周期時間(IntervalMillis)以上経過していたら実行
    
    PreviousMillis += IntervalMillis;      // 次回の起動時間を設定
    
    digitalWrite(LEDPIN, HIGH);            // 動作確認用LED ON 

    Get_TempHumiValue();                   // 温度・湿度データ取得処理

    // Get Light sensor data 
    Get_LightValue();                      // 光センサーデータ取得処理

    SndUART();                             // センサーデータをUARTで出力
    
    digitalWrite(LEDPIN, LOW);             // 動作確認用LED OFF  
  }

  // UARTデータ受信処理
  if (RcvUART()) {
     ChkServoJSON();                       // シリアルデータを受信していれば内容を確認して処理
  }
  delay(1);                                // おまじない （一瞬でもOS?側にディスパッチする）
}


// Function: int SndUART()
// 湿度、気温、光センサーの値をJSONフォーマットでシリアル送信する。
// JSON format {"temp": float, "humi": float, "light": int, "servo": int}
void SndUART()
{
  char sndBuffer[128];
  char tempStr[6];
  char humiStr[6];
  
  memset(sndBuffer, 0, 128);
  char *json = &sndBuffer[0];

  // Arduino IDEのspintf()では%fがサポートされていないため、専用の関数 dtostrf()で
  // floatデータを文字列に変換している。
  dtostrf(tempValue,5, 1, &tempStr[0]);
  dtostrf(humiValue, 5, 1, &humiStr[0]);

  sprintf(json, "{\"temp\":%s, \"humi\":%s, \"light\":%d, \"servo\":%d}", &tempStr[0], &humiStr[0], LightValue, ServoPos);
  Serial.println(json);
}


// Function: int RcvUART()
// シリアルデータの受信を確認し、１バイトつづバッファ(rcvBuffer)に保存する。改行（'\n'）を受信した時点で、
// 受信データありとして戻り値１(retun 1)で戻る。
int RcvUART()
{
  while (1) {
    if (Serial.available() > 0) {
      char data = Serial.read();
      rcvBuffer[rcvCounter] = data;
      if (data == '\n') {
        rcvBuffer[rcvCounter+1] = '\0';
        Serial.println(rcvBuffer);
        rcvCounter = 0;
        return 1;
      } else {
        rcvCounter++;
        if (rcvCounter >= MAX_RCV_BUFF) {
          Serial.println("rcvBuffer Overflow");
          rcvCounter = 0;
          return 1; 
        }
      }    
    }
    return 0;
  }
}


// Function: ChkServoJSON(void)
// UARTで受信したデータをチェックし、正しいJSONデータの場合、サーボモータを動かす。
// サーボモータ制御のJSONは次の通り。{"servo": value}   valueは０から１８０
int ChkServoJSON()
{
  int servo;

  // UART受信バッファの内容をJSONフォマットになっているかチェック
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(&rcvBuffer[0]);
  if (!root.success()) {
    Serial.println("ChkServoJSON() parseObject() failed");
    return 0;
  }

  servo = root["servo"];                      // JSONの"servo"の値を取得

  if ((servo >= 0) && (servo <=180)) {
    ServoPos = servo;                         // サーボモータの値を保存
    myservo.write(ServoPos);                  // サーボモータを制御
  }
  return 1;
}


// Function: void Get_TemHumiValue(void) 
// 温度・湿度データ取得処理
// センサーデータを読み込みグローバル変数(humiValue、tempValue、ftempValue)に
// 値を書き込む。
void Get_TempHumiValue()
{
  humiValue = dht.readHumidity();             // 湿度データの取得
  tempValue = dht.readTemperature();          // 温度データ(摂氏)の取得
  ftempValue = dht.readTemperature(true);     // 温度データ(華氏)の取得

  // 取得したデータが正常かチェック
  if (isnan(humiValue) || isnan(tempValue) || isnan(ftempValue)) {
    Serial.println("Failed to read from DHT sensor!");
    humiValue = -1;
    tempValue = -1;
    ftempValue = -1;
    return;
  }
}


// Function: Get_LightValue(void)
// 光センサーデータ取得処理
// センサーデータを読み込みグローバル変数(LightValue)に値を書き込む。
void Get_LightValue()
{
  int LsensorValue = analogRead(LIGHTPIN);

  LightValue = (float)(1023-LsensorValue)*10/LsensorValue;
}



