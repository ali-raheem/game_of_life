// Copyright 2022 Ali Raheem <github@shoryuken.me>
// https://github.com/ali-raheem/game_of_life
// https://github.com/ali-raheem/conway

#include "conway.h"

//#define USE_STALE_LIMIT
//#define USE_GENERATION_LIMIT
//#define USE_SERIAL
//#define USE_LED

#ifdef USE_GENERATION_LIMIT
const uint16_t GENERATION_LIMIT = 2000;
#endif

#ifdef USE_STALE_LIMIT
const uint8_t STALE_LIMIT = 10;
uint16_t previousPopulation;
uint8_t staleCount;
#endif

const uint8_t ROWS = 32;
typedef uint32_t row; // uint32_t = 32 columns, uint64_t = 64 columns etc...
const uint8_t BUFFER_LENGTH = ROWS + 3;
row state[ROWS + 3];
const uint8_t COLS = 8 * sizeof(row);
const uint32_t FRAME_TIME = 0;
const uint32_t SHOW_TIME_DELAY = 5000;
const uint8_t LED_BRIGHTNESS = 0; // 0-7
Conway<row> gol(state, BUFFER_LENGTH);

#ifdef USE_LED
#include <MD_MAX72xx.h>
#define CLK_PIN   13  // or SCK
#define DATA_PIN  11  // or MOSI
#define CS_PIN    10  // or SS
#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define MAX_DEVICES  16
MD_MAX72XX mx = MD_MAX72XX(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);
#endif

#ifdef USE_SERIAL
const char LIVE[] = " # ";
const char DEAD[] = " - ";
#endif

#include <avr/wdt.h>

//row frame[32] = {
//        0x3FFFFFFC,
//        0x40000002,
//        0x80000001,
//        0x80000001,
//        0x80000001,
//        0x80000001,
//        0x80000001,
//        0xFFFFFFFF,
//        0x80000001,
//        0x80000001,
//        0x80000001,
//        0x80000001,
//        0x80000001,
//        0x80000001,
//        0x80000001,
//        0x80000001,
//        0x80000001,
//        0x80000001,
//        0x80000001,
//        0x80000001,
//        0x80000001,
//        0x80000001,
//        0x80000001,
//        0x80000001,
//        0x80000001,
//        0x80000001,
//        0x80000001,
//        0x80000001,
//        0x80000001,
//        0x80000001,
//        0x40000002,
//        0x3FFFFFFC
//};
//
//row heart[32] = {
//        0x00000000,
//        0x00000000,
//        0x00000000,
//        0x07F83F80,
//        0x1806C070,
//        0x30010018,
//        0x4002000C,
//        0xC0060006,
//        0x80040002,
//        0x80000002,
//        0x80000001,
//        0x80000001,
//        0x80000001,
//        0x80000001,
//        0x80000001,
//        0x80000001,
//        0xC0000001,
//        0x40000002,
//        0x60000006,
//        0x2000000C,
//        0x10000018,
//        0x18000030,
//        0x0C000060,
//        0x060000C0,
//        0x03000180,
//        0x01800300,
//        0x00C00600,
//        0x00600C00,
//        0x00301800,
//        0x00186000,
//        0x000D8000,
//        0x00020000
//};
//
//uint32_t knob[32] = {
//        0x00000000,
//        0x00000000,
//        0x00000000,
//        0x00000000,
//        0x00000000,
//        0x3E000000,
//        0x62000000,
//        0x43E00000,
//        0x623FE000,
//        0x66003FC0,
//        0x64000000,
//        0x44000000,
//        0x44000000,
//        0x64000000,
//        0x34000000,
//        0x0F000000,
//        0x01E00000,
//        0x003C0000,
//        0x0007F810,
//        0x00001818,
//        0x0000300C,
//        0x00002004,
//        0x00006006,
//        0x00004002,
//        0x00004002,
//        0x00004102,
//        0x00004182,
//        0x00006086,
//        0x000038CC,
//        0x00000F78,
//        0x00000000,
//        0x00000000
//};

void showTime() {
  char count[6];
  static uint8_t mins = 30;
  static uint8_t hours = 23;
  if(++mins > 59) {
    mins = 0;
    if(++hours > 23)
      hours = 0;
  }
  snprintf(count, 6, "%02d:%02d", hours, mins);
#ifdef USE_LED
  printText(1, count);
#endif
#ifdef USE_SERIAL
  Serial.println(count);
#endif
}
void showDate() {
  char count[6];
  static uint8_t day = 06;
  static uint8_t month = 02;
  snprintf(count, 6, "%02d.%02d", day, month);
#ifdef USE_LED
  printText(1, count);
#endif
#ifdef USE_SERIAL
  Serial.println(count);
#endif
}

