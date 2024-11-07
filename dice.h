#pragma once
#ifndef DICE_H
#define DICE_H

#include "stdio.h"
#include "stdlib.h"
#include "stdint.h"
#include "string.h"
#include "time.h"
#include "math.h"

#include "dice_tokens.h"

#define DN_ERROR_FAILED_ALLOC 1
#define DN_ERROR_FAILED_ALLOC_CACHE 2
#define DN_ERROR_BAD_ARG 3
#define DN_ERROR_GOT_NULL 4
#define DN_ERROR_LOGIC 5
#define DN_ERROR_SYNTAX 6

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

void dice_notation_init();

void dice_notation_debug();

DiceNotation* dice_notation(const char* text);

long dice_notation_run(DiceNotation* notation);

#endif