 
 # File:   Makefile \
  Author: thibault  


PVMDIR = /usr/lib/pvm3
PVMLIBS = /usr/lib/libpvm3.a
CC = g++

all: client nameserver headoffice bankaccount
client: client.cpp
	g++ -o client client.cpp $(PVMLIBS)
nameserver: nameserver.cpp
	g++ -o nameserver nameserver.cpp $(PVMLIBS)
headoffice: headoffice.cpp
	g++ -o headoffice headoffice.cpp $(PVMLIBS)
bankaccount: bankaccount.cpp
	g++ -o bankaccount bankaccount.cpp $(PVMLIBS)