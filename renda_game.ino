#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
#define I2C_ADDRESS   0x3C

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

const int buttonPin1 = 2; // 割り込みピン1
const int buttonPin2 = 3; // 割り込みピン2

volatile int tapCount = 0;   // タップ回数（割り込みで使用するためvolatile）
bool gameStarted = false;
bool gameEnded = false;
unsigned long startTime = 0;
const int gameDuration = 10000; // 10秒
const int longPressDuration = 2000; // 長押し時間（2秒）
unsigned long pressStartTime = 0;
unsigned long lastTapTime1 = 0; // ボタン1の最後のタップ時間
unsigned long lastTapTime2 = 0; // ボタン2の最後のタップ時間
const unsigned long debounceDelay = 50; // デバウンス遅延時間（50ms）

void setup() {
  Serial.begin(9600);
  if (!display.begin(SSD1306_SWITCHCAPVCC, I2C_ADDRESS)) {
    Serial.println(F("SSD1306の初期化に失敗しました"));
    for (;;);
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  pinMode(buttonPin1, INPUT_PULLUP);
  pinMode(buttonPin2, INPUT_PULLUP);

  // 割り込み設定
  attachInterrupt(digitalPinToInterrupt(buttonPin1), countTap1, FALLING);
  attachInterrupt(digitalPinToInterrupt(buttonPin2), countTap2, FALLING);

  showHomeScreen();
}

void loop() {
  if (!gameStarted && (digitalRead(buttonPin1) == LOW || digitalRead(buttonPin2) == LOW)) {
    delay(200);
    startCountdown();
  }
  
  if (gameStarted && !gameEnded) {
    showRemainingTime();
    
    if (millis() - startTime >= gameDuration) {
      gameEnded = true;
      showScore();
    }
  }

  if (gameEnded) {
    if ((digitalRead(buttonPin1) == LOW || digitalRead(buttonPin2) == LOW) && pressStartTime == 0) {
      pressStartTime = millis();
    } else if ((digitalRead(buttonPin1) == HIGH && digitalRead(buttonPin2) == HIGH) && pressStartTime != 0) {
      pressStartTime = 0;
    }
    
    if (pressStartTime != 0 && millis() - pressStartTime >= longPressDuration) {
      resetGame();
    }
  }
}

void showHomeScreen() {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Tap to Start Game");
  display.display();
}

void startCountdown() {
  display.clearDisplay();
  for (int i = 3; i > 0; i--) {
    display.setCursor(0, 0);
    display.clearDisplay();
    display.print("Starting in: ");
    display.println(i);
    display.display();
    delay(1000);
  }
  gameStarted = true;
  gameEnded = false;
  tapCount = 0;
  startTime = millis();
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Start tapping!");
  display.display();
}

void showRemainingTime() {
  unsigned long elapsed = millis() - startTime;
  int remainingTime = (gameDuration - elapsed) / 1000;
  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("Time: ");
  display.print(remainingTime);
  display.print("s");
  display.setCursor(0, 16);
  display.print("Taps: ");
  display.println(tapCount);
  display.display();
}

void showScore() {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("Time's up!");
  display.setCursor(0, 16);
  display.print("Score: ");
  display.println(tapCount);
  display.setCursor(0, 32);
  display.println("Hold to go home");
  display.display();
}

void resetGame() {
  gameStarted = false;
  gameEnded = false;
  pressStartTime = 0;
  showHomeScreen();
}

// 割り込みによるタップカウント処理（チャタリング除去）
void countTap1() {
  unsigned long currentTime = millis();
  if (currentTime - lastTapTime1 > debounceDelay) {
    tapCount++;
    lastTapTime1 = currentTime;
  }
}

void countTap2() {
  unsigned long currentTime = millis();
  if (currentTime - lastTapTime2 > debounceDelay) {
    tapCount++;
    lastTapTime2 = currentTime;
  }
}
