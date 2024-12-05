/* Printer Enclosure Temperature Controller ***************************************************************************/
//
// Copyright 2024 Marsette A. Vona (martyvona@gmail.com)
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
// documentation files (the “Software”), to deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
// Software.
// 
// THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
// WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
// COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
// OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 

//there are reports that Prusa recommends a max enclosure temp of 40C
//and that the PETG printed parts can deform when attempting 60C enclosure
//https://forum.prusa3d.com/forum/postid/630914
//
//Lars' "Automated Heating System for Original Enclosure" allows up to 45C
//https://www.printables.com/model/561491-automated-heating-system-for-original-enclosure
const int MAX_SET_TEMP = 45;

//Prusa Blaze Cut fire suppression system (T033E) has a max operating temp of 90C, activates at 105C +/-3C
const int SAFETY_TEMP = 85;

int min_temp_c = -1, max_temp_c = -1;

enum { COOL, HEAT, IDLE, CIRC };
int mode = IDLE;
const int NUM_MODES = 4;

enum { MANUAL, STARTING, RECOVERING, COOLING, HEATING };
int state = MANUAL;

//MANUAL -> STARTING -> HEATING/COOLING -> RECOVERING
//  ^         | ^            |  ^             | |
//  |         | |            |  |             | |
//  |         | |            |  --------------- |
//  ---------------------------------------------

/* Timer **************************************************************************************************************/

unsigned long timer_end_ms = 0;
int hours_remaining = 0, minutes_remaining = 0;

void startTimer() {
  bool was_running = timerRunning();
  timer_end_ms = millis() + 1000L * 60 * (hours_remaining * 60 + minutes_remaining);
  if (!was_running) state = STARTING;
}

bool timerRunning() { return timer_end_ms > 0; }

/* LCD ****************************************************************************************************************/

#include "DFRobot_RGBLCD1602.h"

const int LCD_RGB_ADDR = 0x2D;
const int LCD_COLS = 16, LCD_ROWS = 2;
const int LCD_BACKLIGHT = 32;
DFRobot_RGBLCD1602 lcd(LCD_RGB_ADDR, LCD_COLS, LCD_ROWS);

char * itoa2(int i, char * a) {
  if (i >= 0 && i <= 99) {
    a[0] = '0' + (i / 10);
    a[1] = '0' + (i % 10);
  } else a[0] = a[1] = '?';
  return a;
}

void writeStr(const char *s) { for (int i = 0; s[i]; i++) lcd.write(s[i]); }
void writeStrN(const char *s, const int n) { for (int i = 0; s[i] && i < n; i++) lcd.write(s[i]); }

/* Buttons ************************************************************************************************************/

enum { BTN_RIGHT = 0, BTN_UP, BTN_DOWN, BTN_LEFT, BTN_SELECT, BTN_NONE };
const int BTN_THRESHOLD[] = { 103, 306, 515, 724, 924 };
unsigned long btn_timeout[] = { 0, 0, 0, 0, 0 };
const int BTN_ANALOG_PIN = 0;
const int BTN_DEBOUNCE_MS = 50, BTN_REPEAT_MS = 250;

int getButton() {
  unsigned long now = millis(), button_value = analogRead(BTN_ANALOG_PIN);
  int btn = BTN_NONE;
  for (int i = 0; i < 5; i++) {
    if ((i == 0 || button_value > BTN_THRESHOLD[i - 1]) && button_value <= BTN_THRESHOLD[i]) {
      if (btn_timeout[i] == 0) btn_timeout[i] = now + BTN_DEBOUNCE_MS;
      else if (now > btn_timeout[i]) { btn = i; btn_timeout[i] = now + BTN_REPEAT_MS; }
    } else btn_timeout[i] = 0;
  }
  return btn;
}

bool buttonDown() {
  for (int i = 0; i < 5; i++) if (btn_timeout[i] > 0) return true;
  return false;
}

int incr(int &i, const int n) { if (i < 0) i = 0; else if (i < n - 1) i++; else i = 0; return i; }
int decr(int &i, const int n) { if (i < 0) i = n - 1; else if (i > 0) i--; else i = n - 1; return i; }

enum { NO_INPUT, MODE_INPUT, PROFILE_INPUT, MIN_TEMP_INPUT, MAX_TEMP_INPUT, HOURS_INPUT, MINUTES_INPUT };
int current_input = NO_INPUT;
const int LAST_INPUT = MINUTES_INPUT;

