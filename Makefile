all: bimi

bimi:
	clang++ -I/opt/halon/include/ -I/usr/local/include/libxml2/ -I/usr/local/include/ -L/usr/local/lib/ -fPIC -shared bimi.cpp -lxml2 -o bimi.so