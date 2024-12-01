#pragma once

#include "stdio.h"
#include "stdlib.h"
#include "stdint.h"
#include "string.h"

#include "DiceError.h"

#define PAIRTOKEN_SZ sizeof(PairToken)

enum TokenType {
    Empty,
    Dice,
    Number,
    Keep,
    KeepHigh,
    KeepLow,
    TypeGroup,
    GroupStart,
    GroupEnd,
    Add,
    Sub,
    Mult,
    Div,
    Unexpected
};

typedef union {
    int i;
    void* ptr;
} PairTokenValue;

typedef struct tagPairToken {
    enum TokenType type;
    int pos;
    PairTokenValue value;
    void* range_pos;
    int internal_pos;
    uint8_t claimed;
} PairToken;

typedef struct tagDiceToken {
    void* amount;
    void* sides;
    void* keep_high;
    void* keep_low;
} DiceToken;

typedef struct tagGroupPlacement {
    PairToken* token;
    int priority;
    void* group;
    uint8_t claimed;
} GroupPlacement;

enum RangeTokenType
{
    RangeNumber,
    RangeDice,
    RangeKeep,
    RangeAdd,
    RangeSub,
    RangeMult,
    RangeDiv,
    RangeGroup
};

typedef struct tagRangeToken {
    PairToken* start;
    PairToken* end;
    PairToken* main;
    RangeTokenType type;
} RangeToken;

enum DiceResultType {
    DiceNumber,
    DiceKeep
};

typedef struct tagDiceResult {
    enum DiceResultType type;
    long value;
} DiceResult;

typedef struct tagRangeList {
    RangeToken* values;
    int count;
} RangeList;

typedef struct tagPairList {
    PairToken* values;
    int count;
} PairList;

typedef struct tagGroup {
    RangeList ranges;
    PairList tokens;
    PairToken* start;
    PairToken* end;
    PairToken* main;
    int group_priority;
} Group;

DiceResult* DiceResult_Create();

class DiceUtil {
public:
    static const char* TokenTypeToString(TokenType type);

    static const char* RangeTokenTypeToString(RangeTokenType type);
    
    static const char* DiceResultTypeToString(DiceResultType type);

    static void PrintPairToken(PairToken* token);

    static void PrintPointerArray(void** array, int size);

    static void PrintGroupPlacement(GroupPlacement* placement);

    static int RangeTokenSize(RangeToken* token);
};