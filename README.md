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