unsigned long input_end_ms = 0;
const int INPUT_TIMEOUT_MS = 30 * 1000;

/* Sensors ************************************************************************************************************/

unsigned long next_sensor_update = 0;
const unsigned long SENSOR_UPDATE_MS = 1000L;

#include <OneWire.h>
#include <DallasTemperature.h>

const int DS18B20_RESOLUTION = 9;
const int DS18B20_IN_PIN = 2;
OneWire one_wire(DS18B20_IN_PIN);
DallasTemperature dallas_temperature(&one_wire);
boolean dt_initialized = false;

const int MAX_NUM_DS18B20 = 8;
float ds18b20_weights[] = { 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f };

float current_temp_c = -1, current_min_temp_c = -1, current_max_temp_c = -1;

#include <DHT20.h>

const unsigned long DHT20_STARTUP_MS = 2000L, DHT20_UPDATE_MS = 1000L;

DHT20 dht20;

bool dht20_initialized = false;

float dht20_weight = 1.0f;

int current_humidity = -1;

/* Outputs ************************************************************************************************************/

const int HEATER_OUT_PIN = 12;
const int HEAT_FAN_OUT_PIN = 11;
const int COOL_FAN_OUT_PIN = 3;

const int COOL_FAN_PWM = 255;
const int HEAT_FAN_PWM = 255;
const int CIRC_FAN_PWM = 255;

const int HEAT_ON = 1, HEAT_OFF = 0;

/* Profiles ***********************************************************************************************************/

struct Profile {
  const char * name;
  int mode; //HEAT or COOL
  int min_temp_c, max_temp_c;
};
Profile profiles[] = {
  //  MTL   MODE  MIN  MAX
  { " PLA", COOL,  20,  25 },
  { "PETG", COOL,  25,  30 },
  { " ABS", HEAT,  54,  55 }, //temps will be limited elsewhere to MAX_SET_TEMP
  { "  PC", HEAT,  59,  60 },
  { "ACTL", HEAT,  64,  65 }
};
const int DEFAULT_PROFILE = 2;
const int NUM_PROFILES = 5;
int profile = DEFAULT_PROFILE;
int profile_mode = COOL; //HEAT or COOL

void setProfile(int p) {
  max_temp_c = profiles[p].max_temp_c;
  min_temp_c = min(max_temp_c - 1, profiles[p].min_temp_c);
  if (max_temp_c > MAX_SET_TEMP) {
    min_temp_c = MAX_SET_TEMP - (max_temp_c - min_temp_c);
    max_temp_c = MAX_SET_TEMP;
  }
  profile_mode = profiles[p].mode == HEAT ? HEAT : COOL;
  mode = IDLE;
  state = timerRunning() ? STARTING : MANUAL;
}

/* Update Functions ***************************************************************************************************/

void updateTimer() {

  if (timer_end_ms == 0) return;

  unsigned long now_ms = millis();

  if (timer_end_ms > 0 && now_ms > timer_end_ms) {
    hours_remaining = minutes_remaining = 0;
    timer_end_ms = 0;
    mode = IDLE;
    state = MANUAL;
    return;
  }

  unsigned long ms_remaining = timer_end_ms - now_ms;

  hours_remaining = (int)(ms_remaining / (1000L * 60 * 60));
  minutes_remaining = (int)ceil((ms_remaining - hours_remaining * (1000.0f * 60 * 60)) / (60 * 1000.0f));

  if (minutes_remaining == 60) { hours_remaining++; minutes_remaining = 0; }
}

