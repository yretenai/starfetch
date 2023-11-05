SRC = src/starfetch.cpp
#CC = g++
CFLAGS := -Wall -Wextra -O2 -ggdb -std=c++20 -pipe -pedantic -Wundef -Wshadow -W -Wwrite-strings -Wcast-align -Wstrict-overflow=5 -Wconversion -Wpointer-arith -Wformat=2 -Wsign-compare -Wendif-labels -Wredundant-decls -Winit-self
PREFIX ?= /usr/local

all: starfetch

starfetch: 
	${CXX} ${CFLAGS} -DSTARFETCH_PREFIX="${PREFIX}" ${SRC} -o starfetch

install:
	mkdir -p ${PREFIX}/share/starfetch
	cp -rf ./res/* ${PREFIX}/share/starfetch/
	chmod 711 starfetch
	cp starfetch ${PREFIX}/bin/starfetch

uninstall:
	rm -rf ${PREFIX}/bin/starfetch
	unlink ${PREFIX}/share/starfetch

clean:
	rm starfetch

.PHONY: all starfetch install uninstall clean
