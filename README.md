# Zangdar
A UCI chess engine written in C++11.

This project is somewhat a hobby, it will serve to learn the ropes of chess programming.
I also play chess, very humbly; and I was always curious of how the programs work.

I began some years ago, but without time, it never went far. I had only a move generator, a alpha-beta search, and that's almost all.
I have now enough time to spend on the program. By luck, reading the forums, I discovered Vice, and the videos. Thay are great, and very instructive.
So well, Im following the lessons, and Zangdar came to life. I found also the site from Bruce Moreno that explain a lot of things.

Why Zangdar ? Well look for the Naheulbeuk dungeon !!

I would like to thank specially the authors of Vice, TSCP, Gerbil. They helped me a lot understand several aspects of programmation.

At present, Zangdar can play honestly. I has the following features :

+ **language** 
  - written in C++

+ **board** 
  - 0x88

+ **search**
  - alpha-beta
  - quiescence
  - MVVVLA
  - killer heuristic
  - hash table

+ **evaluation**
  - material
  - piece/square table

+ **communication**
  - UCI
