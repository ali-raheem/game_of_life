const int ROWS = 32;
const int COLS = 32; // bits in type used for state array unsigned long
bool active = false;
unsigned long state[2][ROWS];

void setup() {
  Serial.begin(115200);
  Serial.println("Game of Life - Arduino 32x32");
  flip();
  randomize();
}

bool get_state (int i, int j) {
  if (i < 0 || i > (ROWS-1) || j < 0 || j > (COLS-1))
    return false;
  return ((state[active][i]) >> j) & 1;
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
      state[!active][i] |= 1 << j;
      break;
    case 4:
      int s = get_state(i, j);
      state[!active][i] |= (s << j);
      break;
    default:
      state[!active][i] &= (0xff - (1 << j));
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
    state[active][i] =  analogRead(0) << 24 |
                        analogRead(0) << 16 |
                        analogRead(0) << 8  | 
                        analogRead(0);
}

void loop() {
  int i, j;
  for (i = 0; i < ROWS; i++) {
    for (j = 0; j < COLS; j++) {
      bool s = get_state(i, j);
      if (s) {
        Serial.print(" X ");
      } else {
        Serial.print(" O ");
      }
      update_state(i, j);
    }
    Serial.println("");
  }
  Serial.println("");

  flip();
  delay(500);
}
