// Copyright 2022 Ali Raheem <github@shoryuken.me>
// https://github.com/ali-raheem/game_of_life

#include <MD_MAX72xx.h>
#define CLK_PIN   13  // or SCK
#define DATA_PIN  11  // or MOSI
#define CS_PIN    10  // or SS
#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define MAX_DEVICES  4
MD_MAX72XX mx = MD_MAX72XX(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);

//#define USE_STALE_LIMIT
#define USE_GENERATION_LIMIT

#ifdef USE_GENERATION_LIMIT
const uint16_t GENERATION_LIMIT = 2000;
#endif

#ifdef USE_STALE_LIMIT
const uint8_t STALE_LIMIT = 10;
uint16_t last_pop;
uint8_t stale;
#endif

bool get_state_wrapped(int, int);
bool get_state_closed(int, int);
bool (*get_state)(int, int) = get_state_wrapped;

const uint8_t ROWS = 8;
uint32_t state[ROWS + 3];
const uint8_t COLS = 8 * sizeof(state[0]);
uint32_t DELAY = 200;
uint16_t generation;
uint8_t activeLineBuffer = ROWS;

void showTime() {
  char count[5];
  static volatile unsigned int counter = 1;
  snprintf(count, 5, "%04d", counter++);
  mx.control(MD_MAX72XX::UPDATE, MD_MAX72XX::OFF);
  mx.clear();
  mx.setChar(5, count[3]);
  mx.setChar(13, count[2]);
  mx.setChar(21, count[1]);
  mx.setChar(29, count[0]);
 // mx.setPoint(5, 16, 1);
  //mx.setPoint(2, 16, 1);
  mx.control(MD_MAX72XX::UPDATE, MD_MAX72XX::ON);
  delay(2000);
  mx.clear();
}

void initialize() {
  showTime();
  generation = 0;
  
#ifdef USE_STALE_LIMIT
  last_pop = stale = 0;
#endif

  // randomize will intialise them... randomly.
  //randomize();
  // Gliders
  state[0] = 0x2;
  state[1] = 0x1;
  state[2] = 0x7;
  state[5] = 0x20;
  state[6] = 0x10;
  state[7] = 0x70;
}

// Closed topology
bool get_state_closed (int i, int j) {
  if (i < 0 || i > (ROWS-1) || j < 0 || j > (COLS-1))
    return false; // Out of bounds cells count as dead.
  return ((state[i]) >> j) & 1;
}

// Wrapped topology
bool get_state_wrapped (int i, int j) {
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

int sum (int i, int j) {
  int s = get_state(i - 1, j - 1) + get_state(i - 1, j) + get_state(i - 1, j + 1) +
          get_state(i    , j - 1) + get_state(i    , j) + get_state(i    , j + 1) +
          get_state(i + 1, j - 1) + get_state(i + 1, j) + get_state(i + 1, j + 1);
  return s;
}

void update_state (int i, int j) {
  switch (sum(i, j)) {
    case 3:
      state[activeLineBuffer] |= (1UL << j);
      break;
    case 4:
      state[activeLineBuffer] |= (((uint32_t) get_state(i, j)) << j);
      break;
    default:
      state[activeLineBuffer] &= ~(1UL << j);
  }
}

void randomize() {
   int i;
   for(i = 0; i < ROWS; i++)
    state[i] = random();
}

void sendBlock(uint8_t r, uint8_t c) {
  uint8_t block[8];
  uint8_t *block_state = (uint8_t *) state;
  block_state += c + 32 * r;
  uint8_t i;
  for (i = 0; i < 8; i++)
      block[i] = block_state[i * 4];
  mx.setBuffer((c + 1) * ROWS - 1, ROWS, block);
}

void render(){
  uint8_t i, j;
  mx.control(MD_MAX72XX::UPDATE, MD_MAX72XX::OFF);
  mx.clear();
  for(i = 0; i < ROWS/8; i++)
    for(j = 0; j < COLS/8; j++)
      sendBlock(i, j);
  mx.transform(MD_MAX72XX::TRC);
  mx.control(MD_MAX72XX::UPDATE, MD_MAX72XX::ON);
}

void setup() {
  mx.begin();
  mx.control(MD_MAX72XX::INTENSITY, 0);
  int seed = analogRead(0) ^ analogRead(1);
  randomSeed(seed);
  initialize();
}

void loop() {
  int i, j, pop = 0;
  activeLineBuffer = ROWS + 2;
  for (i = 0; i < ROWS; i++) {
    state[activeLineBuffer] = 0;
    for (j = 0; j < COLS; j++) {
      bool s = get_state(i, j);
      pop += s;
      update_state(i, j);
    }
    activeLineBuffer = ROWS + (i % 2);
    if (i > 1)  state[i - 1] = state[activeLineBuffer];
  }
  state[0] = state[ROWS + 2];
  state[ROWS - 1] = state[ROWS + (i % 2)];
  render();
  if (pop < 3) {
    initialize();
  } else {
  #ifdef USE_STALE_LIMIT
  if (++stale > STALE_LIMIT) {
    initialize();
  } else {
    if (last_pop != pop) {
      stale = 0;
      last_pop = pop;
    }
  #endif
  #ifdef USE_GENERATION_LIMIT
  if (generation > GENERATION_LIMIT) {
    initialize();
  } else {
  #endif
    generation++;
    delay(DELAY);
      #ifdef USE_GENERATION_LIMIT
      }
      #endif
    #ifdef USE_STALE_LIMIT
    }
    #endif
  }
}
