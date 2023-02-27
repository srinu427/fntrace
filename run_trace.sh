g++ -finstrument-functions -g -c -o main.o main.cpp
g++ -g -c -o fntrace.o fntrace.cpp
g++ fntrace.o main.o -ldl -o main
./main
