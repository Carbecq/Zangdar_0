DEBUG=no
PROF=no
CXX=g++
INC=src

# needed only for tests
# just comment it if you want
HOMEDIR = -DHOME=\"/media/philippe/Travail/Echecs/Programmation/Zangdar/APP-2/\"


# OPTIONS=-DHASH -DDEBUG_EVAL -DDEBUG_LOG -DDEBUG_HASH -DNEW_EVAL -DPVS -DLMR -DPRETTY
OPTIONS=-DHASH -DTT_XOR  -DLMR -DNEW_EVAL 

ifeq ($(PROF),yes)
    CFPROF=-pg
    LDPROF=-pg
else
    CFPROF=
    LDPROF=
endif

ifeq ($(DEBUG),yes)
    CFLAGS=-pipe -std=c++20 -g -O0 -Wshadow -Wall -Wextra -Wcast-qual -march=native -mpopcnt -msse -msse3 $(CFPROF) -I/$(INC) $(OPTIONS) $(HOMEDIR) 
    LDFLAGS=$(LDPROF) -lpthread
	TARGET = Zangdar_dbg
else
    CFLAGS=-pipe -std=c++20 -O3 -flto -DNDEBUG -fwhole-program -march=native -mpopcnt -msse -msse3 $(CFPROF) -I/$(INC) $(OPTIONS) $(HOMEDIR)
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

