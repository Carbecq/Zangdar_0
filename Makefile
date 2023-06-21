DEBUG=no
PROF=no
CXX=g++
SRC1=src
SRC2=src/pyrrhic

TARGET = Zangdar


# needed only for tests
# just comment it if you want
#HOMEDEF = -DHOME=\"/media/philippe/Travail/Echecs/Programmation/Zangdar/APP-2/\"

CFLAGS_COM = -pipe -std=c++20
CFLAGS_ARCH = -march=native -mpopcnt -msse -msse3
CFLAGS_DEB = -g -O0 -Wshadow -Wall -Wextra -Wcast-qual
CFLAGS_OPT = -O3 -flto -DNDEBUG -fwhole-program

ifeq ($(PROF),yes)
    CFPROF=-pg
    LDPROF=-pg
else
    CFPROF=
    LDPROF=-s
endif

ifeq ($(DEBUG),yes)
    CFLAGS= $(CFLAGS_COM) $(CFLAGS_DEB) $(CFLAGS_ARCH) $(CFPROF) -I/$(SRC1) -I/$(SRC2) $(HOMEDEF)
    LDFLAGS=$(LDPROF) -lpthread
else
    CFLAGS= $(CFLAGS_COM) $(CFLAGS_OPT) $(CFLAGS_ARCH) $(CFPROF) -I/$(SRC1) -I/$(SRC2) $(HOMEDEF)
    LDFLAGS= $(LDPROF) -flto -lpthread -static 
endif


CPP_FILES=$(shell find $(SRC1) -name "*.cpp") $(shell find $(SRC2) -name "*.cpp")

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
	@rm -rf $(SRC1)/*.o $(SRC2)/*.o $(TARGET)

mrproper: clean
	@rm -rf $(TARGET)

