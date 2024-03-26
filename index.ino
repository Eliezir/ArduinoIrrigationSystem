#include <IRremote.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(32,16,2);

const int humidity_sensor = A0;
const int valve = 3;
const int recv_pin = 2;
const int ledRed = 12;
const int ledBlue = 11;
const int ledGreen = 10;
int  menu = 0;
int menuState = 1;
int power = 0;
float minimum_humidity = 438.0;
float humidity;
bool settingHour = true;
int CurrentHours = 0;
int CurrentMinutes = 0;
int MinHours = 0;
int MinMinutes = 0;
int MaxHours = 0;
int MaxMinutes = 0;

unsigned long previousMillis = 0; 
const long interval = 60000; 

void updateCurrentTime() {
  if(menu == 1) return;
  unsigned long currentMillis = millis();
  if(currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    CurrentMinutes++;
    if (CurrentMinutes >= 60) {
      CurrentMinutes = 0;
      CurrentHours++;
      if (CurrentHours >= 24) {
        CurrentHours = 0;
      }
    }
    Serial.print("Current Time: ");
    Serial.print(CurrentHours);
    Serial.print(":");
    Serial.println(CurrentMinutes);
  }
}

void checkHumidityAndTime() {
  int current_time_in_minutes = CurrentHours * 60 + CurrentMinutes;
  int min_time_in_minutes = MinHours * 60 + MinMinutes;
  int max_time_in_minutes = MaxHours * 60 + MaxMinutes;

  if (current_time_in_minutes >= min_time_in_minutes && current_time_in_minutes <= max_time_in_minutes) {
    if (humidity < minimum_humidity) {
      digitalWrite(valve, HIGH);
    }
    else {
      Serial.println("Humidity is above minimum");
      digitalWrite(valve, LOW);
    }
  }
  else {
    Serial.println("Current time is not within the range");
    digitalWrite(valve, LOW);
  }
}

byte arrowUp[8] = {
  0b00100,
  0b01110,
  0b11111,
  0b00000,
  0b00000,
  0b00000,
  0b00000,
  0b00000
};

byte arrowDown[8] = {
  0b00000,
  0b00000,
  0b00000,
  0b00000,
  0b11111,
  0b01110,
  0b00100,
  0b00000
};
 
IRrecv irrecv(recv_pin);
decode_results results;

void adjustHumidity(bool increase) {
  if (!power) return;
  minimum_humidity += increase ? 10 : -10;
}

void displayHumidity(){
    lcd.setCursor(0,0);
	  lcd.print("Soil   = ");
    lcd.setCursor(10,0);
    lcd.print(humidity);
    lcd.setCursor(0,1);
    lcd.print("System = ");
    lcd.setCursor(10,1);
    lcd.print(minimum_humidity);
}


void displayMenu(){
  if(menuState == 1){
  lcd.setCursor(0,0);
  lcd.print("1.Set Time");
  lcd.setCursor(0,1);
  lcd.print("2.Set Humidity");
    blinkingArrow(0);
  }
  else if(menuState == 2){
  lcd.setCursor(0,0);
  lcd.print("3.Min Time");
  lcd.setCursor(0,1);
  lcd.print("4.Max Time");
    blinkingArrow(1);
  }
}


unsigned long previousMillisArrow = 0; 
const long intervalArrow = 100; 
bool arrowVisible = true;

void blinkingArrow(int direction) {
  unsigned long currentMillisArrow = millis();
  if(currentMillisArrow - previousMillisArrow >= intervalArrow) {
    previousMillisArrow = currentMillisArrow;
    lcd.setCursor(15, direction - 1); 
    if (arrowVisible) {
      if (direction == 1) {
        lcd.write(byte(0)); 
      } else {
        lcd.write(byte(1)); 
      }
    } else {
      lcd.print(" ");
    }
    arrowVisible = !arrowVisible;
  }
}

unsigned long previousMillisColon = 0; 
const long intervalColon = 100; 
bool colonVisible = true;

void blinkingColon() {
  unsigned long currentMillisColon = millis();
  if(currentMillisColon - previousMillisColon >= intervalColon) {
    previousMillisColon = currentMillisColon;
    lcd.setCursor(7,1);
    if (colonVisible) {
      lcd.write(':'); 
    } else {
      lcd.write(' '); 
    }
    colonVisible = !colonVisible;
  }
  lcd.setCursor(8,1);
}

void displayCurrentTime() {
  lcd.setCursor(0,0);
  lcd.print("Current Time");
  lcd.setCursor(5,1);
  if(CurrentHours == 0) {
    lcd.print("00");
  }
  else {
    lcd.print(CurrentHours);
  }
  blinkingColon();
  if(CurrentMinutes == 0) {
    lcd.print("00");
  }
  else {
    lcd.print(CurrentMinutes);
  }
}

void displayMinTime() {
  lcd.setCursor(0,0);
  lcd.print("Min Time: ");
  lcd.setCursor(5,1);
  if(MinHours == 0) {
    lcd.print("00");
  }
  else {
    lcd.print(MinHours);
  }
  blinkingColon();
  if(MinMinutes == 0) {
    lcd.print("00");
  }
  else {
    lcd.print(MinMinutes);
  }
}

void displayMaxTime() {
  lcd.setCursor(0,0);
  lcd.print("Max Time: ");
  lcd.setCursor(5,1);
  if(MaxHours == 0) {
    lcd.print("00");
  }
  else {
    lcd.print(MaxHours);
  }
  blinkingColon();
  if(MaxMinutes == 0) {
    lcd.print("00");
  }
  else {
    lcd.print(MaxMinutes);
  }
}


