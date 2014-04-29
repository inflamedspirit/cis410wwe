all: stencil_serial stencil_openmp stencil_cilk stencil_tbb

stencil_serial: stencil_serial.cpp
	icpc -std=c++11 -o stencil_serial stencil_serial.cpp -Wall -Wextra -lopencv_core -lopencv_highgui -lm

stencil_openmp: stencil_mp.cpp
	icpc -std=c++11 -o stencil_openmp stencil_mp.cpp -Wall -Wextra -lopencv_core -lopencv_highgui -lm -fopenmp

stencil_cilk: stencil_cilk.cpp
	icpc -o $@ $^ -std=c++11 -Wall -Wextra -lopencv_core -lopencv_highgui -lm -lcilkrts

stencil_tbb: stencil_tbb.cpp
	icpc -o $@ $^ -std=c++11 -Wall -Wextra -lopencv_core -lopencv_highgui -lm -ltbb

clean:
	rm -f *.o stencil_serial

.PHONY: clean
