//include libraries
#include <U8glib.h>
#include <RtcDS3231.h>
#include <Wire.h>
#include <Stepper.h>
#include <Keypad.h>

//define keypad rows and columns
const int ROW_NUM = 4;
const int COLUMN_NUM = 4;
char keys[ROW_NUM][COLUMN_NUM] = {
  {'1', '2', '3', 'a'},
  {'4', '5', '6', 'b'},
  {'7', '8', '9', 'c'},
  {'*', '0', '#', 'd'}
};
byte pin_rows[ROW_NUM] = {9, 8, 7, 6};
byte pin_column[COLUMN_NUM] = {5, 4, 3, 2};

// Create variables
byte hours;
byte minutes;
byte seconds;
int motionSensorCount;
unsigned long timestamp;
const int stepsPerRevolution = 2048;
long hours_in1;
long minutes_in1;
long hours_in2;
long minutes_in2;
long hours_in3;
long minutes_in3;
String didgitsString;
long didgitsInt;


// create input dependant booleans
bool time1() {
  return (hours == hours_in1 && minutes == minutes_in1 && seconds == 0);
}
bool time2() {
  return (hours == hours_in2 && minutes == minutes_in2 && seconds == 0);
}
bool time3() {
  return (hours == hours_in3 && minutes == minutes_in3 && seconds == 0);
}

//define oled
U8GLIB_SSD1306_128X64 oled(U8G_I2C_OPT_NONE);

//define rtc mocule (clock)
RtcDS3231<TwoWire> rtcModule(Wire);

//define stepper and setting pins and steps per revolution
Stepper myStepper(stepsPerRevolution, 10, 11, 12, 13);

//define keypad
Keypad keypad = Keypad( makeKeymap(keys), pin_rows, pin_column, ROW_NUM, COLUMN_NUM );

void setup() {
  // Begin serial
  Serial.begin(9600);

  // Begin rtcmodule
  rtcModule.Begin();

  // Update RTC module time to compilation time
  RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
  rtcModule.SetDateTime(compiled);

  //set stepper speed
  myStepper.setSpeed(16);

  // limit time digits to 2 digits
  didgitsString.reserve(2);
}

void loop(void) {
  //Update time from clock modlue
  rtcUpdate();
  
  //run motionsensorfunction
  motionsensor();

  // Refresh picture
  oled.firstPage();
  do {
    draw();
  } while (oled.nextPage());

  //run stepperfunction
  movestepper();
  
  //run input function
  keypadinput();
}

//update time
void rtcUpdate(void) {
  RtcDateTime now = rtcModule.GetDateTime();
  hours = now.Hour();
  minutes = now.Minute();
  seconds = now.Second();
}

//oled function
void draw(void) {
  // Set font to Helvetica size 10
  oled.setFont(u8g_font_helvB10);
  //print strings and variables
  oled.drawStr(0, 15, String(String(hours) + ":" + String(minutes) + ":" + String(seconds) + "[Log:" + String(motionSensorCount) + "]").c_str());
  oled.drawStr(0, 30, String("Time 1--" + String(hours_in1) + ":" + String(minutes_in1)).c_str());
  oled.drawStr(0, 45, String("Time 2--" + String(hours_in2) + ":" + String(minutes_in2)).c_str());
  oled.drawStr(0, 60, String("Time 3--" + String(hours_in3) + ":" + String(minutes_in3)).c_str());
}

//function to turn off the stepper after it moves to avoid overheating
void turnoffstepper(void) {
  digitalWrite(10, LOW);
  digitalWrite(11, LOW);
  digitalWrite(12, LOW);
  digitalWrite(13, LOW);
}

//stepper function, if any of time boleans are true the stepper turns 1/4 revolution
void movestepper(void) {
  if (time1() || time2() || time3()) {
    Serial.println("Feed_cat");
    myStepper.step(stepsPerRevolution / 4);
    turnoffstepper();
  }
}

//motionsensor function, if the sensor sends a signal and its been 30s since last signal increase the counter by 1
void motionsensor(void) {
  if (digitalRead(1) == HIGH && (millis() - timestamp) > 30000) {
    motionSensorCount += 1;
    timestamp = millis();
  }
}

//function to input time into the time variables, after inputing a 2 digit number press a corresponding key to set it to a variable
void keypadinput(void) {
  char key = keypad.getKey();
  if (key) {
    if (key >= '0' && key <= '9') {
      didgitsString += key;
      didgitsInt = didgitsString.toInt();
    }
    else if (key == '#') {
      hours_in1 = didgitsInt;
      didgitsString = "";
    }
    else if (key == '*') {
      minutes_in1 = didgitsInt;
      didgitsString = "";
    }
    else if (key == 'a') {
      hours_in2 = didgitsInt;
      didgitsString = "";
    }
    else if (key == 'b') {
      minutes_in2 = didgitsInt;
      didgitsString = "";
    }
    else if (key == 'c') {
      hours_in3 = didgitsInt;
      didgitsString = "";
    }
    else if (key == 'd') {
      minutes_in3 = didgitsInt;
      didgitsString = "";
    }
  }
}
