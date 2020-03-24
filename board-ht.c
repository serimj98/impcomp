#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include "lib/contracts.h"
#include "lib/xalloc.h"
#include "lib/bitvector.h"
#include "lib/hdict.h"
#include "board-ht.h"

//write key_equal, key_hash, value_free functions?

bool key_equal(hdict_key x, hdict_key y) {
  bitvector* xbit = (bitvector*)x;
  bitvector* ybit = (bitvector*)y;
  return bitvector_equal(*xbit, *ybit);
}

size_t key_hash(hdict_key x) {
  bitvector* xb = (bitvector*)x;
  size_t hash = (*xb)%(bitvector)BITVECTOR_LIMIT;
  return hash;
}

void value_free(hdict_value x) {
  struct board_data* xboard = (struct board_data*)x;
  free(xboard);
}

hdict_t ht_new(size_t capacity) {
  REQUIRES(capacity > 0);
  hdict_t H = hdict_new(capacity, &key_equal, &key_hash, &value_free);
  ENSURES(H != NULL);
  return H;
}

struct board_data *ht_lookup(hdict_t H, bitvector B) {
  REQUIRES(H != NULL);
  return hdict_lookup(H, (hdict_key*)(&B));
}

void ht_insert(hdict_t H, struct board_data *DAT) {
  REQUIRES(H != NULL);
  REQUIRES(ht_lookup(H, DAT->board) == NULL);
  hdict_insert(H, (hdict_key*)(&(DAT->board)), (hdict_value*)DAT);
}
