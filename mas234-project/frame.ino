#include <FlexCAN.h>
#include <SPI.h>
#include <Wire.h> 
#include <Adafruit_GFX.h> 
#include <Adafruit_SSD1306.h>

//#include <Fonts/freeMono9pt7b.h>
//const int JOY_LEFT = 18;
//const int JOY_RIGHT = 17; 
//const int JOY_CLICK 19; 
//const int JOY_UP = 22; 
//const int JOY_DOWN = 23;

#define OLED_DC     6
#define OLED_CS     10 
#define OLED_RESET  5
Adafruit_SSD1306 display(OLED_DC, OLED_RESET, OLED_CS);

#if (SSD1306_LCDHEIGHT != 64)
#error ("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif

static CAN_message_t msg,rxmsg; //Creates CAN bus message objects.
//volatile uint32:t count = 0; 
//IntervalTimer TX_timer; 
//String CANStr("");
//volatile uint32_t can_msg_count = 0; 

int countRead;  //Counter variable for number of read messages.
int countWrite; //Counter variable for number of sent messages. 
int id;         //Message ID variable.



//void fillRoundRect(uint16_t x0, uint16_t y0, uint16_t w, uint16_t h, uint16_t radius, uint16_t color); //fill the frame

void setup() {
  Can0.begin(250000); //set speed here. CAN transceiver speed set to 125kbit/s.
  //Can1.begin(250000); //set speed here. CAN transceiver speed set to 125kbit/s.

  //define message.
  msg.len = 1;   //Message lenght. 
  msg.id = 0x1A4;   //Message ID.
  msg.buf[0] = 12;  //Send data-frame.


  display.begin(SSD1306_SWITCHCAPVCC); //OLED activation.
  
}

void loop() {
  //Transmit and recive messages on CAN-bus 0. CAN-bus 0 er koblet til i teensyduino 3.6. 
  Can0.write(msg);
  Can0.read(rxmsg); 

  id = rxmsg.id;  //Store recived ID. 

  if(Can0.write(msg)) {
    countWrite++; //Counting transmitted CAN-bus messages.
    if(countWrite > 100000) {
      countWrite = 0; //Limits count max to 0. 
    }
  } 

  if(Can0.read(rxmsg)){
    countRead++;  //Counting recived CAN-bus messages.
    if(countRead > 100000){
      countRead = 0; //Limits count max to 0.
    }
  }

  //Skriver ut objektene på display. 
  //delay(1000);
  display.clearDisplay(); //Clear the buffer. 
  display.drawRoundRect(0, 0, 128, 64, 5, W); //Spesifiserer rund rektanget rundt skjermen
  display.println(F(" MAS234 - Gruppe 4"));
  display.setTextSize(0);
  display.setTextColor(WHITE);
  display.setCursor(0,5);
  display.println(" CAN-statistikk");
  display.println(" -----------------");
  display.print(" Antall sendt:   ");
  display.println(countWrite); //Henter og skriver ut sendte meldinger
  display.print(" Antall mottatt: ");
  display.println(countRead); //Henter og skriver ut motatt meldinger
  display.print(" Mottok sist ID: ");
  display.println(id); //Henter og skriver ut siste registrerte ID
  display.println(" -----------------");
  display.display();
  //display.println(" IMU-måling z: "); //Trenger ikke IMU måling da vi har gjort ping pong oppgave og ikke har loddet på IMU. 
  
}

//Draw round rectangle. 
//void drawRoundRect(uint16_t x0, uint16_t y0, uint16_t w, uint16_t h, uint16_t radius, uint16_t color); // draw a frame
