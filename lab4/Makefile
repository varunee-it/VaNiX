CXX = g++
CXXFLAGS = --coverage -O0 -g -std=c++11

all: test coverage

test: lab4/tests.cpp
	$(CXX) $(CXXFLAGS) lab4/tests.cpp -o lab4/tests.exe
	./lab4/tests.exe

coverage: test
	gcov -b main.cpp

clean:
	rm -f lab4/*.exe lab4/*.gcda lab4/*.gcno lab4/*.gcov main.cpp.gcov tests.cpp.gcov
