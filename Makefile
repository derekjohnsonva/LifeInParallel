CXX=g++
CXXFLAGS=-std=c++17 -g  -pthread -Wall -Werror -pedantic #-O3
# ASAN = Address SANitizer, see https://github.com/google/sanitizers/wiki/AddressSanitizer
CXXFLAGS_ASAN=-std=c++17 -g -Og -pthread -fsanitize=address -Wall -Werror -pedantic -D_GLIBCXX_DEBUG
# TSAN = Thread SANitizer, see https://github.com/google/sanitizers/wiki/ThreadSanitizer
CXXFLAGS_TSAN=-std=c++17 -g -Og -pthread -fsanitize=thread -fsanitize=undefined -Wall -Werror -pedantic -D_GLIBCXX_DEBUG

all: life life-asan life-tsan

%-asan.o: %.cc
	$(CXX) -c $(CXXFLAGS_ASAN) -o $@ $<

%-tsan.o: %.cc
	$(CXX) -c $(CXXFLAGS_TSAN) -o $@ $<

life.o: life.cc life.h
life-asan.o: life.cc life.h
life-tsan.o: life.cc life.h

life-parallel.o: life-parallel.cc life.h
life-parallel-asan.o: life-parallel.cc life.h
life-parallel-tsan.o: life-parallel.cc life.h

life-serial.o: life-parallel.cc life.h
life-serial-asan.o: life-parallel.cc life.h
life-serial-tsan.o: life-parallel.cc life.h

main.o: main.cc life.h timing.h
main-asan.o: main.cc life.h timing.h
main-tsan.o: main.cc life.h timing.h

timing.o: timing.cc timing.h
timing-asan.o: timing.cc timing.h
timing-tsan.o: timing.cc timing.h

life: main.o life.o timing.o life-parallel.o life-serial.o
	$(CXX) $(CXXFLAGS) -o $@ $^ -lpthread

life-asan: main-asan.o life-asan.o timing-asan.o life-parallel-asan.o life-serial-asan.o
	$(CXX) $(CXXFLAGS_ASAN) -o $@ $^ -lpthread

life-tsan: main-tsan.o life-tsan.o timing-tsan.o life-parallel-tsan.o life-serial-tsan.o
	$(CXX) $(CXXFLAGS_TSAN) -o $@ $^ -lpthread

clean:
	rm -f *.o

SUBMIT_FILENAME=life-submission-$(shell date +%Y%m%d%H%M%S).tar.gz

archive:
	tar -zcf $(SUBMIT_FILENAME) $(wildcard *.cc *.h *.hh *.H *.cpp *.C *.c *.txt *.md *.pdf) Makefile 
	@echo "Created $(SUBMIT_FILENAME); please upload and submit this file."

submit: archive

.PHONY: all clean archive submit
