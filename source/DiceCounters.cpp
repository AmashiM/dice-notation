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

int *DiceBaseCounter_GetCounterFromType(DiceBaseCounter *counter, TokenType type)
{
    switch(type){
        case TokenType::Dice: return &counter->dice;
        case TokenType::Number: return &counter->numbers;
        case TokenType::Add: return &counter->add;
        case TokenType::Sub: return &counter->sub;
        case TokenType::Mult: return &counter->mult;
        case TokenType::Div: return &counter->div;
        case TokenType::GroupStart: return &counter->group_start;
        case TokenType::GroupEnd: return &counter->group_end;
        case TokenType::KeepLow:
        case TokenType::KeepHigh: return &counter->keep;
        case TokenType::TypeGroup: return &counter->group;
        case TokenType::Unexpected:
        default: return NULL;
    };
}
