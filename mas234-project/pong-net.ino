/* Teensy CAN-Bus with OLED 128x64 demo
 *  
 * www.skpang.co.uk
 * 
 * V1.0 July 2017
 *  
 * For use with Teensy 3.6 Dual CAN-Bus Breakout board:
 * http://skpang.co.uk/catalog/teensy-36-dual-canbus-breakout-board-include-teensy-36-p-1532.html
 * requires OLED display
 * http://skpang.co.uk/catalog/oled-128x64-display-for-teensy-breakout-board-p-1508.html
 * 
 *  
 */
#include <FlexCAN.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Fonts/FreeMonoBoldOblique18pt7b.h>

const int JOY_LEFT = 18;
const int JOY_RIGHT = 17;
const int JOY_CLICK = 19;
const int JOY_UP = 22;
const int JOY_DOWN = 23;

#define SSD1306_LCDHEIGHT 64
#define OLED_DC     6
#define OLED_CS     10
#define OLED_RESET  5
Adafruit_SSD1306 display(OLED_DC, OLED_RESET, OLED_CS);

#if (SSD1306_LCDHEIGHT != 64)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif

static CAN_message_t msg,rxmsg;

bool master = false;

int group_id = 4;
unsigned int ball_id = 50 + group_id;
unsigned int bar_id = 20 + group_id;
int master_id = 0;

int BallPosX = 90; // initial for master
float BallPosY = 30; // initial for master
int BarPosY = ((64-20)/2); // y-pos of our bar, initial position.
int RemoteBarPosY = ((64-20)/2); // y-pos remote bar

int dir_x = 1; 
float dir_y = 1;
int step_x = 1;
float step_y = 0.5;


int remote_score = 0;
int local_score = 0;

unsigned long wait = millis();
unsigned long last = millis();


void setup(){

  // CAN setup

  Can0.begin(500000); //set speed here. 
  Can1.begin(500000); //set speed here.  
  
  pinMode(JOY_LEFT, INPUT_PULLUP);
  pinMode(JOY_RIGHT, INPUT_PULLUP);
  pinMode(JOY_CLICK, INPUT_PULLUP);
  pinMode(JOY_UP, INPUT_PULLUP);
  pinMode(JOY_DOWN, INPUT_PULLUP); 
  
  // by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
  display.begin(SSD1306_SWITCHCAPVCC);
  // Clear the buffer.
  display.clearDisplay();
  
  delay(200);
  Serial.println(F("Two-player PONG Game using CAN Bus and 128x64 px OLED screen"));

  display.fillRect(0,0,128, 64, WHITE);
  display.setFont(&FreeMonoBoldOblique18pt7b);
  display.setTextColor(BLACK);
  display.setTextSize(0);
  display.setCursor(10,30); // x,y offset
  display.print("PONG!");
  display.setFont(NULL);
  display.setCursor(50,50);
  display.print("Loading...");
  display.display();

  delay(1000);


  display.setFont(NULL); // default 5x7 system font
  display.fillRect(0,0,128, 64, WHITE); // draw black screen ..fillRect(x, y, width, height, color) 
  display.setTextSize(0);
  display.setTextColor(BLACK);
  display.setCursor(10,10); // x,y offset
  display.println(" Connect player 2!");
  display.println(" ");
  display.println(" then press Joystick");
  display.println(" ");
  display.println("  to start Pong!");
  display.display();

  delay(1000);
  
  display.clearDisplay();

 
}

