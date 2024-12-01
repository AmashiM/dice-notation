#pragma once

#include "Tokens.h"
#include "DiceError.h"

#include "DiceCounters.h"

class DiceRangeMaker {
public:
    DiceRangeMaker();
    ~DiceRangeMaker();

    void SetTokens(PairToken* tokens, int size);


    int Process();

    void SetError(DiceErrorType type, const char* errmsg);

private:
    int CountTokens();

    DiceBaseCounter counters;
    DiceError error;
    RangeToken* ranges;
    PairToken* tokens;
    int size;
    int real_size;

};