#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <PID_v1.h>
#include <Adafruit_BMP085.h>


//LiquidCrystal_I2C lcd(0x38);  // Set the LCD I2C address
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE); 
//LiquidCrystal_I2C lcd(0x38, BACKLIGHT_PIN, POSITIVE);  // Set the LCD I2C address

const int LOWER_TEMP_BUTTON = 13;
const int RAISE_TEMP_BUTTON = 11;
const int FAN_PWM_PIN = 10;
const int WAKE_BUTTON = 12;

const int SCREEN_SLEEP = 100;

Adafruit_BMP085 pressureSensor;
double setTemperature;
char status;

int lowerT = 0;
int raiseT = 0;

int screenTimer = SCREEN_SLEEP;
double fanSpeed = 255;

double kp=2;   //proportional parameter
double ki=5;   //integral parameter
double kd=1;
double fanSpeedMin = 60;
double fanSpeedMax = 255;

double sensedTemperature = 0;
PID myPID(&sensedTemperature, &fanSpeed, &setTemperature,kp,ki,kd, REVERSE);

void wakeUpScreen(){
  screenTimer = SCREEN_SLEEP;
  lcd.backlight();
}

void screenToSleep(){
  lcd.clear();
  lcd.noBacklight();
}

void setup()
{
  // Switch on the backlight
  pinMode (FAN_PWM_PIN, OUTPUT);
  pinMode(LOWER_TEMP_BUTTON, INPUT);
  pinMode(RAISE_TEMP_BUTTON, INPUT);
  pinMode(WAKE_BUTTON, INPUT);
  TCCR1B = TCCR1B & B11111000 | B00000001; 
  
  lcd.begin(16,2);               // initialize the lcd 

  lcd.home ();                   // go home
  lcd.print("");  
  lcd.setCursor ( 0, 1 );        // go to the next line
  lcd.print ("");
  delay ( 1000 );

  if (pressureSensor.begin())
    Serial.println("BMP180 init success");
  else
  {
    // Oops, something went wrong, this is usually a connection problem,
    // see the comments at the top of this sketch for the proper connections.

    Serial.println("BMP180 init fail\n\n");
    while(1); // Pause forever.
  }
  lcd.clear();
  sensedTemperature = pressureSensor.readTemperature();
  setTemperature = sensedTemperature;
  myPID.SetMode(AUTOMATIC);
}
void displayData(){
     if(screenTimer <= 0){
    screenToSleep();
   }else{
     lcd.print (String(sensedTemperature));
     lcd.print ("C");
     lcd.setCursor ( 0, 1 );
     lcd.print ("T:");
     lcd.print(String(setTemperature));
     lcd.print("F:");
     lcd.print(String((fanSpeed / fanSpeedMax)*100));
     
   }
}
void processButtonInput (){
    if(digitalRead(WAKE_BUTTON) == HIGH){
      wakeUpScreen();
    }
    if(digitalRead(LOWER_TEMP_BUTTON) == HIGH){
      if(!lowerT || lowerT > 10){
        setTemperature -= 0.1;
        wakeUpScreen();
      }
    lowerT++;
   }else{
    lowerT = 0;
   }
   if(digitalRead(RAISE_TEMP_BUTTON) == HIGH){
      if(!raiseT || raiseT > 10){
        setTemperature += 0.1;
        wakeUpScreen();
      }
    raiseT++;
   }else{
    raiseT = 0;
   }
}
void loop()
{
   sensedTemperature = pressureSensor.readTemperature();
   myPID.Compute();
   lcd.home ();
   processButtonInput ();
   
   displayData();
   screenTimer--;
   analogWrite(FAN_PWM_PIN, fanSpeed);

   delay (50);
}
