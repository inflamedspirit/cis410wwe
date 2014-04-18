all: pass_init pass_mp pass_tbb pass_cilk

default: pass_init pass_mp pass_tbb pass_cilk

pass_mp: pass_mp.c
	icc -o $@ $@.c -fopenmp -lcrypto

pass_tbb: pass_tbb.cpp
	icc -o $@ $@.cpp -ltbb -lcrypto

pass_cilk: pass_cilk.c
	icc -o $@ $@.c -lcilkrts -lcrypto

pass_init: pass_init.c
	icc -std=c99 -o pass_init pass_init.c -lcrypto

clean:
	rm *.o pass_init
