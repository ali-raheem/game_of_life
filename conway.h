/*
Conway is a library for running John Conway's Game of Life
Copyright Ali Raheem 2022 - https://github.com/ali-raheem/conway
MIT Licensed
File version: 2022-03-23 21:41 GMT
*/

#ifndef CONWAY_H
#define CONWAY_H

#define LIKELY(condition) __builtin_expect(static_cast<bool>(condition), 1)
#define UNLIKELY(condition) __builtin_expect(static_cast<bool>(condition), 0)

#ifdef __AVR
#include <stdint.h>
#else
#include <cstdint>
#endif

template<class T>
class Conway {
  public:
    Conway(T* buffer, uint8_t length);
    uint8_t rows;
    uint8_t cols;
    T* state;
    uint16_t generation;
    uint16_t population;
    uint16_t staleness;
    uint16_t next();
    void clear();
    bool getNextCellState (bool s, uint8_t sum);
    bool getCellState (int8_t i, int8_t j);
  private:
    uint8_t lastPopulation;
};


template <class T>
Conway<T>::Conway(T* buffer, uint8_t length) {
  rows = length - 3;
  state = buffer;
  cols = 8 * sizeof(T);
  generation = population = staleness = 0;
};

template <class T>
bool Conway<T>::getCellState (int8_t i, int8_t j) {
  if (i < 0)
    i = rows - 1;
  if (i > (rows - 1))
    i = 0;
  if (j < 0)
    j = cols - 1;
  if (j > (cols - 1))
    j = 0;
  return (state[i] >> j) & 1;
}

template <class T>
void Conway<T>::clear() {
  uint8_t i;
  for(i = 0; i < rows + 3; i++)
    state[i] = 0;
}

template <class T>
bool Conway<T>::getNextCellState (bool s, uint8_t sum) {
  switch (sum) {
    case 3:
      return true;
      break;
    case 4:
      return s;
      break;
    default:
      return false;
  }
}
template <class T>
uint16_t Conway<T>::next() {
  population = 0;
  uint8_t activeLineBuffer = rows + 2;
  for (uint8_t i = 0; i < rows; i++) {
    state[activeLineBuffer] = 0;
    T prevRow = state[(i == 0)? rows - 1 : i - 1];
    T currRow = state[i];
    T nextRow = state[(i + 1) % rows];
    uint8_t sum_l = !!(prevRow & (1 << (sizeof(T)*8 - 1))) + !!(currRow & (1 << (sizeof(T)*8 - 1))) + !!(nextRow & (1 << (sizeof(T)*8 - 1)));
    bool oldState = !!(currRow & 1);
    uint8_t sum_m = !!(prevRow & 1) + oldState + !!(nextRow & 1);    
//    uint8_t sum_l = getCellState(i - 1, -1) + getCellState(i, -1) + getCellState(i + 1, -1);
//    bool oldState = getCellState(i, 0);
//    uint8_t sum_m = getCellState(i - 1, 0) + oldState + getCellState(i + 1, 0);
    uint8_t sum_0 = sum_m;
//    uint8_t sum_r;
//    bool oldStateR;
//    uint8_t liveCells;
//    if(prevRow == 0 && currRow == 0 && nextRow == 0)
//      continue;
#ifdef __CONWAY_OPTIMIZE_LARGE
    prevRow >>= 1;
    currRow >>= 1;
    nextRow >>= 1;
    bool oldStateR = !!(currRow & 0b1);
    uint8_t sum_r = !!(prevRow & 0b1) + oldStateR + !!(nextRow & 0b1);
#else
    bool oldStateR = getCellState(i, 1);
    uint8_t sum_r = getCellState(i - 1, 1) + oldStateR + getCellState(i + 1, 1);
#endif
    uint8_t liveCells = sum_l + sum_m + sum_r;
    if(getNextCellState(oldState, liveCells)) {
      T one = 1;
      state[activeLineBuffer] |=  one << (sizeof(T)*8 - 1);
    }
    population += oldState;
    oldState = oldStateR;
    sum_l = sum_m;
    sum_m = sum_r;
    for (uint8_t j = 1; j < cols - 1; j++) {
      prevRow >>= 1;
      currRow >>= 1;
      nextRow >>= 1;
 #ifdef __CONWAY_OPTIMIZE_LARGE
      oldStateR = !!(currRow & 0b1);
      sum_r = !!(prevRow & 0b1) + oldStateR + !!(nextRow & 0b1);
 #else
      oldStateR = !!(currRow & 0b10);
      sum_r = !!(prevRow & 0b10) + oldStateR + !!(nextRow & 0b10);
 #endif
      liveCells = sum_l + sum_m + sum_r;
      state[activeLineBuffer] >>= 1;
      if(getNextCellState(oldState, liveCells)) {
        T one = 1;
        state[activeLineBuffer] |=  one << (sizeof(T)*8 - 1);
      }

      population += oldState;
      oldState = oldStateR;
      sum_l = sum_m;
      sum_m = sum_r;
    }
    liveCells = sum_l + sum_m + sum_0;
    state[activeLineBuffer] >>= 1;
    if(getNextCellState(oldState, liveCells)) {
      T one = 1;
      state[activeLineBuffer] |=  one << (sizeof(T)*8 - 1);
    }
    population += oldState;
    activeLineBuffer = rows + (i & 1);
    if (LIKELY(i > 1))  state[i - 1] = state[activeLineBuffer];
  }
  state[0] = state[rows + 2];
  state[rows - 1] = state[rows + (rows & 1)];
  generation++;
  if (UNLIKELY(population == lastPopulation)) {
      staleness++;
  } else {
      lastPopulation = population;
      staleness = 0;
  }
  return population;
}

#endif
