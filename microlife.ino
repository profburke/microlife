/*****************************************************
 *
 * microlife
 *
 * Copyright (c) 2014-2017 Matthew M. Burke.
 *
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 * 
 * 
 * Conway's Game of Life implemented for the SparkFun
 * (nee GeekAmmo) MicroView (https://www.sparkfun.com/products/12923) 
 *
 * See https://github.com/profburke/microlife/issues for issues, suggestions, etc.
 *
 *****************************************************/
#include <MicroView.h>

#define LIVE (1)
#define DEAD (0)

#define ONBOARD(x, y) \
  ( ((x) >= 0) && ((x) < LCDWIDTH) && ((y) >- 0) && ((y) < LCDHEIGHT) )

#define VALID_GRID(idx) \
  ( (idx == 0) || (idx == 1) )

#define COORDINATES_TO_SCREENBYTE(x, y) \
  ((x) + ((y)/8) * LCDWIDTH)
  

static uint8_t grid[2][LCDWIDTH * (LCDHEIGHT/8)];
static uint8_t CURRENT = 0;
static uint8_t NEXT = 1;




void resetGrids()
{
  memset(grid, 0, sizeof(grid));
}



// TODO: replace with memcpy and getScreenBuffer
void grid2screen(uint8_t gridIndex)
{
  if (!VALID_GRID(gridIndex)) { return; }
  
  for (uint8_t x = 0; x < LCDWIDTH; x++) {
    for (uint8_t y = 0; y < LCDHEIGHT; y++) {
      uint8_t value = get(gridIndex, x, y);
      uView.pixel(x, y, value, NORM);
    }
  }
}




// TODO: clean up this function, clear(), and get()
// by making use of bitClear(), bitSet(), bitWrite(), and bitRead()
void set(uint8_t gridIndex, uint8_t x, uint8_t y)
{
  if (!ONBOARD(x, y)) { return; }
  if (!VALID_GRID(gridIndex)) { return; }
  
  grid[gridIndex][COORDINATES_TO_SCREENBYTE(x, y)] |= _BV((y % 8));
}




void clear(uint8_t gridIndex, uint8_t x, uint8_t y)
{
  if (!ONBOARD(x, y)) { return; }
  if (!VALID_GRID(gridIndex)) { return; }

  grid[gridIndex][COORDINATES_TO_SCREENBYTE(x, y)] &= ~_BV((y % 8));
}




uint8_t get(uint8_t gridIndex, uint8_t x, uint8_t y)
{
  if (!ONBOARD(x, y)) { return DEAD; }
  if (!VALID_GRID(gridIndex)) { return DEAD; }

  uint8_t contents = grid[gridIndex][COORDINATES_TO_SCREENBYTE(x, y)];

  return ( (contents & _BV((y % 8))) > 0);
}




void randomFill(uint8_t gridIndex, uint8_t p)
{
  if (!VALID_GRID(gridIndex)) { return; }

  for (uint8_t x = 0; x < LCDWIDTH; x++) {
    for (uint8_t y = 0; y < LCDHEIGHT; y++) {
      if (random(100) < p) {
        set(gridIndex, x, y);
      } else {
        clear(gridIndex, x, y);
      }
    }
  }
}




void swapGrids() {
  CURRENT = 1 - CURRENT; 
  NEXT = 1 - NEXT; 
}




void showGrid()
{
  grid2screen(CURRENT);
  uView.display();
}




uint8_t calculate(uint8_t currentValue, uint8_t nNeighbors)
{
  if ( (currentValue == LIVE) && ((nNeighbors == 2) || (nNeighbors == 3)) ) { return LIVE; }

  if ( /* currentValue == DEAD && */ (nNeighbors == 3) ) { return LIVE; }

  return DEAD;
}




uint8_t countNeighbors(uint8_t gridIndex, uint8_t x, uint8_t y)
{
  uint8_t count = 0;
  
  count += get(gridIndex, x - 1, y - 1);
  count += get(gridIndex, x - 1, y);
  count += get(gridIndex, x - 1, y + 1);
  count += get(gridIndex, x, y - 1);
  count += get(gridIndex, x, y + 1);
  count += get(gridIndex, x + 1, y - 1);
  count += get(gridIndex, x + 1, y);
  count += get(gridIndex, x + 1, y + 1);
  
  return count;
}




void updateBuffer()
{
  for (uint8_t x = 0; x < LCDWIDTH; x++) {
    for (uint8_t y = 0; y < LCDHEIGHT; y++) {
      uint8_t currentValue = get(CURRENT, x, y);
      uint8_t nNeighbors = countNeighbors(CURRENT, x, y);
      uint8_t newValue = calculate(currentValue, nNeighbors);
      if (newValue == LIVE) {
        set(NEXT, x, y);
      } else {
        clear(NEXT, x, y);
      }
    }
  }
  swapGrids();
}




void setup() 
{
  uView.begin();
  uView.clear(PAGE);
  resetGrids();
  
  randomSeed(analogRead(0));
  
  randomFill(CURRENT, 20);  
  showGrid();
}



// if MAXGEN is 0, then we don't limit the evolution of the CA
// otherwise, we reset every MAXGEN steps 
#define MAXGEN 80
static uint8_t generation = 0;

void loop() 
{
  delay(150);
  updateBuffer();

  if (MAXGEN) {
    generation += 1;
  }
  showGrid();
  if (MAXGEN && generation >= MAXGEN) {
    generation = 0;
    randomFill(CURRENT, 20);
  }
}



