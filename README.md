# Conway's Game of Life

A simple Arduino sketch for Conway's Game of Life.

![screenshot.png](Screenshot of the sketch running with serial output).

The goal was to be somewhat light on memory - each cell state is held by a single bit.

It would be exandable to 64x64 I suspect without too much difficulty.

Uses 2 frame buffers which could be simplified and the algorithim could do with some optimizing.