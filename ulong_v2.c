#include "aes_counter.h"

__m128i bench__count(unsigned char n[static 16]){

  unsigned long counter[2] __attribute__ ((aligned (32)));
  memcpy(counter, n, 16);
  counter[0] = __builtin_bswap64(counter[0]);
  counter[1] = __builtin_bswap64(counter[1]);

  __m128i test_vector = _mm_setr_epi32(0x1234, 0x2345, 0x3456, 0x4567);
  __m128i mask = _mm_set_epi8(8,9,10,11,12,13,14,15,0,1,2,3,4,5,6,7);

  for (unsigned long i = 0; i < MAX_LOOP; i++){
    __m128i input[8];


    for (int j = 0; j < 8; j++) {
      input[j] = _mm_shuffle_epi8(*((__m128i*)counter), mask);

      unsigned long x0 = counter[1];
      counter[1]++;
      int c = ((unsigned long) ((x0 | 1) & ~counter[1])) >> 63;
      counter[0] = counter[0] + c;
    }

    for (int j = 0; j < 8; j++){
      test_vector = _mm_add_epi32(input[j], test_vector);
    }
  }

  return test_vector;
}
