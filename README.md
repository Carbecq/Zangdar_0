# Zangdar
A UCI chess engine written in C++17

This project is somewhat a hobby, it will serve to learn the ropes of chess programming.
I also play chess, very humbly; and I was always curious of how the programs work.

I began some years ago, but without time, it never went far. I had only a move generator, a alpha-beta search, and that's almost all.
I have now enough time to spend on the program. By luck, reading the forums, I discovered Vice, and the videos. Thay are great, and very instructive.
So well, Im following the lessons, and Zangdar came to life. I found also the site from Bruce Moreland that explain a lot of things.

Why Zangdar ? Well look for the Naheulbeuk dungeon !!

I would like to thank specially the authors of Vice, TSCP, Gerbil. They helped me a lot understand several aspects of programmation.
I also use the M42 library for generating attacks; and took inspiration from the Libchess library. 

At present, Zangdar can play honestly. I done several matches, and I think Zangdar has an elo of about 2550.
It has the following features :

+ **Language** 
  - Written in C++17

+ **Board** 
  - Magic Bitboard

+ **Search**
  - Iterative Deepening
  - Alpha-Beta  , Fail Soft
  - Quiescence
  - MVVVLA
  - Killer Heuristic
  - Transposition Table, with 4 buckets
  - Null Move 
  - Late Move Reduction
  - Razoring

+ **Parallel**
  - can use several threads

+ **Evaluation**
  - Material
  - Piece/Square Table
  - Positional bonus
  - Mobility

+ **Communication**
  - UCI
  - Options to change OpeningBook usage, OpeningBook location, Hash size, Threads number
  
+ **Usage**
I provide two binaries, one for Windows and one for Linux. The Windows one is copmpiled in static, so you don't need extrernal libraries. The Linux one is done with Linux Ubuntu, also in static. I can't say if it will work with another distro.
By default, the opening book is disabled. It can be re-enabled by using an uci option. Zangdar will search the book in the same directory as the binary. This location can too be specified with an uci option. The book is of polyglot format, and must be named 'book.bin'.

+ **Compilation**
I provide a Makefile that you can use to compile Zangdar. You must have a C++ compiler that use at least C++17. I use personnaly g++. 
Since my computer is rather old, I can only use ancient possibilities like popcount. If you have a newer computer, you can certainly use  bmi2, avx2... Look in the makefile and add what you think is needed. I can't unfortunately test them.