void updateSensors() {

  unsigned long now = millis();
  if (next_sensor_update > 0 && now < next_sensor_update) return;

  current_min_temp_c = current_max_temp_c = -1;

  float sum_weights = 0.0f, dht20_temp = -1, avg = 0.0f;

  if (!dht20.isConnected()) { dht20_initialized = false; current_humidity = -1; }
  else {
    if (!dht20_initialized && now > DHT20_STARTUP_MS) { dht20.requestData(); dht20_initialized = true; }
    else if (dht20_initialized) {
      dht20.readData();
      dht20.convert();
      current_humidity = dht20.getHumidity();
      dht20_temp = dht20.getTemperature();
      sum_weights += dht20_weight;
      dht20.requestData();
      current_min_temp_c = current_max_temp_c = dht20_temp;
    }
  }

  dallas_temperature.begin();
  int ndt = min(MAX_NUM_DS18B20, dallas_temperature.getDS18Count());

  if (ndt == 0) dt_initialized = false;
  else if (!dt_initialized) {
    dallas_temperature.setResolution(DS18B20_RESOLUTION);
    dallas_temperature.setWaitForConversion(false);
    dallas_temperature.requestTemperatures();
    dt_initialized = true;
  } else {
    for (int i = 0; i < ndt; i++) sum_weights += ds18b20_weights[i];
    for (int i = 0; i < ndt; i++) {
        float dt_temp = dallas_temperature.getTempCByIndex(i);
        avg += (ds18b20_weights[i] / sum_weights) * dt_temp;
        current_min_temp_c = current_min_temp_c >= 0 ? min(current_min_temp_c, dt_temp) : dt_temp;
        current_max_temp_c = max(current_max_temp_c, dt_temp);
    }
    dallas_temperature.requestTemperatures();
  }

  if (dht20_temp > 0) avg += (dht20_weight / sum_weights) * dht20_temp;

  current_temp_c = (ndt > 0 || dht20_temp > 0) ? avg : -1;

  next_sensor_update = millis() + max(dallas_temperature.millisToWaitForConversion(DS18B20_RESOLUTION) + 100,
                                      max(DHT20_UPDATE_MS, SENSOR_UPDATE_MS));
}

void updateOutputs() {

  bool has_temp = current_temp_c >= 0;
  bool below_temp = has_temp && current_temp_c <= min_temp_c;
  bool above_temp = has_temp && current_temp_c >= max_temp_c;
  bool in_temp = !below_temp && !above_temp;

  switch (state) {
  case MANUAL: break;
  case RECOVERING:
    if (in_temp) { mode = profile_mode == HEAT ? CIRC : IDLE; break; }
    //otherwise fall through
  case STARTING:
    if (profile_mode == HEAT) {
      if (!has_temp) mode = IDLE;
      else if (current_temp_c < max_temp_c) { mode = HEAT; state = HEATING; }
      else { mode = CIRC; state = RECOVERING; }
    } else {
      if (!has_temp) mode = IDLE;
      else if (current_temp_c > min_temp_c) { mode = COOL; state = COOLING; }
      else { mode = IDLE; state = RECOVERING; }
    }
    break;
  case COOLING:
    if (below_temp) { mode = IDLE; state = RECOVERING; }
    if (above_temp && profile_mode == COOL) mode = COOL;
    break;
  case HEATING:
    if (below_temp && profile_mode == HEAT) mode = HEAT;
    if (above_temp) { mode = IDLE; state = RECOVERING; }
    break;
  }

  if (mode == HEAT && !timerRunning() || !has_temp || current_max_temp_c > SAFETY_TEMP) mode = IDLE;

  switch (mode) {
  case IDLE:
    digitalWrite(HEATER_OUT_PIN, HEAT_OFF);
    analogWrite(HEAT_FAN_OUT_PIN, 0);
    analogWrite(COOL_FAN_OUT_PIN, 0);
    break;
  case CIRC:
    digitalWrite(HEATER_OUT_PIN, HEAT_OFF);
    analogWrite(HEAT_FAN_OUT_PIN, CIRC_FAN_PWM);
    analogWrite(COOL_FAN_OUT_PIN, 0);
    break;
  case COOL:
    digitalWrite(HEATER_OUT_PIN, HEAT_OFF);
    analogWrite(HEAT_FAN_OUT_PIN, 0);
    analogWrite(COOL_FAN_OUT_PIN, COOL_FAN_PWM);
    break;
  case HEAT:
    digitalWrite(HEATER_OUT_PIN, HEAT_ON);
    analogWrite(HEAT_FAN_OUT_PIN, HEAT_FAN_PWM);
    analogWrite(COOL_FAN_OUT_PIN, 0);
    break;
  }
}

