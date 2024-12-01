#include "DiceRangeMaker.h"

DiceRangeMaker::DiceRangeMaker()
{
}

DiceRangeMaker::~DiceRangeMaker()
{
}

void DiceRangeMaker::SetTokens(PairToken *tokens, int size)
{
    this->tokens = tokens;
    this->size = size;
    real_size = size;
}

int DiceRangeMaker::CountTokens()
{
    for(int i = 0; i < size; i++){
        PairToken* token = &tokens[i];

        switch(token->type){
            case TokenType::Empty:
                real_size--;
                break;
            case TokenType::Number:
                counters.numbers++;
                break;
            case TokenType::Dice:
                counters.dice++;
                break;
            case TokenType::Add:
                counters.add++;
                break;
            case TokenType::Sub:
                counters.sub++;
                break;
            case TokenType::Mult:
                counters.mult++;
                break;
            case TokenType::Div:
                counters.div++;
                break;
            case TokenType::GroupEnd:
                counters.group_end++;
                break;
            case TokenType::GroupStart:
                counters.group_start++;
                break;
            case TokenType::TypeGroup:
                counters.group++;
                break;
        }
    }
    return 0;
}

int DiceRangeMaker::Process()
{
    

    return 0;
}

void DiceRangeMaker::SetError(DiceErrorType type, const char *errmsg)
{
    error.errmsg = errmsg;
    error.type = type;
}
