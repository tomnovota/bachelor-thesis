# $^	all dependencies
# $<	first dependency
# $@	output
# $(@D) target's directory
# .PHONY prevents searching for these target names in OS and perform action avery time

TARGET = simulation
OBJECT_DIR = bin
SOURCE_DIR = src

CXX = g++
CXXFLAGS = -Wall -pedantic -g -fsanitize="address"
LD = g++
LDFLAGS = -Wall -pedantic -g -fsanitize="address"
LIBFLAGS = -lsfml-graphics -lsfml-window -lsfml-system

HEADERS := $(wildcard $(SOURCE_DIR)/*.h) # finds all header files
SOURCES := $(wildcard $(SOURCE_DIR)/*.cpp) # finds all source files
OBJECTS := $(patsubst $(SOURCE_DIR)/%.cpp, $(OBJECT_DIR)/%.o, $(SOURCES)) # for every source file assign equivalent object file
DEPENDENCIES := $(OBJECTS:%.o=%.d) # for every object file assign dependency file

.PHONY: all
all: compile

.PHONY: run
run : compile
	./$(TARGET) 1 "params/full_simulation" 1

anim :
	python animation/anim.py

runonly : compile
	./$(TARGET)

.PHONY: compile
compile: $(TARGET)

$(TARGET): $(OBJECT_DIR)/main
	cp $< $@

$(OBJECT_DIR)/main: $(OBJECTS)
	mkdir -p $(@D)
	$(LD) $(LDFLAGS) $^ -o $@

$(OBJECT_DIR)/%.o: $(SOURCE_DIR)/%.cpp
	mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -MMD -c $< -o $@

.PHONY: clean
clean:
	rm -rf $(OBJECT_DIR) $(TARGET) *.o *.out

-include $(DEPENDENCIES)
