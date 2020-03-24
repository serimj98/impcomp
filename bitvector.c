#include <stdbool.h>
#include <stdbool.h>
#include <stdint.h>
#include "lib/contracts.h"
#include "lib/xalloc.h"
#include "lib/bitvector.h"

bitvector bitvector_new() {
  bitvector B = 0;
  return B;
}

bool bitvector_get(bitvector B, uint8_t i) {
  REQUIRES(i < BITVECTOR_LIMIT);
  if (((B >> (BITVECTOR_LIMIT-i-1)) & (bitvector)1) == true) return true;
  return false;
}

bitvector bitvector_flip(bitvector B, uint8_t i) {
  REQUIRES(i < BITVECTOR_LIMIT);
  bitvector C = B;
  C = C^((bitvector)1 << (BITVECTOR_LIMIT-i-1));
  return C;
}

bool bitvector_equal(bitvector B1, bitvector B2) {
  if (B1 != B2) return false;
  return true;
}
