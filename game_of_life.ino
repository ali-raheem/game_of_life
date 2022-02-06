// Copyright 2022 Ali Raheem <github@shoryuken.me>
// https://github.com/ali-raheem/game_of_life

#include <MD_MAX72xx.h>
#define CLK_PIN   13  // or SCK
#define DATA_PIN  11  // or MOSI
#define CS_PIN    10  // or SS
#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define MAX_DEVICES  16
MD_MAX72XX mx = MD_MAX72XX(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);

uint32_t frame[32] = {
        0x3FFFFFFC,
        0x40000002,
        0x80000001,
        0x80000001,
        0x80000001,
        0x80000001,
        0x80000001,
        0xFFFFFFFF,
        0x80000001,
        0x80000001,
        0x80000001,
        0x80000001,
        0x80000001,
        0x80000001,
        0x80000001,
        0x80000001,
        0x80000001,
        0x80000001,
        0x80000001,
        0x80000001,
        0x80000001,
        0x80000001,
        0x80000001,
        0x80000001,
        0x80000001,
        0x80000001,
        0x80000001,
        0x80000001,
        0x80000001,
        0x80000001,
        0x40000002,
        0x3FFFFFFC
};

#define USE_STALE_LIMIT
#define USE_GENERATION_LIMIT
#define USE_SERIAL

#ifdef USE_GENERATION_LIMIT
const uint16_t GENERATION_LIMIT = 2000;
#endif

#ifdef USE_STALE_LIMIT
const uint8_t STALE_LIMIT = 10;
uint16_t previousPopulation;
uint8_t staleCount;
#endif

bool isAlive_wrapped(int, int);
bool isAlive_closed(int, int);
bool (*isAlive)(int, int) = isAlive_wrapped;

const uint8_t ROWS = 32;
uint32_t state[ROWS + 3];
const uint8_t firstRowBuffer = ROWS + 2;
const uint8_t COLS = 8 * sizeof(state[0]);
const uint32_t FRAME_TIME = 100;
const uint32_t SHOW_TIME_DELAY = 1000;
uint16_t generation;
uint8_t activeLineBuffer;

void showTime() {
  char count[5];
  static uint8_t mins = 30;
  static uint8_t hours = 23;
  if(++mins > 59) {
    mins = 0;
    if(++hours > 23)
      hours = 0;
  }
  snprintf(count, 5, "%02d%02d", hours, mins);
  mx.control(MD_MAX72XX::UPDATE, MD_MAX72XX::OFF);
  mx.clear();
  mx.setChar(5, count[3]);
  mx.setChar(12, count[2]);
  mx.setChar(15, ':');
  mx.setChar(21, count[1]);
  mx.setChar(29, count[0]);
  mx.control(MD_MAX72XX::UPDATE, MD_MAX72XX::ON);
  delay(SHOW_TIME_DELAY);
  mx.clear();
}

void clearState() {
  uint8_t i;
  for(i = 0; i < ROWS + 3; i++)
    state[i] = 0;
}

void initialize() {
  clearState();
  render((uint8_t *)frame);
  delay(10000);
  //showTime();
  generation = 0;
  
#ifdef USE_STALE_LIMIT
  previousPopulation = staleCount = 0;
#endif

  //randomize will intialise them... randomly.
  randomize();
  // Gliders
//  state[0] = 0x2;
//  state[1] = 0x1;
//  state[2] = 0x7;
//  state[5] = 0x20;
//  state[6] = 0x10;
//  state[7] = 0x70;
}

// Closed topology
bool isAlive_closed (int i, int j) {
  if (i < 0 || i > (ROWS-1) || j < 0 || j > (COLS-1))
    return false; // Out of bounds cells count as dead.
  return ((state[i]) >> j) & 1;
}

// Wrapped topology
bool isAlive_wrapped (int i, int j) {
  if (i < 0)
    i = ROWS - 1;
  if (i > (ROWS - 1))
    i = 0;
  if (j < 0)
    j = COLS - 1;
  if (j > (COLS - 1))
    j = 0;
  return (state[i] >> j) & 1;
}

void nextState (uint16_t i, uint16_t j, uint32_t s, uint8_t sum) {
  const uint32_t one = 1;
  switch (sum) {
    case 3:
      state[activeLineBuffer] |= (one << j);
      break;
    case 4:
      state[activeLineBuffer] |= (s << j);
      break;
    default:
      state[activeLineBuffer] &= ~(one << j);
  }
}

