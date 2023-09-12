#include <Wire.h>                  // For I2C communication
#include <Adafruit_LIS3DH.h>       // For accelerometer
#include <Adafruit_Protomatter.h>  // For RGB matrix
#include <Adafruit_PixelDust.h>
#include "vars.h"

Adafruit_Protomatter matrix(
  WIDTH, 4, 1, rgbPins, NUM_ADDR_PINS, addrPins,
  clockPin, latchPin, oePin, true);

Adafruit_LIS3DH accel = Adafruit_LIS3DH();

void resetGame(){
  player.score = 0;
  oppo.score = 0;
  player.y = HEIGHT/2-BOX_HEIGHT;
  ball.x =45;
  ball.right = false;
}


void err(int x) {
  uint8_t i;
  pinMode(LED_BUILTIN, OUTPUT);        // Using onboard LED
  for (i = 1;; i++) {                  // Loop forever...
    digitalWrite(LED_BUILTIN, i & 1);  // LED on/off blink to alert user
    delay(x);
  }
}

void setup() {
  // put your setup code here, to run once:
  ProtomatterStatus status = matrix.begin();
  if (!accel.begin(0x19)) {
    Serial.println("Couldn't find accelerometer");
    err(250);  // Fast bink = I2C error
  }
  accel.setRange(LIS3DH_RANGE_2_G);  // 2, 4, 8 or 16 G!
  accel.setClick(2, CLICKTHRESHHOLD);
}

void loop() {
  // Limit the animation frame rate to MAX_FPS.  Because the subsequent sand
  // calculations are non-deterministic (don't always take the same amount
  // of time, depending on their current states), this helps ensure that
  // things like gravity appear constant in the simulation.
  uint32_t t;
  while (((t = micros()) - prevTime) < (1000000L / MAX_FPS))
    ;
  prevTime = t;

  matrix.fillScreen(0x0);  // blank the screen
  switch (screen) {
    case CurrentScreen::Title:  //menu
      matrix.setCursor(15, 10);
      matrix.setTextSize(0);
      matrix.println("PONG!");
      resetGame();
      uint8_t tap;
      tap = accel.getClick();
      // if (tap == 0) return;
      //if (!(tap & 0x30)) return;
      if (tap &0x10) screen = CurrentScreen::Game; //single tap
      if (tap & 0x20) screen = CurrentScreen::Game;  //double tap
      break;

    case CurrentScreen::Game:  //game


      //----player interface-----//
      sensors_event_t event;
      accel.getEvent(&event);
      player.y += event.acceleration.y;

      //-----player bounds-----------|
      if (player.y > HEIGHT - 8) {  //|
        player.y = HEIGHT - 8;      //|
      }                             //|
      if (player.y < 0) {           //|
        player.y = 0;               //|
      }                             //|
      //---------------------------//|

      //-------Ball Physics(hehehe)--------------------//|
      if (ball.right) {
        ball.x++;
      } else {
        ball.x--;
      }
      if (ball.down) {
        ball.y++;
      } else {
        ball.y--;
      }
      if (ball.x == player.x + BOX_WIDTH && player.y < ball.y + BALL_SIZE && player.y + BOX_HEIGHT > ball.y) {
        ball.right = true;
      }  //if player paddle touches the ball, change directions
      if (ball.x + BALL_SIZE == oppo.x && oppo.y < ball.y + BALL_SIZE && oppo.y + BALL_SIZE > ball.y) {
        ball.right = false;
      }  //if opponent touches the ball, change directions
      if (ball.y == 0) {
        ball.down = true;
        /*if (random(0,3)==1){
          ball.right=true;
        } else {ball.right=false;}*/
      }
      if (ball.y == HEIGHT - BALL_SIZE) {
        ball.down = false;
        /*if (random(0,3)==1){
          ball.right=true;
        }
        else {ball.right=false;}*/
      }
      //-----------------------------------------------------------------------------------------------------------------|


      //-------------scoring---------------------------|
      if (ball.x + BALL_SIZE <= 0) {
        oppo.score++;
        ball.x = 29;
      }
      if (ball.x >= 64) {
        player.score++;
        ball.x=29;
      }
      if (oppo.score == 11){
        screen = CurrentScreen::Gameover;
      }
      if (player.score == 11){
        screen = CurrentScreen::Win;
      }
      //-------------------------------------------------|

      //-----------Opponent "AI" --------------------|
      if (ball.x > 40 || random(0, 20) == 1) {
        if (ball.y < oppo.y || random(0, 20)==8 &&oppo.y>1)
          oppo.y--;
        if (ball.y + BALL_SIZE > oppo.y + BOX_HEIGHT || random(0,20)==7 && oppo.y<HEIGHT-BOX_HEIGHT)
          oppo.y++;
      }
      //---------------------------------------------|

      //draw objects
      matrix.setCursor(10, 0);
      matrix.println(player.score);
      matrix.setCursor(49, 0);
      matrix.print(oppo.score);
      matrix.drawFastVLine(WIDTH/2, 0, 32, WHITE);
      matrix.fillRect(player.x, player.y, BOX_WIDTH, BOX_HEIGHT, 0XFFFF);
      matrix.fillRect(ball.x, ball.y, BALL_SIZE, BALL_SIZE, ball.color);
      matrix.fillRect(oppo.x, oppo.y, BOX_WIDTH, BOX_HEIGHT, WHITE);
      
      break;

    case CurrentScreen::Gameover:  //gameover
      matrix.setCursor(0, 0);
      matrix.print("HAHA! \nI WIN, \nLOSER!");
      //matrix.println("Tap to play again")
      
      tap = accel.getClick();
      // if (tap == 0) return;
      //if (!(tap & 0x30)) return;
      if (tap &0x10) screen = CurrentScreen::Title; //single tap
      if (tap & 0x20) screen = CurrentScreen::Title;  //double tap
      break;

    case CurrentScreen::Win:  //winner
      matrix.setCursor(0, 8);
      matrix.println("YOU FUCKIN DID IT,\n CHAMP!");
     
      tap = accel.getClick();
      // if (tap == 0) return;
      //if (!(tap & 0x30)) return;
      if (tap &0x10) screen = CurrentScreen::Title; //single tap
      if (tap & 0x20) screen = CurrentScreen::Title;  //double tap
      break;
  }
  matrix.show();  //show objects
}