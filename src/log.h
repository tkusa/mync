#include <iostream>

using namespace std;

#ifndef LOG_H
#define LOG_H


class Log
{
private:
    bool verbose_flg = false;
    bool debug_flg = false;
public:
    void setVerbose(bool val);
    void setDebug(bool val);
    void info(string msg);
    void error(string msg);
    void debug(string msg);
};

#endif
