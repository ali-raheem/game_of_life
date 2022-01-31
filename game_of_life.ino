// Copyright 2022 Ali Raheem <github@shoryuken.me>
// https://github.com/ali-raheem/game_of_life

const char *LIVE = " @";
const char *DEAD = " -";

// Comment this out to not monitor for static population numbers
#define USE_STALE_LIMIT

// Comment this out to not limit numebr of generations
#define USE_GENERATION_LIMIT

// Comment this out to stop statistics bring printed
#define PRINT_STATS

#ifdef USE_GENERATION_LIMIT
const unsigned int GENERATION_LIMIT = 1000; // Reset after this many generations
#endif

#ifdef USE_STALE_LIMIT
const unsigned int STALE_LIMIT = 5; // Reset if population not changed in STALE_LIMIT generations
unsigned int last_pop;
unsigned int stale;
#endif

bool get_state_wrapped(int, int);
bool get_state_closed(int, int);
bool (*get_state)(int, int) = get_state_wrapped;

const unsigned int ROWS = 32;
unsigned long state[2][ROWS];
const unsigned int COLS = 8 * sizeof(state[0][0]); // bits in type used for state array unsigned long
const unsigned int DELAY = 25;
unsigned int generation;
bool active = false;


void initialize() {
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
  // A loan Glider
//  state[active][0] = 0x00020000;
//  state[active][1] = 0x00010000;
//  state[active][2] = 0x00070000;
  
}

void setup() {
  Serial.begin(115200);
  Serial.println("Game of Life - Arduino");
  Serial.println("Copyright 2022 Ali Raheem <github@shoryuken.me>");
  Serial.println("https://github.com/ali-raheem/game_of_life");
  initialize();
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
      state[!active][i] |= (1ULL << j); // type
      break;
    case 4:
      unsigned long s = get_state(i, j);
      state[!active][i] |= (s << j);
      break;
    default:
      state[!active][i] &= ~(1ULL << j); // type
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
    state[active][i] =  (unsigned long) analogRead(0) << 24 |
                        (unsigned long) analogRead(1) << 16 |
                        (unsigned long) analogRead(2) << 8  | 
                        (unsigned long) analogRead(3);
}

void loop() {
  unsigned long timer = millis();
  int i, j, pop = 0;
  for (i = 0; i < ROWS; i++) {
    for (j = 0; j < COLS; j++) {
      bool s = get_state(i, j);
      pop += s;
      if (s) {
        Serial.print(LIVE);
      } else {
        Serial.print(DEAD);
      }
      update_state(i, j);
    }
    Serial.println("");
  }
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
    #ifdef PRINT_STATS
    timer = millis() - timer;
    Serial.print("Generation: ");
    Serial.print(generation, DEC);
    Serial.print(" Population: ");
    Serial.print(pop, DEC);
    Serial.print(" Render time: ");
    Serial.print(timer, DEC);
    Serial.println("ms");
    #endif
    delay(DELAY);
      #ifdef USE_GENERATION_LIMIT
      }
      #endif
    #ifdef USE_STALE_LIMIT
    }
    #endif
  }
}
