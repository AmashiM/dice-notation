#pragma once

#include "stdio.h"
#include "stdlib.h"
#include "stdint.h"
#include "string.h"

#include "Tokens.h"

typedef struct tagDiceBaseCounter {
    int dice;
    int keep;
    int numbers;
    int add;
    int sub;
    int mult;
    int div;
    int group_start;
    int group_end;
    int group;
} DiceBaseCounter;

int* DiceBaseCounter_GetCounterFromType(DiceBaseCounter* counter, TokenType type);

class DiceCounters {
public:
    DiceCounters();
    ~DiceCounters();

    int real_token_count;

    int dice;
    int keep;
    int numbers;

    int add;
    int sub;
    int mult;
    int div;

    int group_start;
    int group_end;

    int group;

    int ranges;

    int range_pos;

    int* GetCounterFromType(TokenType type);
};