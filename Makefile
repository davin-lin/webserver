CXX = g++
CFLAGS = -std=c++14 -O2 -Wall -g 

TARGET = server
OBJS = log/*.cpp buffer/*.cpp *.cpp
      
      
all: $(OBJS)
	$(CXX) $(CFLAGS) $(OBJS) -o $(TARGET)  -pthread 

clean:
	rm -rf ../bin/$(OBJS) $(TARGET)
