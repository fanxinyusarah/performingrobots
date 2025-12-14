#include <EnableInterrupt.h>

// ====== RC Receiver ======
#define RC_NUM_CHANNELS  6

#define RC_CH1  0
#define RC_CH2  1
#define RC_CH3  2
#define RC_CH4  3  // Forward/Backward
#define RC_CH5  4  // Left/Right
#define RC_CH6  5

#define RC_CH1_PIN  2
#define RC_CH2_PIN  3
#define RC_CH3_PIN  4
#define RC_CH4_PIN  5
#define RC_CH5_PIN  6
#define RC_CH6_PIN  7

uint16_t rc_values[RC_NUM_CHANNELS];
uint32_t rc_start[RC_NUM_CHANNELS];
volatile uint16_t rc_shared[RC_NUM_CHANNELS];

void rc_read_values() {
  noInterrupts();
  memcpy(rc_values, (const void *)rc_shared, sizeof(rc_shared));
  interrupts();
}

void calc_input(uint8_t channel, uint8_t input_pin) {
  if (digitalRead(input_pin) == HIGH) {
    rc_start[channel] = micros();
  } else {
    uint16_t rc_compare = (uint16_t)(micros() - rc_start[channel]);
    rc_shared[channel] = rc_compare;
  }
}

void calc_ch1() { calc_input(RC_CH1, RC_CH1_PIN); }
void calc_ch2() { calc_input(RC_CH2, RC_CH2_PIN); }
void calc_ch3() { calc_input(RC_CH3, RC_CH3_PIN); }
void calc_ch4() { calc_input(RC_CH4, RC_CH4_PIN); }
void calc_ch5() { calc_input(RC_CH5, RC_CH5_PIN); }
void calc_ch6() { calc_input(RC_CH6, RC_CH6_PIN); }

// ============
const int EN1 = 9;
const int EN2 = 11;

const int IN1 = 8;
const int IN2 = 7;
const int IN3 = 10;
const int IN4 = 12;

void setup() {
  Serial.begin(9600);

  pinMode(RC_CH1_PIN, INPUT);
  pinMode(RC_CH2_PIN, INPUT);
  pinMode(RC_CH3_PIN, INPUT);
  pinMode(RC_CH4_PIN, INPUT);
  pinMode(RC_CH5_PIN, INPUT);
  pinMode(RC_CH6_PIN, INPUT);

  enableInterrupt(RC_CH1_PIN, calc_ch1, CHANGE);
  enableInterrupt(RC_CH2_PIN, calc_ch2, CHANGE);
  enableInterrupt(RC_CH3_PIN, calc_ch3, CHANGE);
  enableInterrupt(RC_CH4_PIN, calc_ch4, CHANGE);
  enableInterrupt(RC_CH5_PIN, calc_ch5, CHANGE);
  enableInterrupt(RC_CH6_PIN, calc_ch6, CHANGE);

  pinMode(EN1, OUTPUT);
  pinMode(EN2, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
}

void loop() {
  rc_read_values();

  int ch4 = rc_values[RC_CH4]; // forward/backward
  int ch5 = rc_values[RC_CH5]; // steering

  int leftSpeed = 0;
  int rightSpeed = 0;

  // ---- CH4: forward,backward ----
  if (ch4 < 1510) {
    // forward: 988–1510 → 255–0
    int speed = map(ch4, 988, 1510, 255, 0);
    leftSpeed = speed;
    rightSpeed = speed;
    digitalWrite(IN1, LOW);  digitalWrite(IN2, LOW);  // L 正转
    digitalWrite(IN3, LOW);  digitalWrite(IN4, LOW);  // R 正转
  } else if (ch4 > 1530) {
    // backward: 1530–2048 → 0–255
    int speed = map(ch4, 1530, 2048, 0, 255);
    leftSpeed = speed;
    rightSpeed = speed;
    digitalWrite(IN1, HIGH); digitalWrite(IN2, HIGH);   // L 反转
    digitalWrite(IN3, HIGH); digitalWrite(IN4, HIGH);   // R 反转
  } else {
    // stop
    leftSpeed = 0;
    rightSpeed = 0;
  }

  // ---- CH5: turn ----
  if (ch5 < 1306) {
    // left: 884–1306 → 255–0
    int turnStrength = map(ch5, 884, 1306, 255, 0);
    leftSpeed = constrain(leftSpeed - turnStrength, 0, 255);
  } else if (ch5 > 1326) {
    // right: 1326–1848 → 0–255
    int turnStrength = map(ch5, 1326, 1848, 0, 255);
    rightSpeed = constrain(rightSpeed - turnStrength, 0, 255);
  }


  analogWrite(EN1, constrain(leftSpeed, 0, 255));
  analogWrite(EN2, constrain(rightSpeed, 0, 255));

  // 
  Serial.print("CH4: "); Serial.print(ch4);
  Serial.print("  CH5: "); Serial.print(ch5);
  Serial.print("  L: "); Serial.print(leftSpeed);
  Serial.print("  R: "); Serial.println(rightSpeed);

  delay(50);
}