#include "DiceCache.h"

DiceCache::DiceCache()
{
    tokens = NULL;
    dice = NULL;
    numbers = NULL;
    keep = NULL;
    add = NULL;
    sub = NULL;
    mult = NULL;
    div = NULL;
    group_start = NULL;
    group_end = NULL;
}

DiceCache::~DiceCache()
{
    Clean();
}

int DiceCache::AllocateIndexes(DiceCounters *counters)
{
    dice = (void**)calloc(sizeof(void*), counters->dice);
    if(dice == NULL){
        SetError(DiceErrorType::FailAlloc, "failed to allocate space in cache for dice");
        return 1;
    }
    numbers = (void**)calloc(sizeof(void*), counters->numbers);
    if(numbers == NULL){
        SetError(DiceErrorType::FailAlloc, "failed to allocate space in cache for numbers");
        return 1;
    }
    keep = (void**)calloc(sizeof(void*), counters->keep);
    if(keep == NULL){
        SetError(DiceErrorType::FailAlloc, "failed to allocate space in cache for keep");
        return 1;
    }
    add = (void**)calloc(sizeof(void*), counters->add);
    if(add == NULL){
        SetError(DiceErrorType::FailAlloc, "failed to allocate space in cache for add");
        return 1;
    }
    sub = (void**)calloc(sizeof(void*), counters->sub);
    if(sub == NULL){
        SetError(DiceErrorType::FailAlloc, "failed to allocate space in cache for sub");
        return 1;
    }
    mult = (void**)calloc(sizeof(void*), counters->mult);
    if(mult == NULL){
        SetError(DiceErrorType::FailAlloc, "failed to allocate space in cache for mult");
        return 1;
    }
    div = (void**)calloc(sizeof(void*), counters->div);
    if(div == NULL){
        SetError(DiceErrorType::FailAlloc, "failed to allocate space in cache for div");
        return 1;
    }
    group_start = (GroupPlacement*)calloc(sizeof(GroupPlacement), counters->group_start);
    if(group_start == NULL){
        SetError(DiceErrorType::FailAlloc, "failed to allocate space in cache for group_start");
        return 1;
    }
    group_end = (GroupPlacement*)calloc(sizeof(GroupPlacement), counters->group_end);
    if(group_end == NULL){
        SetError(DiceErrorType::FailAlloc, "failed to allocate space in cache for group_end");
        return 1;
    }

    return 0;
}

void **DiceCache::GetCacheFromTokenType(TokenType type)
{
    switch(type){
        case TokenType::Dice: return dice;
        case TokenType::Number: return numbers;
        case TokenType::Add: return add;
        case TokenType::Sub: return sub;
        case TokenType::Mult: return mult;
        case TokenType::Div: return div;
        case TokenType::GroupStart: return (void**)&group_start;
        case TokenType::GroupEnd: return (void**)&group_end;
        case TokenType::KeepLow:
        case TokenType::KeepHigh: return keep;
        case TokenType::Unexpected:
        default: return NULL;
    };
}

int DiceCache::IndexOfToken(PairToken *token)
{
    return token - tokens;
}

PairToken* DiceCache::GetPreviousUnclaimed(int pos)
{
    for(int i = pos-1; i >= 0; i--){
        PairToken* token = &tokens[i];
        if(token->claimed){
            continue;
        }
        return token;
        break;
    }
    return NULL;
}

PairToken* DiceCache::GetNextUnclaimed(int pos, int length)
{
    for(int i = pos+1; i < length; i++){
        PairToken* token = &tokens[i];
        if(token->claimed){
            continue;
        }
        return token;
        break;
    }
    return NULL;
}

void DiceCache::SetError(DiceErrorType type, const char *errmsg)
{
    if(error == NULL){
        printf("failed to write to dice error location in memory.\n\tGot Error Type: %d, Got Error:\n\t%s", type, errmsg);
        return;
    }
    error->errmsg = errmsg;
    error->type = type;
}

void DiceCache::CleanTokens()
{
    if(tokens != NULL){
        free(tokens);
        tokens = NULL;
    }
}

void DiceCache::Clean()
{

    if(dice != NULL){
        free(dice);
        dice = NULL;
    }
    
    if(numbers != NULL){
        free(numbers);
        numbers = NULL;
    }
    
    if(keep != NULL){
        free(keep);
        keep = NULL;
    }
    
    if(add != NULL){
        free(add);
        add = NULL;
    }
    
    if(sub != NULL){
        free(sub);
        sub = NULL;
    }
    
    if(mult != NULL){
        free(mult);
        mult = NULL;
    }
    
    if(div != NULL){
        free(div);
        div = NULL;
    }
    
    if(group_start != NULL){
        free(group_start);
        group_start = NULL;
    }
    
    if(group_end != NULL){
        free(group_end);
        group_end = NULL;
    }
    


    CleanTokens();
}
