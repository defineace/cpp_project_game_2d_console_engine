CXX = g++
CPP = ./src/main.cpp
TARGET = ./bin/main
OBJECTS = $(CPP)

$(TARGET): $(CPP)
	$(CXX) $(OBJECTS) -o$(TARGET)