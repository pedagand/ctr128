#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <x86intrin.h>

#define unlikely(x)	(!__builtin_expect(!(x),1))
#define likely(x) (__builtin_expect(!!(x),1))

unsigned long MAX_LOOP;

static void print_ulong(unsigned long counter[static 2]){
  printf("%016lx %016lx\n", counter[0], counter[1]);
}

static void print_m128i(__m128i input){
  unsigned long out[2]  __attribute__ ((aligned (16)));
  _mm_store_si128((__m128i*)out, input);
  print_ulong(out);
}

static void print_input(__m128i input[static 8]){
  for (int i = 0; i < 8; i++){
    print_m128i(input[i]);
  }
}

static __m128i bench__count_ulong__v0(unsigned char n[static 16]){
  // Reference implementation

  unsigned long counter[2] __attribute__ ((aligned (32)));
  memcpy(counter, n, 16);
  counter[0] = __builtin_bswap64(counter[0]);
  counter[1] = __builtin_bswap64(counter[1]);

  print_ulong(counter);

  __m128i test_vector = _mm_setr_epi32(0x1234, 0x2345, 0x3456, 0x4567);
  __m128i mask = _mm_set_epi8(8,9,10,11,12,13,14,15,0,1,2,3,4,5,6,7);

  for (unsigned long i = 0; i < MAX_LOOP; i++){
    __m128i input[8];
    for (int j = 0; j < 8; j++) {
      // Counter is big-endian
      input[j] = _mm_shuffle_epi8(*((__m128i*)counter), mask);
      if (++counter[1] == 0) {
        printf("OVERFLOW!\n");
        ++counter[0];
      }
    }
    for (int j = 0; j < 8; j++){
      test_vector = _mm_add_epi32(input[j], test_vector);
    }
  }
  //print_m128i(test_vector);
  print_ulong(counter);

  return test_vector;
}

static __m128i bench__count_ulong__v1(unsigned char n[static 16]){
  // Identify fast-path

  unsigned long counter[2] __attribute__ ((aligned (32)));
  memcpy(counter, n, 16);
  counter[0] = __builtin_bswap64(counter[0]);
  counter[1] = __builtin_bswap64(counter[1]);

  __m128i test_vector = _mm_setr_epi32(0x1234, 0x2345, 0x3456, 0x4567);
  __m128i mask = _mm_set_epi8(8,9,10,11,12,13,14,15,0,1,2,3,4,5,6,7);

  for (unsigned long i = 0; i < MAX_LOOP; i++){
    __m128i input[8];

    if (counter[1] > (0xffffffffffffffff-8)) {
      for (int j = 0; j < 8; j++) {
        input[j] = _mm_shuffle_epi8(*((__m128i*)counter), mask);
        if (++counter[1] == 0) {
          ++counter[0];
        }
      }
    } else {
      for (int j = 0; j < 8; j++) {
        input[j] = _mm_shuffle_epi8(*((__m128i*)counter), mask);
        ++counter[1];
      }
    }
    for (int j = 0; j < 8; j++){
      test_vector = _mm_add_epi32(input[j], test_vector);
    }
  }

  return test_vector;
}

