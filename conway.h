/*
Conway is a library for running John Conway's Game of Life
Copyright Ali Raheem 2022 - https://github.com/ali-raheem/conway
MIT Licensed
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
    bool getCellStateClosed (int i, int j);
    bool getCellStateWrapped (int i, int j);
    bool getNextCellState (bool s, uint8_t sum);
    void updateCellState (uint16_t i, uint16_t j, uint32_t s, uint8_t sum);
    bool (Conway::*getCellState)(int, int);
  private:
    uint8_t activeLineBuffer;
    uint8_t lastPopulation;
};


template <class T>
Conway<T>::Conway(T* buffer, uint8_t length) {
  rows = length - 3;
  state = buffer;
  cols = 8 * sizeof(T);
  generation = population = staleness = 0;
  getCellState = &Conway::getCellStateWrapped;
};

// Closed topology
template <class T>
bool Conway<T>::getCellStateClosed (int i, int j) {
  if (i < 0 || i > (rows - 1) || j < 0 || j > (cols - 1))
    return false;
  return ((state[i]) >> j) & 1;
}

// Wrapped topology
template <class T>
bool Conway<T>::getCellStateWrapped (int i, int j) {
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
void Conway<T>::updateCellState (uint16_t i, uint16_t j, uint32_t s, uint8_t sum) {
  const T one = 1;
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
  uint16_t i, j;
  population = 0;
  activeLineBuffer = rows + 2;
  for (i = 0; i < rows; i++) {
    state[activeLineBuffer] = 0;
    bool oldState = (*this.*getCellState)(i, 0);
    uint8_t sum_l = (*this.*getCellState)(i - 1, -1) + (*this.*getCellState)(i, -1) + (*this.*getCellState)(i + 1, -1);
    uint8_t sum_m = (*this.*getCellState)(i - 1, 0) + oldState + (*this.*getCellState)(i + 1, 0);
    uint8_t sum_r;
    bool oldStateR;
    uint8_t liveCells;
    j = 0;
    oldStateR = (*this.*getCellState)(i, j + 1);
    sum_r = (*this.*getCellState)(i - 1, j + 1) + oldStateR + (*this.*getCellState)(i + 1, j + 1);
    liveCells = sum_l + sum_m + sum_r;
    state[activeLineBuffer] |= ((T) getNextCellState(oldState, liveCells)) << (sizeof(T)*8 - 1);
    population += oldState;
    oldState = oldStateR;
    sum_l = sum_m;
    sum_m = sum_r;
    T prevRow = state[(i == 0)? rows - 1 : i - 1];
    T currRow = state[i];
    T nextRow = state[(i + 1) % rows];
       prevRow >>= 1;
      currRow >>= 1;
      nextRow >>= 1;
    for (j = 1; j < cols - 1; j++) {
      oldStateR = !!(currRow & 0b10);
      sum_r = !!(prevRow & 0b10) + oldStateR + !!(nextRow & 0b10);
      liveCells = sum_l + sum_m + sum_r;
      state[activeLineBuffer] >>= 1;
      state[activeLineBuffer] |= ((T) getNextCellState(oldState, liveCells)) << (sizeof(T)*8 - 1);
      population += oldState;
      oldState = oldStateR;
      sum_l = sum_m;
      sum_m = sum_r;
      prevRow >>= 1;
      currRow >>= 1;
      nextRow >>= 1;
    }
    oldStateR = (*this.*getCellState)(i, j + 1);
    sum_r = (*this.*getCellState)(i - 1, j + 1) + oldStateR + (*this.*getCellState)(i + 1, j + 1);
    liveCells = sum_l + sum_m + sum_r;
    state[activeLineBuffer] >>= 1;
    state[activeLineBuffer] |= ((T) getNextCellState(oldState, liveCells)) << (sizeof(T)*8 - 1);
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
