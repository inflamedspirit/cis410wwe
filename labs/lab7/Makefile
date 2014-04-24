default: matmul_serial matmul_mp

all: matmul_serial matmul_mp matmul_cilk matmul_tbb

matmul_serial: matmul.c
	icc -o matmul_serial matmul.c -fopenmp

matmul_mp: matmul_mp.c
	icc -o matmul_mp matmul_mp.c -fopenmp	

matmul_mp_all: matmul_mp.c
	for ((i=1;i<10;i=i+1)); do echo "Compiling:$$i";\
	j=$$((2**$$i));\
	icc -o matmul_mp$$j -D BLOCK_SIZE=$$j matmul_mp.c -fopenmp;\
	done
	#Compiles several versions with different block sizes.
	#Must remove BLOCK_SIZE definition from main program

matmul_cilk:

matmul_tbb:

clean:
	rm matmul_serial matmul_mp matmul_cilk matmul_tbb 2>/dev/null
