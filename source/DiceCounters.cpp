#include "DiceCounters.h"

DiceCounters::DiceCounters()
{
}

DiceCounters::~DiceCounters()
{
}

int *DiceCounters::GetCounterFromType(TokenType type)
{
    switch(type){
        case TokenType::Dice: return &dice;
        case TokenType::Number: return &numbers;
        case TokenType::Add: return &add;
        case TokenType::Sub: return &sub;
        case TokenType::Mult: return &mult;
        case TokenType::Div: return &div;
        case TokenType::GroupStart: return &group_start;
        case TokenType::GroupEnd: return &group_end;
        case TokenType::KeepLow:
        case TokenType::KeepHigh: return &keep;
        case TokenType::Unexpected:
        default: return NULL;
    };
}
