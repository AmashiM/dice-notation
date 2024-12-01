#pragma once

#include "stdio.h"

enum DiceErrorType {
    None,
    GotNull,
    FailAlloc,
    Logic
};

class DiceError {
public:
    const char* errmsg;
    DiceErrorType type;

    void SetError(DiceErrorType type, const char* errmsg);
};
