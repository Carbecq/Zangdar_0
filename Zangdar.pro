TEMPLATE = app
CONFIG += console c++20
CONFIG -= app_bundle
CONFIG -= qt

CONFIG += static
QMAKE_LFLAGS += -static -static-libgcc -static-libstdc++ -lstdc++
DEFINES += STATIC

#------------------------------------------------------

# ATTENTION : ne pas mettre "\" car ils sont reconnus comme caractères spéciaux
# pour Github
#HOME_STR = "./"
# pour moi
HOME_STR = "D:/Echecs/Programmation/Zangdar/"
DEFINES += HOME='\\"$${HOME_STR}\\"'

#------------------------------------------------------

# DEFINES += USE_TUNER
# QMAKE_CXXFLAGS_RELEASE += -fopenmp
# LIBS += -fopenmp

#------------------------------------------------------

# DEFINES +=  DEBUG_EVAL
# DEFINES +=  DEBUG_LOG
# DEFINES +=  DEBUG_HASH
# DEFINES +=  DEBUG_TIME

#------------------------------------------------------
# https://gcc.gnu.org/onlinedocs/gcc/x86-Options.html

QMAKE_CXXFLAGS_RELEASE -= -g
QMAKE_CXXFLAGS_RELEASE -= -O
QMAKE_CXXFLAGS_RELEASE -= -O0
QMAKE_CXXFLAGS_RELEASE -= -O1
QMAKE_CXXFLAGS_RELEASE -= -O2

# si NDEBUG est défini, alors assert ne fait rien

QMAKE_CXXFLAGS_RELEASE += -pipe -std=c++20 -O3 -flto -DNDEBUG -fwhole-program
QMAKE_CXXFLAGS_RELEASE += -pedantic -Wshadow -Wall -Wextra -Wcast-qual -Wuninitialized

VERSION = 2.27.07
TARGET  = native

DEFINES += VERSION='\\"$${VERSION}\\"'

##------------------------------------------ 1) old
equals(TARGET, "old"){
TARGET = Zangdar-$${VERSION}-old
QMAKE_CXXFLAGS_RELEASE +=
}

##------------------------------------------ 2) x86-64
equals(TARGET, "x86-64"){
TARGET = Zangdar-$${VERSION}-x86-64

## x86-64
QMAKE_CXXFLAGS_RELEASE += -msse -msse2
}

##------------------------------------------ 3) popcnt
equals(TARGET, "popcnt"){
TARGET = Zangdar-$${VERSION}-popcnt

## x86-64
QMAKE_CXXFLAGS_RELEASE += -msse -msse2
## popcount
QMAKE_CXXFLAGS_RELEASE += -msse3 -mpopcnt
##sse 4.1
QMAKE_CXXFLAGS_RELEASE += -msse4.1 -msse4.2 -msse4a
## ssse3
QMAKE_CXXFLAGS_RELEASE += -mssse3
## mmx
QMAKE_CXXFLAGS_RELEASE += -mmmx
## avx
QMAKE_CXXFLAGS_RELEASE += -mavx
}

##------------------------------------------ 4) avx2 (2013)
equals(TARGET, "avx2"){
TARGET = Zangdar-$${VERSION}-avx2

## x86-64
QMAKE_CXXFLAGS_RELEASE += -msse -msse2
## popcount
QMAKE_CXXFLAGS_RELEASE += -msse3 -mpopcnt
##sse 4.1
QMAKE_CXXFLAGS_RELEASE += -msse4.1 -msse4.2 -msse4a
## ssse3
QMAKE_CXXFLAGS_RELEASE += -mssse3
## mmx
QMAKE_CXXFLAGS_RELEASE += -mmmx
## avx
QMAKE_CXXFLAGS_RELEASE += -mavx
## avx2
QMAKE_CXXFLAGS_RELEASE += -mavx2 -mfma
}

##------------------------------------------ 5) bmi2
equals(TARGET, "bmi2"){
TARGET = Zangdar-$${VERSION}-bmi2

## x86-64
QMAKE_CXXFLAGS_RELEASE += -msse -msse2
## popcount
QMAKE_CXXFLAGS_RELEASE += -msse3 -mpopcnt
##sse 4.1
QMAKE_CXXFLAGS_RELEASE += -msse4.1 -msse4.2 -msse4a
## ssse3
QMAKE_CXXFLAGS_RELEASE += -mssse3
## mmx
QMAKE_CXXFLAGS_RELEASE += -mmmx
## avx
QMAKE_CXXFLAGS_RELEASE += -mavx
## avx2
QMAKE_CXXFLAGS_RELEASE += -mavx2 -mfma
## bmi2
QMAKE_CXXFLAGS_RELEASE += -DUSE_PEXT -mbmi -mbmi2
}

##------------------------------------------ 6) native
equals(TARGET, "native"){
TARGET = Zangdar-$${VERSION}-5950X

QMAKE_CXXFLAGS_RELEASE += -march=native -DUSE_PEXT
}





#----------------------------------------------------------------------
QMAKE_CXXFLAGS_DEBUG += -g
QMAKE_CXXFLAGS_DEBUG -= -O
QMAKE_CXXFLAGS_DEBUG += -O0
QMAKE_CXXFLAGS_DEBUG -= -O1
QMAKE_CXXFLAGS_DEBUG -= -O2
QMAKE_CXXFLAGS_DEBUG -= -O3

QMAKE_CXXFLAGS_DEBUG += -pipe -std=c++20 -pedantic -Wshadow -Wall -Wextra -Wcast-qual -Wuninitialized
QMAKE_CXXFLAGS_DEBUG += -march=native
QMAKE_CXXFLAGS_DEBUG += -DUSE_PEXT


DISTFILES += \
    Makefile \
    src/pyrrhic/LICENSE


HEADERS += \
    src/Attacks.h \
    src/Bitboard.h \
    src/Board.h \
    src/Move.h \
    src/MoveList.h \
    src/MovePicker.h \
    src/OrderInfo.h \
    src/PolyBook.h \
    src/Search.h \
    src/Square.h \
    src/ThreadPool.h \
    src/Timer.h \
    src/TranspositionTable.h \
    src/Tuner.h \
    src/Uci.h \
    src/bitmask.h \
    src/defines.h \
    src/evaluate.h \
    src/pyrrhic/stdendian.h \
    src/pyrrhic/tbconfig.h \
    src/pyrrhic/tbprobe.h \
    src/types.h \
    src/zobrist.h


SOURCES += \
    src/Attacks.cpp \
    src/Board.cpp \
    src/MoveList.cpp \
    src/MovePicker.cpp \
    src/OrderInfo.cpp \
    src/PolyBook.cpp \
    src/Search.cpp \
    src/ThreadPool.cpp \
    src/Timer.cpp \
    src/TranspositionTable.cpp \
    src/Tuner.cpp \
    src/Uci.cpp \
    src/add_moves.cpp \
    src/attackers.cpp \
    src/bitmask.cpp \
    src/evaluate.cpp \
    src/fen.cpp \
    src/legal_evasions.cpp \
    src/legal_moves.cpp \
    src/legal_noisy.cpp \
    src/legal_quiet.cpp \
    src/main.cpp \
    src/makemove.cpp \
    src/perft.cpp \
    src/pyrrhic/tbprobe.cpp \
    src/quiescence.cpp \
    src/see.cpp \
    src/syzygy.cpp \
    src/tests.cpp \
    src/think.cpp \
    src/tools.cpp \
    src/undomove.cpp \
    src/valid.cpp
