all: crypto_serial crypto_mp crypto_tbb crypto_cilk

default: all

crypto_mp: mp.c key.c
	icc -std=c99 -o $@ mp.c key.c -fopenmp 

#crypto_tbb: tbb.cpp key.c
#	icpc -o $@ tbb.cpp key.c -ltbb

crypto_cilk: cilk.cpp
	icpc -o $@ cilk.cpp key.c -lcilkrts

crypto_serial: serial.c key.c
	icc -std=c99 -o crypto_serial serial.c key.c

clean:
	rm -f *.o crypto_serial crypto_mp crypto_tbb crypto_cilk decryptedOut encryptedOut
