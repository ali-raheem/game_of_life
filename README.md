# Conway's Game of Life

A simple Arduino sketch for Conway's Game of Life.

The goal was to be somewhat light on memory - each cell state is held by a single bit.

Uses 2 frame buffers which could be simplified and the algorithim could do with some optimizing.

![Screenshot of the sketch running with serial output](screenshot.png).
![A Gosper gun](screenshot_gosper.png).
![Performance improves with printing less](screenshot-print-buffer.png).

## Settings

Code is hopefully self explanatory but:

1. First decide on a topology and set `get_state` to point to either `get_state_wrapped` or `get_state_closed` (or define your own).
2. Decide on initial conditions in `setup` randomize, a glider... both?
3. You can change string printed for living or dead cells with `LIVE` and `DEAD`.
4. `USE_GENERATION_LIMIT` and `GENERATION_LIMIT` resets after a set number of generations
5. `USE_STALE_LIMT` and `STALE_LIMIT` resets if population hasn't changes for a number of generations
6. `ROWS` can be changed.
7. `COLS` **shouldn't** be changed, instead you could change the type of state to be an array of unsigned int or unsigned char. But you probably don't want this cause you could only make it smaller!
8. `USE_STATS` controls if stats are printed.

## Performance

Slowest part of the code is (of course) printing to the Serial output. Using a short string for living and dead states is the biggest perforamnce improvement I've seen so far. Even better than buffering the print (which probably happens anyways).

Other speed ups like unrolling the inner loop, amortizing the sum into coloums, using the buffer as an `unsigned long[]` to copy up to 4bytes a time don't seem particularly useful. Probably because `avr-gcc` is smarter than me.

## Memory usage

