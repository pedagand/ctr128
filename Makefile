CC=clang -static -mavx -O3
AS=clang -S -mavx -O3

TARGETS=ulong_v0 ulong_v1 ulong_v2 m128_v0 m128_v1 m128_v1_2 m128_v1_3 m128_v2
SRC=$(addsuffix .c, $(TARGETS))
ASM=$(SRC:.c=.s)
PERF=$(SRC:.c=.perf)

all: $(ASM) $(TARGETS) $(PERF)

%.s: aes_counter.c %.c
	$(CC) $*.c -S -o $@

%: aes_counter.c %.c
	$(CC) $^ -o $@

%.perf: %
	perf stat -o $@ -d ./$*
	grep 'cycles:u' $@

clean:
	rm -f $(ASM) $(TARGETS) $(PERF)
