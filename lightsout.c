#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "lib/contracts.h"
#include "lib/bitvector.h"
#include "lib/hdict.h"
#include "lib/queue.h"
#include "lib/xalloc.h"
#include "lib/boardutil.h"
#include "board-ht.h"

void queue_elem_free(queue_elem x) {
  queue_t* xqueue = (queue_t*)x;
  free(xqueue);
}

bitvector press_button(bitvector board, uint8_t w, uint8_t h, uint8_t i) {
  board = bitvector_flip(board, i);
  if (i % w != 0) board = bitvector_flip(board, i-1); //first column
  if (i % w != w-1) board = bitvector_flip(board, i+1); //last column
  if (i / w != 0) board = bitvector_flip(board, i-w); //first row
  if (i / w != h-1) board = bitvector_flip(board, i+w); //last row
  return board;
}

//Breadth-First Search algorithm from Lightsout writeup

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "Usage: lightsout <board name>\n");
    return 1;
  }
  char *path = argv[1];
  bitvector* vec_out = xmalloc(sizeof(bitvector));
  uint8_t* width_out = xmalloc(sizeof(uint8_t));
  uint8_t* height_out = xmalloc(sizeof(uint8_t));
  file_read(path, vec_out, width_out, height_out);
  uint8_t height = *height_out;
  uint8_t width = *width_out;

  REQUIRES(is_valid_boardsize(*width_out, *height_out));
  queue_t Q = queue_new();
  hdict_t H = ht_new(BITVECTOR_LIMIT);
  struct board_data* init = xmalloc(sizeof(struct board_data));
  init->board = *vec_out;
  init->solution = bitvector_new();
  enq(Q, (queue_elem*)init);
  while (!queue_empty(Q)) {
    //Find a board that we haven't looked at from the queue
    struct board_data* B = deq(Q);
    //Consider all the moves
    for (uint8_t row = 0; row < height; row ++) {
      for (uint8_t col = 0; col < width; col++) {

        REQUIRES(is_valid_pos(row, col, width, height));

        uint8_t i = get_index(row, col, width, height);

        struct board_data* copy = xmalloc(sizeof(struct board_data));
        copy->test = B->test;
        copy->board = B->board;
        copy->solution = B->solution;

        bitvector newboard = press_button(B->board, width, height, i);

        bitvector empty = bitvector_new();
        if (bitvector_equal(newboard, empty)) {
          for (uint8_t sol = 0; sol < width*height; sol++) {
            if (bitvector_get(B->solution, sol) == true) {
              int srow = sol/width;
              int scol = sol%width;
              printf("%d:%d\n", srow, scol);
            }
          }
          hdict_free(H);
          queue_free(Q, &queue_elem_free);
          fprintf(stderr, "You got all the lights out!\n");
          return 0;
        }

        struct board_data* newB = xmalloc(sizeof(struct board_data));
        newB->board = newboard;
        newB->solution = bitvector_flip(B->solution, i);
        //newB->solution = B->solution;
        if (ht_lookup(H, newB->board) == NULL) {
          ht_insert(H, newB);
          enq(Q, (queue_elem*)newB);
        }
      }
    }
  }

  hdict_free(H);
  queue_free(Q, &queue_elem_free);
  fprintf(stderr, "No solution.\n");
  return 1;
}
