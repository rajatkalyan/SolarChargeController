#include<SPI.h>
#include<SD.h>
#include<Wire.h>
#include "RTClib.h"

RTC_DS3231 rtc;

// variables
int CS = 10;
float avg = 0.0;
int analog_current = A2;
int analog_bat = A1;
int analog_sol = A0;
float bat_volt = 0.0;
float solar_volt = 0.0;
float SP1 = 13.6;
float SP2 = 14.4;
float bat_min = 9.5;
float bat_max = 15.0;
float lvd = 9.0;
float voltage_bat = 0.0;
float voltage_sol = 0.0;
float current = 0.0;
float watt = 0.0;
int pwm_pin = 3;
int load_pin = 2;
float duty = 0.0;
float error = 0.0;
float Ep = 0.0;
float sum = 0.0;
float voltage = 0.0;
int bat_red_led = 5;
int bat_green_led = 6;
int bat_blue_led = 7;
int load_led1 = 9;
int load_led2 = 8;
int load_status = 0;


// List of Tasks
#define BATT_VOLT_MSR 1
#define PANL_VOLT_MSR 2
#define BATT_CHG_MGMT 3
#define LOAD_PWR_MGMT 4
#define LOAD_PWR_CALC 5
#define LED_IND       6
#define SD_LOG        7

char  state = BATT_VOLT_MSR;
///int  daysaving=1;
void setup() {
  // put your setup code here, to run once:

  Serial.begin(9600);
  SD.begin(CS);
  rtc.begin();
  store_var();
  pinMode(bat_red_led, OUTPUT);
  pinMode(bat_green_led, OUTPUT);
  pinMode(bat_blue_led, OUTPUT);
  pinMode(load_led1, OUTPUT);
  pinMode(load_led2, OUTPUT);
  pinMode(pwm_pin, OUTPUT);
  pinMode(load_pin, OUTPUT);

}

void loop()
{
  // put your main code here, to run repeatedly:
  switch (state)
  {
    case BATT_VOLT_MSR :

      analogWrite(pwm_pin, LOW);
      delay(100);
      voltage_bat = analogRead(analog_bat);
      voltage_bat = .00493 * voltage_bat;
      bat_volt = 5.567 * voltage_bat;
      //Serial.println(bat_volt);
      state = PANL_VOLT_MSR;
      break;

    case PANL_VOLT_MSR:
      voltage_sol = analogRead(analog_sol);
      voltage_sol = .00493 * voltage_sol;
      solar_volt = 5.567 * voltage_sol;
      //Serial.println(solar_volt);
      state = BATT_CHG_MGMT;
      break;

    case BATT_CHG_MGMT:
      if (solar_volt > bat_volt && bat_volt <= SP2)
      {
        if (bat_volt < SP1)
        {
          duty = 252.46;
          analogWrite(pwm_pin, duty);
        }
        else if (bat_volt > SP1 && bat_volt < SP2)
        {
          error = SP2 - bat_volt;
          Ep = error * 100;
          if (Ep < 0)
          {
            Ep = 0;
          }
          else if (Ep > 100)
          {
            Ep = 100;
          }
          else if (Ep > 0 && Ep < 100)
          {
            duty = (Ep * 255) / 100;
          }
          analogWrite(pwm_pin, duty);
        }

      }
      else
      {
        duty = 0;
        analogWrite(pwm_pin, duty);
      }
      state = LOAD_PWR_MGMT;
      break;
    case LOAD_PWR_MGMT:
      if (solar_volt < 5)
      {
        if (bat_volt > lvd)
        {
          load_status = 1;
          digitalWrite(load_pin, HIGH);
          Serial.println("load is connected");
        }
        else if (bat_volt < lvd)
        {
          load_status = 0;
          digitalWrite(load_pin, LOW);
        }
      }
      else
      {
        load_status = 0;
        digitalWrite(load_pin, LOW);
      }
      state = LOAD_PWR_CALC;
      break;

    case LOAD_PWR_CALC:

      for (int i = 0; i < 120; i++)
      {
        delay(5);
        voltage = abs(510.90 - analogRead(analog_current));
        sum = sum + voltage;
      }
      sum = sum / 120;
      current = (sum) * 27.03 / 1023;
      Serial.println(current);
      watt = bat_volt * current;
      state = LED_IND;
      break;

    case LED_IND:
      if (bat_volt > 10.0 && bat_volt < 14.4)
      {
        leds_off_all();
        digitalWrite(bat_green_led, LOW);
      }
      else if (bat_volt > SP2)
      {
        leds_off_all();
        digitalWrite(bat_blue_led, LOW);
      }
      else if (bat_volt < 9.5)
      {
        leds_off_all();
        digitalWrite(bat_red_led, LOW);
      }
      if (load_status == 1)
      {
        digitalWrite(load_led1, LOW);
      }
      else if (load_status == 0)
      {
        digitalWrite(load_led2, LOW);
      }
      state = SD_LOG;
      break;
    case SD_LOG:
      File datafile = SD.open("test.csv", FILE_WRITE);
      if (datafile)
      {
        delay(30000);
        delay(30000);
        DateTime now = rtc.now();
        Serial.println("Storing");
        datafile.print(now.hour(), DEC);
        datafile.print(":");
        datafile.print(now.minute(), DEC);
        datafile.print(":");
        datafile.print(",");
        datafile.print(now.year(), DEC);
        datafile.print(now.month(), DEC);
        datafile.print(now.day(), DEC);
        datafile.print(",");
        datafile.print(solar_volt);
        datafile.print(",");
        datafile.print(bat_volt);
        datafile.print(",");
        datafile.print(current);
        datafile.print(",");
        datafile.println(watt);
        Serial.print("Saving Data");
        datafile.close();

      }
      state = BATT_VOLT_MSR;
      break;
  }
}





void leds_off_all(void)
{ digitalWrite(bat_red_led, HIGH);
  digitalWrite(bat_green_led, HIGH);
  digitalWrite(bat_blue_led, HIGH);
  digitalWrite(load_led1, HIGH);
  digitalWrite(load_led2, HIGH);
}
void store_var() {
  File datafile = SD.open("test.csv", FILE_WRITE);
  if (datafile) {
    //delay(30000);
    //delay(30000);
    //DateTime now=rtc.now();
    Serial.println("Storing data");
    datafile.print('T');
    datafile.print(',');
    datafile.print('D');
    datafile.print(',');
    datafile.print('S');
    datafile.print(',');
    datafile.print('V');
    datafile.print(',');
    datafile.print('C');
    datafile.print(',');
    datafile.println('P');
    Serial.print("SAVING!!!!");

    //Serial.print(now.year(),DEC);
    datafile.close();

  }
}
