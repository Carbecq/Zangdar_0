#ifndef UCI_H
#define UCI_H

class Uci;

#include <string>

class Uci
{
public:
    Uci() = default;

    void run();
    void processCommand();
    void stop();
    void go(std::istringstream& is);


private:

};

#endif // UCI_H
