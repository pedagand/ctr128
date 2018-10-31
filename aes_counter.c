#include "aes_counter.h"

extern __m128i bench__count(unsigned char n[static 16]);

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

  __m128i out = bench__count(n);

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
