#include <Encoder.h>
#include <Bounce.h>

// Quadrature Encoders
Encoder encoder_x(8, 9);
Encoder encoder_y(15, 14);

volatile long position_x = 0;
volatile long position_y = 0;
volatile long new_x = 0, new_y = 0;
volatile long delta_x = 0, delta_y = 0;
volatile long scroll_x = 0, scroll_y = 0;

// 4 Push buttons
const int btn1_pin = 2;
const int btn2_pin = 3;
const int btn3_pin = 6;
const int btn4_pin = 7;

Bounce btn1 = Bounce(btn1_pin, 10);
Bounce btn2 = Bounce(btn2_pin, 10);
Bounce btn3 = Bounce(btn3_pin, 10);
Bounce btn4 = Bounce(btn4_pin, 10);

// LED
const int led_pin = 17;
const int led_int_pin = 13;

volatile boolean shifted = false;

// Mouse wheel operation
const int scroll_threshold_x = 1;
const int scroll_threshold_y = 1;

// Buzzer
const int buzzer_pin = 16;

void setup() {
  pinMode(led_pin, OUTPUT);
  pinMode(led_int_pin, OUTPUT);
  digitalWrite(led_pin, HIGH);
  digitalWrite(led_int_pin, HIGH);

  pinMode(btn1_pin, INPUT_PULLUP);
  pinMode(btn2_pin, INPUT_PULLUP);
  pinMode(btn3_pin, INPUT_PULLUP);
  pinMode(btn4_pin, INPUT_PULLUP);

  shifted = false;
  digitalWrite(led_pin, shifted);

  tone(buzzer_pin, 3600, 300);
}

void loop() {
  // Upper right
  if (btn1.update()) {
    if (btn1.risingEdge()) {
      if (shifted) Mouse.release(MOUSE_BACK);
      else Mouse.release(MOUSE_LEFT);
    }
    if (btn1.fallingEdge()) {
      if (shifted) Mouse.press(MOUSE_BACK);
      else Mouse.press(MOUSE_LEFT);
    }
  }

  // Upper left
  if (btn2.update()) {
    if (btn2.risingEdge()) {
      Mouse.release(MOUSE_MIDDLE);
    }
    if (btn2.fallingEdge()) {
      Mouse.press(MOUSE_MIDDLE);
    }
  }

  // Lower right
  if (btn3.update()) {
    if (btn3.risingEdge()) {
      if (shifted) Mouse.release(MOUSE_FORWARD);
      else Mouse.release(MOUSE_RIGHT);
    }
    if (btn3.fallingEdge()) {
      if (shifted) Mouse.press(MOUSE_FORWARD);
      else Mouse.press(MOUSE_RIGHT);
    }
  }

  // Lower left
  if (btn4.update()) {
    if (btn4.risingEdge()) {

    }
    if (btn4.fallingEdge()) {
      shifted = !shifted;
      digitalWrite(led_pin, shifted);
    }
  }
  
  new_x = encoder_x.read();
  new_y = encoder_y.read();

  /*
  Serial.print("encoder_x = ");
  Serial.print(new_x);
  Serial.print(", encoder_y = ");
  Serial.println(new_y);
  */

  delta_x = new_x - position_x;
  delta_y = new_y - position_y;
  
  if (delta_x < -127) delta_x = -127;
  if (delta_x > 127) delta_x = 127;
  if (delta_y < -127) delta_y = -127;
  if (delta_y > 127) delta_y = 127;

  if (delta_x > scroll_threshold_x) scroll_x = -1;
  else if (delta_x < -scroll_threshold_x) scroll_x = 1;
  else scroll_x = 0;

  if (delta_y > scroll_threshold_y) scroll_y = -1;
  else if (delta_y < -scroll_threshold_y) scroll_y = 1;
  else scroll_y = 0;

  /*
  Serial.print("delta_x = ");
  Serial.print(delta_x);
  Serial.print(", delta_y = ");
  Serial.println(delta_y);
  */
  
  if (shifted) {
    Mouse.scroll(scroll_x, 0);
    Mouse.scroll(scroll_y, 1);
  } else {
    Mouse.move(delta_x, delta_y);
  }
  
  position_x = new_x;
  position_y = new_y;
}
