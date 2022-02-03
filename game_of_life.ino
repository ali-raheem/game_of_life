// Copyright 2022 Ali Raheem <github@shoryuken.me>
// https://github.com/ali-raheem/game_of_life

#include <MD_MAX72xx.h>
#define CLK_PIN   13  // or SCK
#define DATA_PIN  11  // or MOSI
#define CS_PIN    10  // or SS
#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define MAX_DEVICES  4
MD_MAX72XX mx = MD_MAX72XX(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);

#define USE_STALE_LIMIT
#define USE_GENERATION_LIMIT

#ifdef USE_GENERATION_LIMIT
const uint16_t GENERATION_LIMIT = 2000; // Reset after this many generations
#endif

#ifdef USE_STALE_LIMIT
const uint8_t STALE_LIMIT = 10; // Reset if population not changed in STALE_LIMIT generations
uint16_t last_pop;
uint8_t stale;
#endif

bool get_state_wrapped(int, int);
bool get_state_closed(int, int);
bool (*get_state)(int, int) = get_state_wrapped;

const uint8_t ROWS = 8;
uint32_t state[2][ROWS];
const uint8_t COLS = 8 * sizeof(state[0][0]); // bits in type used for state array unsigned long
uint32_t DELAY = 200;
uint16_t generation;
bool active = false;

//void showTime() {
//  mx.control(MD_MAX72XX::UPDATE, MD_MAX72XX::OFF);
//  mx.clear();
//  mx.setChar(COLS-2, '8');
//  mx.setChar(2*COLS - 2, '2');
//  mx.setChar(3*COLS - 2, '3');
//  mx.setChar(4*COLS - 2, '2');
//  mx.setPoint(5, 16, 1);
//  mx.setPoint(2, 16, 1);
//  mx.control(MD_MAX72XX::UPDATE, MD_MAX72XX::ON);
//  delay(2000);
//  mx.clear();
//}

void initialize() {
  //showTime();
  generation = 0;
  
#ifdef USE_STALE_LIMIT
  last_pop = stale = 0;
#endif

  // flip will initialise the frame buffers to 0
  // randomize will intialise them... randomly.
  flip();
  flip();
  randomize();
  
  // Note when setting these the field it is mirrored
  // Gosper Gun Use at least 40 ROWS!
  // With eater
//  state[active][2] =  0x00003000;
//  state[active][3] =  0x00003000;
//  state[active][12] = 0x00007000;
//  state[active][13] = 0x00008800;
//  state[active][14] = 0x00010400;
//  state[active][15] = 0x00010400;
//  state[active][16] = 0x00002000;
//  state[active][17] = 0x00008800;
//  state[active][18] = 0x00007000;
//  state[active][19] = 0x00002000;
//  state[active][22] = 0x00001c00;
//  state[active][23] = 0x00001c00;
//  state[active][24] = 0x00002200;
//  state[active][25] = 0x000c0000;
//  state[active][26] = 0x00146300;
//  state[active][27] = 0x00100000;
//  state[active][28] = 0x00300000;
//  state[active][36] = 0x00000c00;
//  state[active][37] = 0x00000c00;
  // Gosper Gun Use at least 40 ROWS!
  //
//  state[active][2] =  0x00003000;
//  state[active][3] =  0x00003000;
//  state[active][12] = 0x00007000;
//  state[active][13] = 0x00008800;
//  state[active][14] = 0x00010400;
//  state[active][15] = 0x00010400;
//  state[active][16] = 0x00002000;
//  state[active][17] = 0x00008800;
//  state[active][18] = 0x00007000;
//  state[active][19] = 0x00002000;
//  state[active][22] = 0x00001c00;
//  state[active][23] = 0x00001c00;
//  state[active][24] = 0x00002200;
//  state[active][26] = 0x00006300;
//  state[active][36] = 0x00000c00;
//  state[active][37] = 0x00000c00;
  // Gliders
//  state[active][0] = 0x2;
//  state[active][1] = 0x1;
//  state[active][2] = 0x7;
//  state[active][5] = 0x20;
//  state[active][6] = 0x10;
//  state[active][7] = 0x70;
}

// Closed topology
bool get_state_closed (int i, int j) {
  if (i < 0 || i > (ROWS-1) || j < 0 || j > (COLS-1))
    return false; // Out of bounds cells count as dead.
  return ((state[active][i]) >> j) & 1;
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
  return (state[active][i] >> j) & 1;
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
      state[!active][i] |= (1UL << j); // type
      break;
    case 4:
      state[!active][i] |= ((uint32_t) get_state(i, j) << j);
      break;
    default:
      state[!active][i] &= ~(1UL << j); // type
  }
}

void flip() {
  int i;
  for(i = 0; i < ROWS; i++)
    state[active][i] = 0;
  active = !active;
}

void randomize() {
   int i;
   active = !active;
   for(i = 0; i < ROWS; i++)
    state[active][i] = random();
}

void sendBlock(uint8_t r, uint8_t c) {
  uint8_t block[8];
  uint8_t *block_state = (uint8_t *) state[active];
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
  uint32_t timer = millis();
  int i, j, pop = 0;
  for (i = 0; i < ROWS; i++) {
    for (j = 0; j < COLS; j++) {
      bool s = get_state(i, j);
      pop += s;
      update_state(i, j);
    }
  }
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
    flip();
    delay(DELAY);
      #ifdef USE_GENERATION_LIMIT
      }
      #endif
    #ifdef USE_STALE_LIMIT
    }
    #endif
  }
}
