# Conway's Game of Life

A simple Arduino sketch for Conway's Game of Life.

The goal was to be somewhat light on memory - each cell state is held by a single bit.

Uses 2 frame buffers which could be simplified and the algorithim could do with some optimizing.

![Screenshot of the sketch running with serial output](screenshot.png).

## Settings

Code is hopefully self explanatory but:

1. First decide on a topology and set `get_state` to point to either `get_state_wrapped` or `get_state_closed` (or define your own).
2. Decide on initial conditions in `setup` randomize, a glider... both?
3. You can change string printed for living or dead cells with `LIVE` and `DEAD`.
4. `USE_GENERATION_LIMIT` and ``GENERATION_LIMIT` resets after a set number of generations
5. `USE_STALE` and `STALE_LIMIT` resets if population hasn't changes for a number of generations
6. `ROWS` can be changed.
7. `COLS` **shouldn't** be changed, instead you could change the type of state to be an array of unsigned int or unsigned char. But you probably don't want this cause you could only make it smaller!
