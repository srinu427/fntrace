gcc -g -c -o fntrace.o fntrace.c
g++ -fPIC -g -c -finstrument-functions -o find_primes.o find_primes.cpp
g++ -shared -o libfind_primes.so find_primes.o
g++ -fPIC -g -c -finstrument-functions -o dlexmp.o dlexmp.cpp
g++ -shared -o dlexmp.so dlexmp.o
g++ -g -c -finstrument-functions -o main.o main.cpp -pthread
g++  main.o fntrace.o -L./ -ldl -lfind_primes -o main -pthread
export LD_LIBRARY_PATH=./:$LD_LIBRARY_PATH
./main
python3 ./parse_trace.py
