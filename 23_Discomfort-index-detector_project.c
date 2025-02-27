#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include <Servo.h>

#define DHTPIN 7      // DHT 센서의 데이터 핀
#define DHTTYPE DHT11 // DHT11 센서를 사용하므로 DHT11로 설정

DHT_Unified dht(DHTPIN, DHTTYPE);

#define RED_PIN 9    // 빨간색 LED 핀
#define GREEN_PIN 10  // 초록색 LED 핀
#define BLUE_PIN 11   // 파란색 LED 핀

Servo servoMotor;   // 서보모터 객체 생성

// LCD 설정
LiquidCrystal_I2C lcd(0x27, 16, 2);

const int SERVO_THRESHOLD = 70; // 불쾌지수가 이 값보다 높을 때만 서보모터를 돌림

void setup() {
  Serial.begin(9600);

  // DHT 센서 초기화
  dht.begin();

  // RGB LED 핀을 출력으로 설정
  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);

  // Servo 모터 핀 설정
  servoMotor.attach(6); // 사용할 핀 번호 지정

  // LCD 초기화
  lcd.begin(16, 2);
  lcd.backlight();

  // 초기화된 LCD에 화면 표시
  lcd.setCursor(0, 0);
  lcd.print("Temp: ");

  lcd.setCursor(0, 1);
  lcd.print("Humidity: ");

  lcd.setCursor(0, 2);
  lcd.print("Discom: ");
}

void loop() {
  // 센서 데이터를 읽어오기
  sensors_event_t event;
  dht.temperature().getEvent(&event);
  float temperature = isnan(event.temperature) ? 0 : event.temperature;

  dht.humidity().getEvent(&event);
  float humidity = isnan(event.relative_humidity) ? 0 : event.relative_humidity;

  // 불쾌지수 계산
  float discomfortIndex = 0.81 * temperature + 0.01 * humidity * (0.99 * temperature - 14.3) + 46.3;

  // 불쾌지수를 퍼센트로 변환
  int discomfortPercentage = map(discomfortIndex, 0, 100, 0, 100);

  // LCD에 온도, 습도, 불쾌지수 표시
  lcd.setCursor(6, 0);
  lcd.print(temperature, 1);
  lcd.print(" C");

  // LCD의 두 번째 줄에 습도 표시 및 문자 이동
  lcd.setCursor(0, 1);
  lcd.print("Humidity: ");
  lcd.print(humidity, 1);
  lcd.print(" %   ");

  // LCD의 세 번째 줄에 불쾌지수 표시 및 문자 이동
  lcd.setCursor(0, 2);
  lcd.print("Discom: ");
  lcd.print(discomfortPercentage);
  lcd.print("%   ");

  // 문자 이동
  scrollText();

  // 시리얼 출력
  Serial.print("Discomfort Index: ");
  Serial.print(discomfortPercentage);
  Serial.println("%");

  // 불쾌지수에 따른 LED 색상 설정
  if (discomfortPercentage <= 20) {
    setColor(0, 0, 255);  // 파랑
  } else if (discomfortPercentage <= 68) {
    setColor(0, 255, 0);  // 초록
  } else {
    setColor(255, 0, 0);  // 빨강

    // 불쾌지수가 일정 값보다 높을 때만 서보모터 동작
    if (discomfortPercentage > SERVO_THRESHOLD) {
      // 서보모터를 계속 무한히 회전
      for (int angle = 0; angle <= 180; angle += 1) {
        moveServo(angle);
      }
    }
  }

  delay(1000);  // 1초에 한 번 측정
}

// RGB LED 색상 설정 함수
void setColor(int red, int green, int blue) {
  analogWrite(RED_PIN, red);
  analogWrite(GREEN_PIN, green);
  analogWrite(BLUE_PIN, blue);
}

// Servo 모터 회전 함수
void moveServo(int angle) {
  servoMotor.write(angle);
  delay(15); // 서보모터가 안정화되기를 기다림
}

// LCD 문자 이동 함수
void scrollText() {
  static int offset = 0;
  lcd.scrollDisplayLeft();
  offset++;
  if (offset >= 16) {
    offset = 0;
    lcd.setCursor(0, 1);
    delay(50);
  }
}