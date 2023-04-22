DEBUG=no
PROF=no
CXX=g++
INC=src

VERSION = 2.15.03

TARGET     = Zangdar-$(VERSION)
VERSIONDEF = -DVERSION=\"$(VERSION)\"


# needed only for tests
# just comment it if you want
#HOMEDEF = -DHOME=\"/media/philippe/Travail/Echecs/Programmation/Zangdar/APP-2/\"

ifeq ($(PROF),yes)
    CFPROF=-pg
    LDPROF=-pg
else
    CFPROF=
    LDPROF=
endif

ifeq ($(DEBUG),yes)
    CFLAGS=-pipe -std=c++20 -g -O0 -Wshadow -Wall -Wextra -Wcast-qual -march=native -mpopcnt -msse -msse3 $(CFPROF) -I/$(INC) $(HOMEDEF) $(VERSIONDEF)
    LDFLAGS=$(LDPROF) -lpthread
else
    CFLAGS=-pipe -std=c++20 -O3 -flto -DNDEBUG -fwhole-program -march=native -mpopcnt -msse -msse3 $(CFPROF) -I/$(INC) $(HOMEDEF) $(VERSIONDEF)
    LDFLAGS= $(LDPROF) -flto -lpthread -static -s
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

