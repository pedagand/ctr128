#include "aes_counter.h"

__m128i bench__count(unsigned char n[static 16]){
  __m128i one64ze64 = _mm_set_epi64x(-1, 0);
  __m128i test_vector = _mm_setr_epi32(0x1234, 0x2345, 0x3456, 0x4567);

  __m128i mask = _mm_set_epi8(8,9,10,11,12,13,14,15,
                              0,1,2,3,4,5,6,7);

  __m128i counter = _mm_load_si128((__m128i*) n);
  counter = _mm_shuffle_epi8(counter, mask);

  for (unsigned long i = 0; i < MAX_LOOP; i++){
    __m128i input[8];

    if (((uint64_t*)&counter)[1] <= (0xfffffffffffffff7)) {
      for (int j = 0; j < 8; j++) {
        input[j] = _mm_shuffle_epi8(counter, mask);
        counter = _mm_sub_epi64(counter, one64ze64);
      }
    } else {
      for (int j = 0; j < 8; j++) {
        input[j] = _mm_shuffle_epi8(counter, mask);
        __m128i overflow = _mm_cmpeq_epi64(counter, one64ze64);
        overflow = _mm_srli_si128(overflow,8);
        counter = _mm_sub_epi64(counter, one64ze64);
        counter = _mm_sub_epi64(counter, overflow);
      }
    }

    for (int j = 0; j < 8; j++){
      test_vector = _mm_add_epi32(input[j], test_vector);
    }

  }

  return test_vector;
}
