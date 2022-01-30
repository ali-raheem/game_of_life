// Copyright 2022 Ali Raheem <github@shoryuken.me>

const int ROWS = 32;
const int COLS = 32; // bits in type used for state array unsigned long
bool active = false;
unsigned long state[2][ROWS];

bool get_state_wrapped(int, int);
bool get_state_closed(int, int);

bool (*get_state)(int, int) = get_state_wrapped;

// Closed topology
bool get_state_closed (int i, int j) {
  if (i < 0 || i > (ROWS-1) || j < 0 || j > (COLS-1))
    return false;
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

void setup() {
  Serial.begin(115200);
  Serial.println("Game of Life - Arduino 32x32");
  // flip will initialise the frame buffers to 0
  // randomize will intialise them... randomly.
  flip();
  // flip();
  randomize();
  // Setting a glider to start with.
  state[active][15] = 0x00020000; // Glider
  state[active][16] = 0x00010000; //
  state[active][17] = 0x00070000; //
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
      state[!active][i] |= (1UL << j);
      break;
    case 4:
      unsigned long s = get_state(i, j);
      state[!active][i] |= (s << j);
      break;
    default:
      state[!active][i] &= ~(1UL << j);
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
                        (unsigned long) analogRead(0) << 16 |
                        (unsigned long) analogRead(0) << 8  | 
                        (unsigned long) analogRead(0);
}

void loop() {
  int i, j;
  for (i = 0; i < ROWS; i++) {
    for (j = 0; j < COLS; j++) {
      bool s = get_state(i, j);
      if (s) {
        Serial.print(" + ");
      } else {
        Serial.print(" - ");
      }
      update_state(i, j);
    }
    Serial.println("");
  }
  Serial.println("");

  flip();
 delay(50);
}