int tempTime = -1;
void setTime(int digit, int timeType, int isHours) {
  if(tempTime == -1) {
    tempTime = digit;
    return;
  }
  if(isHours) {
    if(timeType == 1) {
      CurrentHours = tempTime * 10 + digit;
    }
    else if(timeType == 3) {
      MinHours = tempTime * 10 + digit;
    }
    else if(timeType == 4) {
      MaxHours = tempTime * 10 + digit;
    }
    settingHour = false;
  }
  else {
    if(timeType == 1) {
      CurrentMinutes = tempTime * 10 + digit;
    }
    else if(timeType == 3) {
      MinMinutes = tempTime * 10 + digit;
    }
    else if(timeType == 4) {
      MaxMinutes = tempTime * 10 + digit;
    }
    settingHour = true;
  }
    tempTime = -1;
}


void setColor(int red, int green, int blue) {
  digitalWrite(ledRed, LOW);
  digitalWrite(ledGreen, LOW);
  digitalWrite(ledBlue, LOW);
  delay(150);
  digitalWrite(ledRed, red);
  digitalWrite(ledGreen, green);
  digitalWrite(ledBlue, blue);
};
void warning() {
  unsigned long startTime = millis();
  unsigned long elapsedTime = 0;
  
  while (elapsedTime < 10000) { 
    elapsedTime = millis() - startTime; 
    setColor(LOW, LOW, HIGH); 
    delay(300);
    setColor(LOW, HIGH, LOW);
    delay(300);
  }
}



void remoteResponse(){
setColor(LOW, HIGH, HIGH);
delay(500);
setColor(LOW, HIGH, LOW);
}


void menuNavigation(int digit){
  if(!power) return;
  lcd.clear();
  if(menu == 0){
    if(digit > 4) return;
    menu = digit;
  }
  else {
    if (settingHour) {
      setTime(digit, menu, 1);
    }
    else {
      setTime(digit, menu, 0);
    }
  }
}

void displayInfo(){
  lcd.setCursor(0,0);
  lcd.print("Time");
  lcd.setCursor(6,0);
    if(CurrentHours == 0) {
    lcd.print("00");
  }
  else {
    lcd.print(CurrentHours);
  }
  lcd.print(":");
  if(CurrentMinutes == 0) {
    lcd.print("00");
  }
  else {
    lcd.print(CurrentMinutes);
  }
  lcd.setCursor(0,1);
  lcd.print("Humidity");
  lcd.setCursor(10,1);
  lcd.print(humidity);

}


void setup()
{
  Serial.begin(96000);
  irrecv.enableIRIn();
  pinMode(humidity_sensor, INPUT);
  pinMode(valve, OUTPUT);
  pinMode(ledRed, OUTPUT);
  pinMode(ledBlue, OUTPUT);
  pinMode(ledGreen, OUTPUT);
  lcd.begin(16,2);
  lcd.init();
  lcd.createChar(0, arrowUp);
  lcd.createChar(1, arrowDown);
  setColor(HIGH, LOW, LOW);
}

void reset () {
  lcd.clear();
  minimum_humidity = 438.0;
  menu = 0;
  menuState = 1;
  settingHour = true;
  tempTime = -1;
}


void loop()
{
    updateCurrentTime();
    humidity = analogRead(humidity_sensor);
  if (irrecv.decode()){
   unsigned long  remote =(unsigned long)irrecv.decodedIRData.decodedRawData;
    switch (remote){
      case 4278238976: //power
        power = !power;
        if(power){
		    setColor(LOW, HIGH, LOW);
        break;
        }
        setColor(HIGH, LOW, LOW);
      	digitalWrite(valve, LOW);
        lcd.clear();
        lcd.noBacklight();
        break;
      case 4111122176: //up
        if(!power) break;
        if(menu == 0){
          lcd.clear();
          menuState = 1;
        }
        else if(menu == 2) {
          adjustHumidity(true);
        }
        break;
      case 4144545536: //down
        if(!power) break;
    	if(menu == 0){
          lcd.clear();
          menuState = 2;
        }
        else if(menu == 2) {
          adjustHumidity(false);
        }
        break;
      case 4077698816: //0
      menuNavigation(0);
          break;
      case 4010852096: //1
      menuNavigation(1);
        break;
      case 3994140416: //2
       menuNavigation(2);
        break;
      case 3977428736: //3
       menuNavigation(3);
        break;
      case 3944005376: //4
       menuNavigation(4);
        break;
      case 3927293696: //5
        menuNavigation(5);
          break;
      case 3910582016: //6
        menuNavigation(6);
          break;
      case 3877158656: //7
        menuNavigation(7);
          break;
      case 3860446976: //8
        menuNavigation(8);
          break;
      case 3843735296: //9
        menuNavigation(9);
          break; 
      case 4194680576: //pause-break
        reset();
        break;
      case 4244815616: // Func-Stop
      if(!power) break;
      lcd.clear();
      reset();
      menu = 5;
      break;
      default:
        Serial.println(remote);
        break;
    }
    irrecv.resume();
  }
  if(power){
    lcd.backlight();
    switch (menu){
      case 0:
        displayMenu();
        break;
      case 1:
        displayCurrentTime();
        break;
      case 2:
        displayHumidity();
        break;
      case 3:
        displayMinTime();
        break;
      case 4:
        displayMaxTime();
        break;
      case 5:
        displayInfo();
        break;
    }
      checkHumidityAndTime();
}
}