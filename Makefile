CC=clang -mavx -O3
AS=clang -S -mavx -O3

TARGETS=ulong_v0 ulong_v1 ulong_v2 ulong_v3 m128_v0 m128_v1 m128_v1_2 m128_v1_3 m128_v2
SRC=$(addsuffix .c, $(TARGETS))
ASM=$(SRC:.c=.s)
PERF=$(SRC:.c=.perf)

all: $(ASM) summary

%.s: aes_counter.c %.c
	$(CC) $*.c -S -o $@

%.perf: %
	perf stat -o $@ -d ./$*
	grep 'cycles' $@

%:: aes_counter.c %.c
	$(CC) $^ -o $@

summary: $(PERF)
	for file in ${PERF}; \
	do \
	  grep -H 'cycles' $$file; \
	  grep -H 'instructions' $$file; \
	done

clean:
	rm -f $(ASM) $(TARGETS) $(PERF)
