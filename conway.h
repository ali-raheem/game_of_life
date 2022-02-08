/*
Conway is a library for running John Conway's Game of Life
Copyright Ali Raheem 2022 - https://github.com/ali-raheem/conway
MIT Licensed
File version: 6db345b0733da7751e8f3cf59a212976cac4e349 08/02/2022 2020 BST
*/

#ifndef CONWAY_H
#define CONWAY_H

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
uint16_t Conway<T>::next() {
  uint16_t i, j;
  population = 0;
  activeLineBuffer = rows + 2;
  for (i = 0; i < rows; i++) {
    state[activeLineBuffer] = 0;
    bool oldState = (*this.*getCellState)(i, 0);
    uint8_t sum_l = (*this.*getCellState)(i - 1, -1) + (*this.*getCellState)(i, -1) + (*this.*getCellState)(i + 1, -1);
    uint8_t sum_m = (*this.*getCellState)(i - 1, 0) + oldState + (*this.*getCellState)(i + 1, 0);
    for (j = 0; j < cols; j++) {
      bool oldStateR = (*this.*getCellState)(i, j + 1);
      uint8_t sum_r = (*this.*getCellState)(i - 1, j + 1) + oldStateR + (*this.*getCellState)(i + 1, j + 1);
      uint8_t liveCells = sum_l + sum_m + sum_r;
      updateCellState(i, j, oldState, liveCells);
      population += oldState;
      oldState = oldStateR;
      sum_l = sum_m;
      sum_m = sum_r;
    }
    activeLineBuffer = rows + (i % 2);
    if (i > 1)  state[i - 1] = state[activeLineBuffer];
  }
  state[0] = state[rows + 2];
  state[rows - 1] = state[rows + (i % 2)];
  generation++;
  if (population == lastPopulation) {
      staleness++;
  } else {
      lastPopulation = population;
      staleness = 0;
  }
  return population;
}

#endif
