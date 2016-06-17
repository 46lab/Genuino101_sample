// 湿度・温度センサー、サーボモータのサンプル
#include <DHT.h>
#include <Servo.h>

#define DHTPIN 2         // DHTモジュールの信号線を接続するピン番号
#define DHTTYPE DHT11    // DHT デバイスの設定　DHT11/DHT21/DHT22

DHT dht(DHTPIN, DHTTYPE);  // DHTのインスタンスを作成

#define SRVPIN 9         // サーボモータのPWM信号線を接続するピン
Servo myservo;           // サーボモータドライバのインスタンスを作成
int servo_dir = 0;       // サーボモータ動作のためのフラグ

/////// setup()
void setup() {
  Serial.begin(9600);     // シリアルコンソールのボーレートを9600に設定
  while (!Serial) {
   ; // USB シリアルで接続している場合は、シリアルポートの接続待ちが必要（Genuino101のみ)
  }

  dht.begin();            // DHTドライバのイニシャライズ

  myservo.attach(SRVPIN);

  Serial.println("Genuino101 Temp&Humi, Servo sample sketch");
}

/////// loop()
void loop() {
  delay(2000);    // 2秒待ち DHTセンサーは、湿度、気温データの取得に約250mSecかかります。
                  // そのため、２秒周期でループを回しています。
                  
  float h = dht.readHumidity();         // 湿度データの読み込み
  float t = dht.readTemperature();      // 温度データ（摂氏）の読み込み 
  float f = dht.readTemperature(true);  // 温度データ（華氏）の読み込み

  // 正常にセンサーからデータが取り込めたか確認
  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  // シリアルコンソールに計測したデータを出力
  Serial.print("Humidity: ");
  Serial.print(h);
  Serial.print(" %\t");
  Serial.print("Temperature: ");
  Serial.print(t);
  Serial.print(" *C ");
  Serial.print(f);
  Serial.print(" *F\t");

  if (servo_dir == 0) {
    myservo.write(0);                  // サーボモータの位置を０度に
    servo_dir = 1;
  } else {
    myservo.write(180);                // サーボモータの位置を０度に
    servo_dir = 0;
  }
}

