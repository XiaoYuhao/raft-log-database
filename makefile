###########################
#	Makefile
###########################

#source object target
SOURCE 	:= server.cpp common/network.cpp common/network.h common/package.h SSTable/sstable.h SSTable/sstable.cpp 
OBJS 	:= 
TARGET	:= server

#compile and lib parameter
CXX		:= g++
LIBS	:= -lpthread
LDFLAGS	:= 
DEFINES	:=
CFLAGS	:= 
CXXFLAGS:= -std=c++11
 
.PHONY: clean

#link parameter
LIB := 

#link
$(TARGET): $(OBJS) $(SOURCE)
	$(CXX) -o $@ $(CFLAGS) $(CXXFLAGS) $(LDFLAGS) $^ $(LIBS)

#clean
clean:
	rm -fr *.o
	rm -fr *.gch
	rm -rf server
	rm -rf *.log


