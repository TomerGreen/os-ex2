SOURCE=tests.cpp thread.cpp uthreads.cpp


tests: $(SOURCE)
	g++ -std=c++11 -Wall $(SOURCE) -o tests

tar:
	tar -cvf ex2.tar general.h thread.cpp thread.h uthreads.cpp uthreads.h blackbox.h Makefile README

shirtest:thread.cpp uthreads.cpp ./test/main.cpp
    g++ -std=c++11 -Wall thread.cpp uthreads.cpp ./test/main.cpp -o shirTest