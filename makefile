CXX = g++
CPP = ./source/main.cpp
TARGET = ./bin/main
OBJECTS = $(CPP)

$(TARGET): $(CPP)
	$(CXX) $(OBJECTS) -o$(TARGET)