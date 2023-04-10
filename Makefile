DEBUG=no
PROF=no
CXX=g++
INC=src

ifeq ($(PROF),yes)
    CFPROF=-pg
    LDPROF=-pg
else
    CFPROF=
    LDPROF=
endif

ifeq ($(DEBUG),yes)
    CFLAGS=-pipe -std=c++20 -g -O0 -Wshadow -Wall -Wextra -Wcast-qual -march=native -mpopcnt -msse -msse3 $(CFPROF) -I/$(INC)
    LDFLAGS=$(LDPROF) -lpthread
	TARGET = Zangdar_dbg
else
    CFLAGS=-pipe -std=c++20 -O3 -flto -DNDEBUG -fwhole-program -march=native -mpopcnt -msse -msse3 $(CFPROF) -I/$(INC)
    LDFLAGS= $(LDPROF) -flto -lpthread -static -s
	TARGET = Zangdar_rel
endif


CPP_FILES=$(shell find src -name "*.cpp")

OBJ=$(patsubst %.cpp, %.o, $(CPP_FILES))



all: $(TARGET)
ifeq ($(DEBUG),yes)
    $(info Génération en mode debug)
else
    $(info Génération en mode release)
endif

$(TARGET): $(OBJ)
	@$(CXX) -o $@ $^ $(LDFLAGS)


%.o: %.cpp
	@$(CXX) -o $@ -c $< $(CFLAGS)

.PHONY: clean mrproper

clean:
	@rm -rf src/*.o $(TARGET)

mrproper: clean
	@rm -rf $(TARGET)

