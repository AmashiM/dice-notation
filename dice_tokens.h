#pragma once
#ifndef DICE_TOKENS_H
#define DICE_TOKENS_H

#include "stdint.h"

enum TokenType {
    TYPE_NONE,
    TYPE_SPACE,
    TYPE_DICE,
    TYPE_NUM,
    TYPE_MATH,
    TYPE_ADD,
    TYPE_SUB,
    TYPE_MULT,
    TYPE_DIV,
    TYPE_EXP,
    TYPE_EMPTY,
    TYPE_GROUP_START,
    TYPE_GROUP_END,
    TYPE_KEEP,
    TYPE_KEEP_HIGH,
    TYPE_KEEP_LOW
};

enum TokenCategory {
    TOKEN_NONE,
    TOKEN_PAIR,
    TOKEN_NUMBER,
    TOKEN_MATH,
    TOKEN_DICE,
    TOKEN_KEEP,
    TOKEN_GROUP,
    TOKEN_GROUP_PLACEMENT
};

typedef struct tagPairToken {
    enum TokenType type;
    int value;
} PairToken;

typedef struct tagRealToken {
    enum TokenType type;
    uint64_t pos;
    void* special;
    enum TokenCategory category;
    uint8_t used;
} RealToken;

typedef struct tagNumberToken {
    RealToken* location;
    // union {
    //     uint8_t u8;
    //     uint16_t u16;
    //     uint32_t u32;
    //     uint64_t u64;
    //     int8_t s8;
    //     int16_t s16;
    //     int32_t s32;
    //     int64_t s64;
    //     float f;
    //     double d;
    // } value;

    int value;
    uint8_t used;
} NumberToken;

typedef struct tagKeepToken {
    RealToken* location;
    enum TokenType type;
    RealToken* value;
    uint8_t used;
} KeepToken;

typedef struct tagDiceToken {
    RealToken* location;
    NumberToken* amount;
    NumberToken* sides;
    KeepToken* keep_high;
    KeepToken* keep_low;
    uint8_t used;
} DiceToken;

typedef struct tagGroupToken {
    uint8_t priority;
    RealToken* start_pos;
    RealToken* end_pos;
    long value;
    uint8_t used;
} GroupToken;

typedef struct tagMathToken {
    RealToken* location;
    enum TokenType type;
    uint8_t used;
    GroupToken* group;
    uint8_t group_priority;
    uint8_t priority;
    RealToken* before;
    RealToken* after;
} MathToken;

typedef struct tagGroupPlacementToken {
    RealToken* location;
    uint8_t priority;
    uint8_t used;
    GroupToken* group;
} GroupPlacementToken;

#endif