void randomize() {
   uint8_t i;
   for(i = 0; i < ROWS; i++)
    state[i] = random();
}

void sendBlock(uint8_t *data, uint8_t r, uint8_t c) {
  uint8_t block[8];
  uint8_t *blocksStart = data  + (32 * r) + c;
  uint8_t i;
  Serial.print("Block:");
  Serial.print(8 * ((r * 4) + c + 1), DEC);
  Serial.println();
  for (i = 0; i < 8; i++) {
      block[i] = blocksStart[4 * i];
      Serial.print(block[i], HEX);
      Serial.println();
  }
  mx.setBuffer(8 * ((r * 4) + c + 1)-1, 8, block);
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

uint16_t updateBoard() {
  uint16_t i, j, population = 0;
  activeLineBuffer = firstRowBuffer;
  for (i = 0; i < ROWS; i++) {
    state[activeLineBuffer] = 0;
    bool oldState = isAlive(i, 0);
    uint8_t sum_l = isAlive(i - 1, -1) + isAlive(i, -1) + isAlive(i + 1, -1);
    uint8_t sum_m = isAlive(i - 1, 0) + oldState + isAlive(i + 1, 0);
    for (j = 0; j < COLS; j++) {
      bool oldStateR = isAlive(i, j + 1);
      mx.setPoint(i % 8, j + i / 8 * COLS, oldState);
      #ifdef USE_SERIAL
      if (oldState)
        Serial.print(" # ");
      else
        Serial.print(" - ");
      #endif
      uint8_t sum_r = isAlive(i - 1, j + 1) + oldStateR + isAlive(i + 1, j + 1);
      uint8_t liveCells = sum_l + sum_m + sum_r;
      nextState(i, j, oldState, liveCells);
      population += oldState;
      oldState = oldStateR;
      sum_l = sum_m;
      sum_m = sum_r;
    }
    activeLineBuffer = ROWS + (i % 2);
    if (i > 1)  state[i - 1] = state[activeLineBuffer];
    #ifdef USE_SERIAL
      Serial.println();
    #endif 
  }
  state[0] = state[firstRowBuffer];
  state[ROWS - 1] = state[ROWS + (i % 2)];
  
  return population;
}

void setup() {
  mx.begin();
  mx.clear();
  mx.control(MD_MAX72XX::INTENSITY, 7);
 #ifdef USE_SERIAL
  Serial.begin(115200);
  Serial.println("Game of Life for Arduino");
  Serial.println("Copyright 2022 Ali Raheem <github@shoryuken.me>");
  Serial.println("https://github.com/ali-raheem/game_of_life");
  uint16_t columnCount = mx.getColumnCount();
  Serial.print(columnCount, DEC);
 #endif
  //mx.begin();
 // mx.control(MD_MAX72XX::INTENSITY, 0);
  //mx.clear();
  int seed = analogRead(0) ^ analogRead(1);
  randomSeed(seed);
  initialize();
}

void loop() {
  mx.control(MD_MAX72XX::UPDATE, MD_MAX72XX::OFF);
  mx.clear();
  uint32_t updateTime = millis();
  uint16_t population = updateBoard();
  updateTime = millis() - updateTime;
  mx.control(MD_MAX72XX::UPDATE, MD_MAX72XX::ON);
  //render();
#ifdef USE_SERIAL
  Serial.print("Generation:\t");
  Serial.print(generation, DEC);
  Serial.print("\t Population:\t");
  Serial.print(population, DEC);
  Serial.print("\t Took:\t");
  Serial.print(updateTime, DEC);
  Serial.println("ms.");
#endif
  if (population < 3) {
    initialize();
  } else {
  #ifdef USE_STALE_LIMIT
  if (++staleCount > STALE_LIMIT) {
    initialize();
  } else {
    if (previousPopulation != population) {
      staleCount = 0;
      previousPopulation = population;
    }
  #endif
  #ifdef USE_GENERATION_LIMIT
  if (generation > GENERATION_LIMIT) {
    initialize();
  } else {
  #endif
    generation++;
    delay(FRAME_TIME);
      #ifdef USE_GENERATION_LIMIT
      }
      #endif
    #ifdef USE_STALE_LIMIT
    }
    #endif
  }
}
