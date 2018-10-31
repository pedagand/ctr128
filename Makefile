CC=clang -mavx -O3

TARGETS=ULONG_V0 ULONG_V1 ULONG_V2 M128_V0 M128_V1 M128_V1_2 M128_V1_3 M128_V2 M128_V2_1
ASM=$(TARGETS:%=aes_counter_%.s)
BIN=$(TARGETS:%=aes_counter_%)
PERF=$(TARGETS:%=aes_counter_%.perf)

all: $(ASM) $(BIN) $(PERF)

aes_counter_%.s: aes_counter.c
	$(CC) -D$* $^ -S -o $@

aes_counter_%: aes_counter.c
	$(CC) -D$* $^ -o $@

aes_counter_%.perf: aes_counter_%
	perf stat -o $@ -d ./$^

summary: $(PERF)
	for file in ${PERF}; \
	do \
	  grep -H 'cycles:u' $$file; \
	  grep -H 'instructions:u' $$file; \
	done

clean:
	rm -f $(ASM) $(BIN) $(PERF)
