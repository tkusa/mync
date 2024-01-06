#include <iostream>
#include <string>

#include "log.h"

using namespace std;

void Log::setVerbose(bool val) {
    this->verbose_flg = val;
}

void Log::setDebug(bool val) {
    this->debug_flg = val;
}
void Log::info(string msg) {
    if (this->verbose_flg) {
        cerr << msg << endl;
    }
}

void Log::debug(string msg) {
    if (this->debug_flg) {
        cerr << msg << endl;
    }
}

void Log::error(string msg) {
    if (this->verbose_flg) {
        cerr << msg << endl;
    }
}