void initialize() {
  wdt_disable();
//  render((uint8_t *) frame);
//  showTime();
//  showDate();
//  delay(SHOW_TIME_DELAY);
//  render((uint8_t *) heart);
//  delay(SHOW_TIME_DELAY);
//    render((uint8_t *) knob);
//  delay(SHOW_TIME_DELAY);

  //randomize will intialise them... randomly.
  randomize();
  // Gliders
//  state[0] = 0x2;
//  state[1] = 0x1;
//  state[2] = 0x7;
//  state[5] = 0x20;
//  state[6] = 0x10;
//  state[7] = 0x70;
  wdt_enable(WDTO_1S);
}

#ifdef USE_LED
void printText(const uint8_t r, const char data[6]) {
  mx.control(MD_MAX72XX::UPDATE, MD_MAX72XX::OFF);
  mx.setChar(5+r*32, data[4]);
  mx.setChar(12+r*32, data[3]);
  mx.setChar(15+r*32, data[2]);
  mx.setChar(21+r*32, data[1]);
  mx.setChar(29+r*32, data[0]);
  mx.control(MD_MAX72XX::UPDATE, MD_MAX72XX::ON);
}
void sendBlock(uint8_t *data, uint8_t r, uint8_t c) {
  uint8_t block[8];
  uint8_t *blocksStart = data  + (COLS * r) + c;
  uint8_t i;
  for (i = 0; i < 8; i++)
      block[i] = blocksStart[(COLS/8) * i];
  mx.setBuffer(8 * ((r * (COLS/8)) + c + 1) - 1, 8, block);
}

void render(uint8_t *data){
  uint8_t i, j;
  mx.control(MD_MAX72XX::UPDATE, MD_MAX72XX::OFF);
  mx.clear();
  for(i = 0; i < ROWS/8; i++)
    for(j = 0; j < COLS/8; j++)
      sendBlock(data, i, j);
  mx.transform(MD_MAX72XX::TRC);
  mx.control(MD_MAX72XX::UPDATE, MD_MAX72XX::ON);
}
#endif

void randomize() {
   uint8_t i;
   for(i = 0; i < ROWS; i++)
    gol.state[i] = random();
}

void setup() {
#ifdef USE_LED
  mx.begin();
  mx.clear();
  mx.control(MD_MAX72XX::INTENSITY, LED_BRIGHTNESS);
#endif
#ifdef USE_SERIAL
  Serial.begin(115200);
  Serial.println("Game of Life for Arduino");
  Serial.println("Copyright 2022 Ali Raheem <github@shoryuken.me>");
  Serial.println("https://github.com/ali-raheem/game_of_life");
#endif
  int seed = analogRead(0) ^ analogRead(1);
  randomSeed(seed);
  initialize();
}

void loop() {
  wdt_reset();
#ifdef USE_SERIAL
  uint32_t updateTime = millis();
#endif
  gol.next();
#ifdef USE_SERIAL
  updateTime = millis() - updateTime;
#endif
  uint8_t i, j;
#ifdef USE_SERIAL
  for(i = 0; i < ROWS; i++) {
    for(j = 0; j < COLS; j++) {
      bool s = (gol.*(gol.getCellState))(i, j);
      Serial.print((s? LIVE : DEAD));
    }
    Serial.println();
  }
  Serial.print("Generation:\t");
  Serial.print(gol.generation, DEC);
  Serial.print("\t Population:\t");
  Serial.print(gol.population, DEC);
  Serial.print("\t Stale:\t");
  Serial.print(gol.staleness, DEC);
  Serial.print("\t Took:\t");
  Serial.print(updateTime, DEC);
  Serial.println("ms.");
#endif
#ifdef USE_LED
  render((uint8_t *) gol.state);
#endif
  delay(FRAME_TIME);
  if (gol.population < 3) {
    reset();
  }
#ifdef USE_STALE_LIMIT
  if (gol.staleness > STALE_LIMIT) {
    reset();
  }
#endif
#ifdef USE_GENERATION_LIMIT
  if (gol.generation > GENERATION_LIMIT) {
    reset();
  }
#endif
 }

void reset() {
  asm volatile (" jmp 0");
}
