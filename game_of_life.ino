// Copyright 2022-2024 Ali Raheem <github@shoryuken.me>
// https://github.com/ali-raheem/game_of_life
// https://github.com/ali-raheem/conway

#pragma GCC optimize ("O3")

// Switch on optimziation for uint64_t matricies
//#define __CONWAY_OPTIMIZE_LARGE
uint16_t memory __attribute__ ((section (".noinit")))  ;
#include "conway.h"

#define USE_STALE_LIMIT
#define USE_GENERATION_LIMIT
//#define USE_SERIAL
#define USE_LED

#ifdef USE_GENERATION_LIMIT
const uint16_t GENERATION_LIMIT = 2000;
#endif

#ifdef USE_STALE_LIMIT
const uint8_t STALE_LIMIT = 200;
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

#ifdef __AVR
#include <avr/wdt.h>
#endif

void initialize() {
 randomize();
  // Gliders
 // state[0] = 0x00200;
  //state[1] = 0x00100;
  //state[2] = 0x00700;
//  state[5] = 0x20;
//  state[6] = 0x10;
//  state[7] = 0x70;
#ifdef __AVR
  wdt_enable(WDTO_2S);
#endif
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
   uint8_t i, j;
   row r;
   for(i = 0; i < ROWS; i++) {
    r = random();
    for(j = 0; j < sizeof(row)/sizeof(long); j++) {
      r <<= 8*sizeof(long);
      r |= (row) random();
    }
    gol.state[i] = r;
    r = 0;
   }
}

void LFSR(uint16_t *m) {
  // adapated from https://en.wikipedia.org/wiki/Linear-feedback_shift_register
  uint8_t lsb = *m & 1u;
  *m >>= 1;
  if(lsb == 1)
    *m ^= 0xB400u;
}
void setup() {
  Serial.begin(115200);

#ifdef __AVR
  wdt_disable();
#endif
#ifdef USE_LED
  mx.begin();
  mx.clear();
#endif
#ifdef USE_SERIAL
  while (!Serial);
  Serial.begin(115200);
  Serial.println("Game of Life for Arduino");
  Serial.println("Copyright 2022-2024 Ali Raheem <github@shoryuken.me>");
  Serial.println("https://github.com/ali-raheem/game_of_life");
#endif
  pinMode(A1, INPUT);
  digitalWrite(A1, LOW); // fake ground FIXME
  int lightLevel = analogRead(0);
  int seed = lightLevel ^ analogRead(1) ^ analogRead(2);
#ifdef USE_LED
  mx.control(MD_MAX72XX::INTENSITY, LED_BRIGHTNESS);
#endif
  if (memory == 0) memory = seed;
  LFSR(&memory);
  randomSeed(seed^memory);
  initialize();
#ifdef USE_SERIAL
  printSerial(0);
#endif
}
#ifdef USE_SERIAL
void printSerial(uint32_t updateTime) {
  uint8_t i, j;
  for(i = 0; i < ROWS; i++) {
    for(j = 0; j < COLS; j++) {
      bool s = gol.getCellState(i, j);
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
}
#endif

void loop() {
#ifdef __AVR
  wdt_reset();
#endif
#ifdef USE_SERIAL
  uint32_t updateTime = millis();
#endif
  gol.next();
#ifdef USE_SERIAL
  updateTime = millis() - updateTime;
  printSerial(updateTime);
#endif
#ifdef USE_LED
  render((uint8_t *) gol.state);
#endif
  if(FRAME_TIME > 0)
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

[[noreturn]] void reset() {
 #ifdef __AVR
  asm volatile (" jmp 0");
#endif
}
