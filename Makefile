DEBUG=no
PROF=no
CXX=g++
INC=src

HOMEDIR = -DHOME=\"/media/philippe/Travail/Echecs/Programmation/Zangdar/APP-2/\"

TARGET = Zangdar

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
    CFLAGS=-pipe -std=c++20 -march=native -O3 -flto -DNDEBUG -fwhole-program -DPOPCOUNT -mpopcnt -m64 -msse
    LDFLAGS=$(LDPROF)
else
    CFLAGS=-pipe -std=c++20 -march=native -O3 -flto -DNDEBUG -fwhole-program -DPOPCOUNT -mpopcnt -msse -msse3 $(CFPROF) -I/src $(OPTIONS)  $(HOMEDIR)
    LDFLAGS= $(LDPROF) -flto  -lpthread -static
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