void updateDisplay() {
  char nn[3]; nn[2] = 0;

  unsigned long sec = millis() / 1000;
  bool odd_sec = sec & 1;
  bool blink = !buttonDown() && odd_sec;

  lcd.setCursor(0, 0);

  bool show_minmax = (sec % 10) >= 8; //show min/max for 2 out of every 10s
  if (show_minmax) {
      writeStr(itoa2((int)current_min_temp_c, nn));
      lcd.write('-');
      writeStr(itoa2((int)current_max_temp_c, nn));
  } else {
      writeStr(itoa2((int)current_temp_c, nn));
      lcd.write('.');
      writeStr(itoa2(current_temp_c >= 0 ? (int)((current_temp_c - (int)current_temp_c) * 100) : -1, nn));
  }
  lcd.write('C');
    
  lcd.write(' ');

  if (current_input == MODE_INPUT && blink) writeStr("    ");
  else {
    switch (mode) {
    case IDLE: writeStr("IDLE"); break;
    case CIRC: writeStr("CIRC"); break;
    case COOL: writeStr("COOL"); break;
    case HEAT: writeStr("HEAT"); break;
    }
  }

  lcd.write(' ');

  writeStrN(current_input == PROFILE_INPUT && blink ? "    " : profiles[profile].name, 4);

  lcd.setCursor(0, 1);

  writeStr(current_input == MIN_TEMP_INPUT && blink ? "  " : itoa2(min_temp_c, nn));
  lcd.write('-');
  writeStr(current_input == MAX_TEMP_INPUT && blink ? "  " : itoa2(max_temp_c, nn));
  lcd.write('C');

  lcd.write(' ');

  writeStr(current_input == HOURS_INPUT && blink ? "  " : itoa2(hours_remaining, nn));
  lcd.write(odd_sec && timer_end_ms > 0 ? ' ' : ':');
  writeStr(current_input == MINUTES_INPUT && blink ? "  " : itoa2(minutes_remaining, nn));

  lcd.write(' ');
  if (current_humidity >= 0) { writeStr(itoa2(current_humidity, nn)); writeStr("%H"); }
  else writeStr("    ");
}

void updateUI() {
  bool button_down = true;
  switch (getButton()) {
  case BTN_UP:
    switch (current_input) {
    case NO_INPUT: break;
    case MODE_INPUT: incr(mode, NUM_MODES); state = MANUAL; break;
    case PROFILE_INPUT: setProfile(incr(profile, NUM_PROFILES)); break;
    case MIN_TEMP_INPUT: if (min_temp_c < max_temp_c - 1) incr(min_temp_c, MAX_ALLOWED_TEMP); break;
    case MAX_TEMP_INPUT: if (incr(max_temp_c, MAX_ALLOWED_TEMP) < min_temp_c) max_temp_c = min_temp_c + 1; break;
    case HOURS_INPUT: incr(hours_remaining, 100); startTimer(); break;
    case MINUTES_INPUT: incr(minutes_remaining, 60); startTimer(); break;
    }
    break;
  case BTN_DOWN:
    switch (current_input) {
    case NO_INPUT: break;
    case MODE_INPUT: decr(mode, NUM_MODES); state = MANUAL; break;
    case PROFILE_INPUT: setProfile(decr(profile, NUM_PROFILES)); break;
    case MIN_TEMP_INPUT: decr(min_temp_c, max_temp_c - 1); break;
    case MAX_TEMP_INPUT: if (max_temp_c > min_temp_c + 1) decr(max_temp_c, 100); break;
    case HOURS_INPUT: decr(hours_remaining, 100); startTimer(); break;
    case MINUTES_INPUT: decr(minutes_remaining, 60); startTimer(); break;
    }
    break;
  case BTN_LEFT: if (--current_input < 0) current_input = LAST_INPUT; break;
  case BTN_RIGHT: if (++current_input > LAST_INPUT) current_input = NO_INPUT; break;
  case BTN_SELECT: current_input = NO_INPUT; break;
  case BTN_NONE: button_down = false; break;
  }
  long now = millis();
  if (button_down && current_input != NO_INPUT) input_end_ms = now + INPUT_TIMEOUT_MS;
  else if (now > input_end_ms) current_input = NO_INPUT;
}

/* Arduino Runtime ****************************************************************************************************/

void setup() {

  pinMode(HEATER_OUT_PIN, OUTPUT);
  pinMode(HEAT_FAN_OUT_PIN, OUTPUT);
  pinMode(COOL_FAN_OUT_PIN, OUTPUT);

  setProfile(DEFAULT_PROFILE);

  lcd.init();
  lcd.setRGB(LCD_BACKLIGHT, LCD_BACKLIGHT, LCD_BACKLIGHT);

  Wire.begin();
  Wire.setClock(400000);
  dht20.begin();
}

void loop() {
  updateTimer();
  updateSensors();
  updateOutputs();
  updateDisplay();
  updateUI();
}
