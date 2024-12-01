#include "Tokens.h"

const char *DiceUtil::TokenTypeToString(TokenType type)
{
    switch(type){
        case TokenType::Empty: return "Empty";
        case TokenType::Dice: return "Dice";
        case TokenType::Number: return "Number";
        case TokenType::Keep: return "Keep";
        case TokenType::KeepHigh: return "KeepHigh";
        case TokenType::KeepLow: return "KeepLow";
        case TokenType::TypeGroup: return "Group";
        case TokenType::GroupStart: return "GroupStart";
        case TokenType::GroupEnd: return "GroupEnd";
        case TokenType::Add: return "Add";
        case TokenType::Sub: return "Sub";
        case TokenType::Mult: return "Mult";
        case TokenType::Div: return "Div";
        case TokenType::Unexpected: return "Unexpected";
        default: return "UNKNOWN";
    }
}

const char *DiceUtil::RangeTokenTypeToString(RangeTokenType type)
{
    switch(type){
        case RangeTokenType::RangeNumber: return "RangeNumber";
        case RangeTokenType::RangeDice: return "RangeDice";
        case RangeTokenType::RangeKeep: return "RangeKeep";
        case RangeTokenType::RangeAdd: return "RangeAdd";
        case RangeTokenType::RangeSub: return "RangeSub";
        case RangeTokenType::RangeMult: return "RangeMult";
        case RangeTokenType::RangeDiv: return "RangeDiv";
        case RangeTokenType::RangeGroup: return "RangeGroup";
        default: return "RangeUnknown";
    }
}

const char *DiceUtil::DiceResultTypeToString(DiceResultType type)
{
    switch(type){
        case DiceNumber: return "DiceNumber";
        case DiceKeep: return "DiceKeep";
        default: return "DiceUnknown";
    }
}

void DiceUtil::PrintPairToken(PairToken *token)
{
    printf("[%p] PairToken(type: %s, value_i: %d, value_ptr: %p, pos: %d, claimed: %d)", token, TokenTypeToString(token->type), token->value.i, token->value.ptr, token->pos, token->claimed);
}

void DiceUtil::PrintPointerArray(void **array, int size)
{
    for(int i = 0; i < size; i++){
        void* value = &array[i];
        if(value == NULL){
            printf("NULL");
        } else {
            printf("%p", value);
        }

        if(i+1 < size){
            printf(", ");
        }
    }
}

void DiceUtil::PrintGroupPlacement(GroupPlacement *placement)
{
    printf("GroupPlacement[%p](pos: %p, priority: %d)", placement, placement->token, placement->priority);
}

int DiceUtil::RangeTokenSize(RangeToken *token)
{
    return (token->end - token->start)+1;
}

DiceResult *DiceResult_Create()
{
    DiceResult* result = (DiceResult*)calloc(sizeof(DiceResult), 1);
    return result;
}