static __m128i bench__count_ulong__v2(unsigned char n[static 16]){
  // Hacker's delight implem of two word's increment

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

static __m128i bench__count_m128__v0(unsigned char n[static 16]){
  // Reference implementation on SSE registers

  __m128i one64ze64 = _mm_set_epi64x(-1, 0);
  __m128i test_vector = _mm_setr_epi32(0x1234, 0x2345, 0x3456, 0x4567);

  __m128i mask = _mm_set_epi8(8,9,10,11,12,13,14,15,
                              0,1,2,3,4,5,6,7);

  __m128i counter = _mm_load_si128((__m128i*) n);
  counter = _mm_shuffle_epi8(counter, mask);

  // print_m128i(counter);

  for (unsigned long i = 0; i < MAX_LOOP; i++){
    __m128i input[8];

    for (int j = 0; j < 8; j++) {
      input[j] = _mm_shuffle_epi8(counter, mask);
      __m128i overflow = _mm_cmpeq_epi64(counter, one64ze64);
      overflow = _mm_srli_si128(overflow,8);
      counter = _mm_sub_epi64(counter, one64ze64);
      counter = _mm_sub_epi64(counter, overflow);
    }
    for (int j = 0; j < 8; j++){
      test_vector = _mm_add_epi32(input[j], test_vector);
    }
  }

  // print_m128i(counter);

  return test_vector;
}

static __m128i bench__count_m128__v1(unsigned char n[static 16]){
  // Identify fast-path

  __m128i one64ze64 = _mm_set_epi64x(-1, 0);
  __m128i test_vector = _mm_setr_epi32(0x1234, 0x2345, 0x3456, 0x4567);

  __m128i mask = _mm_set_epi8(8,9,10,11,12,13,14,15,
                              0,1,2,3,4,5,6,7);

  __m128i counter = _mm_load_si128((__m128i*) n);
  counter = _mm_shuffle_epi8(counter, mask);

  for (unsigned long i = 0; i < MAX_LOOP; i++){
    __m128i input[8];

    if (((uint64_t*)&counter)[1] > 0xfffffffffffffff7) {

      for (int j = 0; j < 8; j++) {
        input[j] = _mm_shuffle_epi8(counter, mask);
        __m128i overflow = _mm_cmpeq_epi64(counter, one64ze64);
        overflow = _mm_srli_si128(overflow,8);
        counter = _mm_sub_epi64(counter, one64ze64);
        counter = _mm_sub_epi64(counter, overflow);
      }
    } else {
      for (int j = 0; j < 8; j++) {
        input[j] = _mm_shuffle_epi8(counter, mask);
        counter = _mm_sub_epi64(counter, one64ze64);
      }
    }

    for (int j = 0; j < 8; j++){
      test_vector = _mm_add_epi32(input[j], test_vector);
    }
  }

  return test_vector;
}

static __m128i bench__count_m128__v1_2(unsigned char n[static 16]){
  // Reorder branches to put forward the fast-path

  __m128i one64ze64 = _mm_set_epi64x(-1, 0);
  __m128i test_vector = _mm_setr_epi32(0x1234, 0x2345, 0x3456, 0x4567);

  __m128i mask = _mm_set_epi8(8,9,10,11,12,13,14,15,
                              0,1,2,3,4,5,6,7);

  __m128i counter = _mm_load_si128((__m128i*) n);
  counter = _mm_shuffle_epi8(counter, mask);

  for (unsigned long i = 0; i < MAX_LOOP; i++){
    __m128i input[8];

    if (! (((uint64_t*)&counter)[1] > 0xfffffffffffffff7)) {
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

static __m128i bench__count_m128__v1_3(unsigned char n[static 16]){
  // Reorder branches & clarify condition to put forward the fast-path

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

static __m128i bench__count_m128__v2(unsigned char n[static 16]){
  // Instruct clang of fast-path

  __m128i one64ze64 = _mm_set_epi64x(-1, 0);
  __m128i test_vector = _mm_setr_epi32(0x1234, 0x2345, 0x3456, 0x4567);

  __m128i mask = _mm_set_epi8(8,9,10,11,12,13,14,15,
                              0,1,2,3,4,5,6,7);

  __m128i counter = _mm_load_si128((__m128i*) n);
  counter = _mm_shuffle_epi8(counter, mask);

  for (unsigned long i = 0; i < MAX_LOOP; i++){
    __m128i input[8];

    if (unlikely(((uint64_t*)&counter)[1] > (0xfffffffffffffff7))) {
      for (int j = 0; j < 8; j++) {
        input[j] = _mm_shuffle_epi8(counter, mask);
        __m128i overflow = _mm_cmpeq_epi64(counter, one64ze64);
        overflow = _mm_srli_si128(overflow,8);
        counter = _mm_sub_epi64(counter, one64ze64);
        counter = _mm_sub_epi64(counter, overflow);
      }
    } else {
      for (int j = 0; j < 8; j++) {
        input[j] = _mm_shuffle_epi8(counter, mask);
        counter = _mm_sub_epi64(counter, one64ze64);
      }
    }

    for (int j = 0; j < 8; j++){
      test_vector = _mm_add_epi32(input[j], test_vector);
    }

  }

  return test_vector;
}

static __m128i bench__count_m128__v2_1(unsigned char n[static 16]){
  // Force shifts1 to be pre-computed out of the tight loop

  volatile __m128i shifts1 = _mm_set_epi64x(1, 0);
  __m128i shifts2 = _mm_set_epi64x(2, 0);
  __m128i shifts3 = _mm_set_epi64x(3, 0);
  __m128i shifts4 = _mm_set_epi64x(4, 0);
  __m128i shifts5 = _mm_set_epi64x(5, 0);
  __m128i shifts6 = _mm_set_epi64x(6, 0);
  __m128i shifts7 = _mm_set_epi64x(7, 0);
  __m128i shifts8 = _mm_set_epi64x(8, 0);

  __m128i one64ze64 = _mm_set_epi64x(-1, 0);
  __m128i test_vector = _mm_setr_epi32(0x1234, 0x2345, 0x3456, 0x4567);

  __m128i mask = _mm_set_epi8(8,9,10,11,12,13,14,15,
                              0,1,2,3,4,5,6,7);

  __m128i counter = _mm_load_si128((__m128i*) n);
  counter = _mm_shuffle_epi8(counter, mask);

  for (unsigned long i = 0; i < MAX_LOOP; i++){
    __m128i input[8];

    if (unlikely(((uint64_t*)&counter)[1] > (0xfffffffffffffff7))) {
      for (int j = 0; j < 8; j++) {
        input[j] = _mm_shuffle_epi8(counter, mask);
        __m128i overflow = _mm_cmpeq_epi64(counter, one64ze64);
        overflow = _mm_srli_si128(overflow,8);
        counter = _mm_sub_epi64(counter, one64ze64);
        counter = _mm_sub_epi64(counter, overflow);
      }
    } else {
      input[0] = _mm_shuffle_epi8(counter, mask);
      input[1] = _mm_shuffle_epi8(_mm_add_epi64(counter, shifts1), mask);
      input[2] = _mm_shuffle_epi8(_mm_add_epi64(counter, shifts2), mask);
      input[3] = _mm_shuffle_epi8(_mm_add_epi64(counter, shifts3), mask);
      input[4] = _mm_shuffle_epi8(_mm_add_epi64(counter, shifts4), mask);
      input[5] = _mm_shuffle_epi8(_mm_add_epi64(counter, shifts5), mask);
      input[6] = _mm_shuffle_epi8(_mm_add_epi64(counter, shifts6), mask);
      input[7] = _mm_shuffle_epi8(_mm_add_epi64(counter, shifts7), mask);
      counter = _mm_add_epi64(counter, shifts8);
    }

    for (int j = 0; j < 8; j++){
      test_vector = _mm_add_epi32(input[j], test_vector);
    }

  }

  return test_vector;
}

int main(int argc, char *argv[]){

  int size = 32;
  if (argc == 2){
    size = atoi(argv[1]);
  }

  MAX_LOOP = (1ULL << size) + 393;

  unsigned char n[16] = { 0x12, 0x23, 0x53, 0x32,
                          0x21, 0x32, 0x35, 0x23,
                          0xff, 0xff, 0xff, 0xff,
                          0x89, 0x1f, 0xc1, 0xff };
  __m128i out;


#ifdef ULONG_V0
  out = bench__count_ulong__v0(n);
#endif

#ifdef ULONG_V1
  out = bench__count_ulong__v1(n);
#endif

#ifdef ULONG_V2
  out = bench__count_ulong__v2(n);
#endif

#ifdef M128_V0
  out = bench__count_m128__v0(n);
#endif

#ifdef M128_V1
  out = bench__count_m128__v1(n);
#endif

#ifdef M128_V1_2
  out = bench__count_m128__v1_2(n);
#endif

#ifdef M128_V1_3
  out = bench__count_m128__v1_3(n);
#endif

#ifdef M128_V2
  out = bench__count_m128__v2(n);
#endif

#ifdef M128_V2_1
  out = bench__count_m128__v2_1(n);
#endif

  // for MAX_LOOP = (1ULL << 32) + 393;
  __m128i expected = _mm_set_epi64x(0x49668fef7a1ff655,
                                    0xac4fc88d0d06c744);

  if (memcmp(&expected, &out, 16) != 0){
    printf("ERROR!\n");
    printf("Expected: ");
    print_m128i(expected);
    printf("Got: ");
    print_m128i(out);
  } else {
    printf("Done.\n");
  }
}