void loop() {

  last = millis();
  
  // should have programmed this with some handshake functionality, setting up the environment with consensus between devices, like; how are we communicating.
  // We could / should also use a CPU clocked timer here
  
  /*while (Can0.available()) { // available == finnes en melding. Connection exists?? 
     // wait at start-screen for CAN connection
  }*/
  
  
  if (master == false && digitalRead(JOY_CLICK) == LOW && master_id == 0) {
    // send master start signal! send our group number in master message
    master_id = group_id;
    msg.id = 0; 
    msg.len = 1;
    msg.buf[0] = master_id;
    Can0.write(msg);
    Can1.write(msg);
    master = true;
    Serial.println("We are master");
  }
  
  
  // clear display everything
  display.fillRect(0,0,128, 64, BLACK);


  // if master == none.

  if ((remote_score > (local_score + 1) && remote_score > 10) || ((remote_score + 1) < local_score && local_score > 10)) {
      // wait for joy click to become LOW to resume the game.
      while (digitalRead(JOY_CLICK) == HIGH) {
        
        display.fillRect(0,0,128, 64, BLACK);
        display.setTextSize(2);
        display.setTextColor(WHITE);
        display.setCursor(0,10);
        if (local_score > remote_score) {
          display.println("YOU WIN!");          
        }
        if (local_score < remote_score) {
          display.println("YOU LOOSE!");          
        }
        display.setTextSize(0);
        display.print("   Score:   ");
        display.print(remote_score);
        display.print(" - ");
        display.println(local_score);
        display.println(" ");
        display.println(" Press Joystick ");
        display.println("to start new game ");
        display.display();      
        display.fillRect(0,0,128, 64, BLACK);  
      }
      _reboot_Teensyduino_();  // quick hack to restart game
  }
  
  if (master == true) {

    
    // Ball movemenment
    BallPosX = BallPosX + (step_x * dir_x);
    BallPosY = float(BallPosY) + (step_y * dir_y);


    if (BallPosY <= 0) {
      dir_y = 1;
    }
    if (BallPosY > 64) {
      dir_y = -1;
    }

    if (BallPosX >= 125) {
      if (BallPosY > BarPosY && BallPosY < (BarPosY + 20)) {
        // ball is within bar position, reverse x direction.
        dir_x = -1;
        if (BallPosY < (BarPosY + 10)) {
          // switch upper/lower bar y-direction based on bar-halves
          dir_y = -1;
          step_y = float(float(newrandom(2,5)) / 10);
        }
        if (BallPosY > (BarPosY + 10) && BallPosY < (BarPosY + 20)) {
          dir_y = 1;
          step_y = float(float(newrandom(2,5)) / 10);
        }
      } else {
        // outside of bounds - game over! 1 point to remote.
        remote_score++;
        // reset ball, give ball to looser, and center bar position. Wait for joystick click to resume.
        BallPosX = 84;
        BallPosY = 30;
        step_y = float(float(newrandom(2,5)) / 10);
        if (round(float(step_y) * 2) == 1) { dir_y = -1; } else { dir_y = 1; }
        dir_x = 1;
        
        // wait for joy click to become LOW to resume the game.
        if ((remote_score > (local_score + 1) && remote_score > 10) || ((remote_score + 1) < local_score && local_score > 10)) {
          // remote win
        } else {
          
          // 2 second timeout scoreboard.
          display.fillRect(0,0,128, 64, BLACK);
          display.setTextSize(0);
          display.setTextColor(WHITE);
          display.setCursor(40,10);
          display.println(" ");
          display.println("They score!");
          display.println(" ");
          display.println(" Press Joystick ");
          display.println(" to serve ball");
          display.display();
          display.fillRect(0,0,128, 64, BLACK);
          wait = millis();
          while ((wait + 2000) > millis()) {
            // wait 2 seconds
          }          

        }

      }
    }
    
    if (BallPosX <= 3) {
      if (BallPosY > RemoteBarPosY && BallPosY < (RemoteBarPosY + 20)) {
        // ball is within bar position, reverse direction.
        dir_x = 1;
        //if (dir_y == 1) dir_y = -1;
        //if (dir_y == -1) dir_y = 1;

        if (BallPosY < (RemoteBarPosY + 10)) {
          // upper bar
          dir_y = -1;
          step_y = float(float(newrandom(2,5)) / 10);
        }
        if (BallPosY > (RemoteBarPosY + 10) && BallPosY < (RemoteBarPosY + 20)) {
          dir_y = 1;
          step_y = float(float(newrandom(2,5)) / 10);
        }

      } else {
        // game over! 1 point to local
        local_score++;

        if ((remote_score > (local_score + 1) && remote_score > 10) || ((remote_score + 1) < local_score && local_score > 10)) {
          // local win
        } else {


          
          // 2 second timeout scoreboard.
          display.fillRect(0,0,128, 64, BLACK);
          display.setTextSize(0);
          display.setTextColor(WHITE);
          display.setCursor(40,10);
          display.println(" ");
          display.println("You score!");
          display.println(" ");
          display.println("Waiting on serve");
          display.display();      
          display.fillRect(0,0,128, 64, BLACK);
          wait = millis();
          while ((wait + 2000) > millis()) {
            // wait 2 seconds
          }          
          
        }
        
        BallPosX = 44;
        BallPosY = 30;
        step_y = float(float(newrandom(2,5)) / 10);
        dir_x = -1;
        if (round(float(step_y) * 2) == 1) { dir_y = -1; } else { dir_y = 1; }
      }

    }

  // report the ball position to the network.

  msg.len = 2;
  msg.id = ball_id; // ball id 036h = 54 d
  msg.buf[0] = BallPosX; // x pos
  msg.buf[1] = BallPosY; // y pos
  Can0.write(msg);
  Can1.write(msg);

  msg.len = 1;
  msg.id = bar_id; // bar id 018h = 24 d
  msg.buf[0] = BarPosY; // y pos 00 - 2C
  Can0.write(msg);
  Can1.write(msg);  

  }


  // if not master, get the position of ball and bar.


  while(Can0.read(rxmsg))
  { 

     // we recieve ball position since we're not master. check that its not ourselves (loopback err)
     //if (!master) - could add extra local control, incase they do something wrong.

     if (master == false && rxmsg.id == 0) {
        // we are not master, they can take master.
        master_id = rxmsg.buf[0];
        master = false;
     }

     if (master == false && master_id > 0) {
       if (rxmsg.id > 50 && rxmsg.id != ball_id) {
        /*
        Serial.print("Remote ball ID: ");
        Serial.print(rxmsg.id);
        Serial.print(" x/y pos: ");
        Serial.print(rxmsg.buf[0]);
        Serial.print(" / ");
        Serial.print(rxmsg.buf[1]);
        Serial.println(" ");

        */
        
        BallPosX = (128 - rxmsg.buf[0]); // reversed for the reported perspective
        BallPosY = rxmsg.buf[1];
       }
     }


     // if it is not our bar, it has to be the opponent.
     if (rxmsg.id > 20 && rxmsg.id < 50 && rxmsg.id != bar_id) {
      RemoteBarPosY = rxmsg.buf[0];
      /*Serial.print("Remote bar y pos: ");
      Serial.print(rxmsg.buf[0]);  
      Serial.println(" "); */

     }
     
  }


  if ((master == false) && (master_id > 0)) {


  
    if (BallPosX >= 125) {
      if (BallPosY >= BarPosY && BallPosY <= (BarPosY + 20)) {
      } else {
        Serial.println("REMOTE score ...?!");
        remote_score++;
        if ((remote_score > (local_score + 1) && remote_score > 10) || ((remote_score + 1) < local_score && local_score > 10)) {
          // remote win
        } else {

          // 2 second timeout scoreboard.
          display.fillRect(0,0,128, 64, BLACK);
          display.setTextSize(0);
          display.setTextColor(WHITE);
          display.setCursor(40,10);
          display.println(" ");
          display.println("They score!");
          display.println(" ");
          display.println(" Press Joystick ");
          display.println(" to serve ball");
          display.display(); 
          display.fillRect(0,0,128, 64, BLACK);
          wait = millis();
          while ((wait + 2100) > millis()) {
            // wait 2 seconds
          }          

          
        }
      }
    }
    
    if (BallPosX <= 3) {
      if (BallPosY >= RemoteBarPosY && BallPosY <= (RemoteBarPosY + 20)) {
      } else {
        Serial.println("LOCAL score ...?!");
        local_score++;
        if ((remote_score > (local_score + 1) && remote_score > 10) || ((remote_score + 1) < local_score && local_score > 10)) {
          // local win
        } else {


          // 2 second timeout scoreboard.
          display.fillRect(0,0,128, 64, BLACK);
          display.setTextSize(0);
          display.setTextColor(WHITE);
          display.setCursor(40,10);
          display.println(" ");
          display.println("You score!");
          display.println(" ");
          display.println("Waiting on serve");
          display.display();
          display.fillRect(0,0,128, 64, BLACK);
          wait = millis();
          while ((wait + 2050) > millis()) {
            // wait 2 seconds
          }
  

        }

        
      }
    }


  msg.len = 1;
  msg.id = bar_id; // bar id 018h = 24 d
  msg.buf[0] = BarPosY; // y pos 00 - 2C
  Can0.write(msg);
  Can1.write(msg);  

    
  }



  // move our bar
  if (digitalRead(JOY_DOWN) == HIGH) {
      if (BarPosY > 0) {
        BarPosY = BarPosY - 1; // y-pos of our bar, initial position.
      }
  }
  if (digitalRead(JOY_UP) == HIGH) {
    if (BarPosY < (64-20)) {
      BarPosY = BarPosY + 1; // y-pos of our bar, initial position.
    }
  }




  // draw mid net
  for (int i=4; i < 64; i = i+8) {
      display.drawFastVLine(128/2, i, 4, WHITE);
  }

  // draw the score
  display.setTextColor(WHITE);
  display.setCursor(((128/2)-14),0); 
  display.print(remote_score);
  display.setCursor(((128/2)+8),0); 
  display.print(local_score);


  // draw the ball position and local + remote bar 
  display.fillCircle(BallPosX, BallPosY, 2, WHITE); // ball
  display.fillRect(0,RemoteBarPosY,3,20, WHITE); // left bar (remote - opponent) initial Y-position center.
  display.fillRect((128-3),BarPosY,3,20, WHITE); // our right bar, initial Y-pos center.


  
  display.display(); // show everything
  
  
  while (millis() < (last + 10)) {
    // wait for 10 ms since loop start
  }

}


unsigned long newrandom(unsigned long howsmall, unsigned long howbig) {
 return howsmall + random() % (howbig - howsmall);
}
