#pragma once
#ifndef DICE_H
#define DICE_H

#include "stdio.h"
#include "stdlib.h"
#include "stdint.h"
#include "string.h"

#define DN_ERROR_FAILED_ALLOC 1
#define DN_ERROR_FAILED_ALLOC_CACHE 2
#define DN_ERROR_BAD_ARG 3
#define DN_ERROR_GOT_NULL 4
#define DN_ERROR_LOGIC 5
#define DN_ERROR_SYNTAX 6

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

typedef struct tagMathToken {
    RealToken* location;
    enum TokenType type;
    uint8_t used;
    uint8_t priority;
} MathToken;

typedef struct tagGroupToken {
    uint8_t priority;
    RealToken* start_pos;
    RealToken* end_pos;
    long value;
    uint8_t used;
} GroupToken;

typedef struct tagGroupPlacementToken {
    RealToken* location;
    uint8_t priority;
    uint8_t used;
    GroupToken* group;
} GroupPlacementToken;

typedef struct tagDiceNotationCache {
    PairToken* tokens;

    DiceToken* dice;
    MathToken* math;
    NumberToken* number;
    KeepToken* keep;

    GroupPlacementToken* start_tokens;
    GroupPlacementToken* end_tokens;

    GroupToken* group;

    RealToken* real_tokens;
} DiceNotationCache;

typedef struct tagDiceNotationCounters {
    uint64_t real_token_count;

    uint8_t dice_count;
    uint8_t keep_count;
    uint8_t number_count;
    uint8_t group_count;
    uint8_t math_count;

    uint8_t group_start_count;
    uint8_t group_end_count;

    uint64_t real_token_pos;
    uint8_t group_priority;

    uint8_t dice_pos;
    uint8_t keep_pos;
    uint8_t math_pos;
    uint8_t number_pos;

    uint8_t group_start_pos;
    uint8_t group_end_pos;

    uint8_t group_pos;
} DiceNotationCounters;

typedef struct tagDiceNotationState {
    size_t length;
    const char* text;
} DiceNotationState;

typedef struct tagDiceNotation {
    DiceNotationCache cache;
    DiceNotationState state;
    DiceNotationCounters counters;
    int errcode;
    const char* errmsg;
} DiceNotation;

void dice_notation_debug();

DiceNotation* dice_notation(const char* text);

long dice_notation_run(DiceNotation* notation);

#endif