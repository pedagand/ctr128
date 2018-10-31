#ifndef AES_COUNTER
#define AES_COUNTER

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

#endif
