#include "DiceNotation.h"

static uint8_t DiceNotation_HasInit = 0;

DiceNotation::DiceNotation()
{
    cache = new DiceCache();
    counters = new DiceCounters();
    DiceNotation::Init();

    cache->error = &error;
}

DiceNotation::~DiceNotation()
{
    delete cache;
    delete counters;
}

bool DiceNotation::IsDigit(char c)
{
    return '0' <= c && c <= '9';
}

void DiceNotation::Init()
{
    if(DiceNotation_HasInit == 0){
        srand(time(NULL));
        DiceNotation_HasInit = 1;
    }
}

void DiceNotation::SetText(const char *text)
{
    this->text = text;
    this->length = strlen(text);
}

void DiceNotation::SetError(DiceErrorType type, const char *errmsg)
{
    this->error.errmsg = errmsg;
    this->error.type = type;
}

int DiceNotation::ProcessText()
{

    cache->tokens = (PairToken*)calloc(sizeof(PairToken), this->length);
    if(cache->tokens == NULL){
        SetError(DiceErrorType::FailAlloc, "failed to allocate space for token cache");
        return 1; // failed to allocate
    }

    bool processing_number = false;
    int last_number_pos = -1;

    counters->real_token_count = this->length;
    for(int i = 0; i < this->length; i++){
        char c = this->text[i];
        TokenType type = TokenType::Empty;
        int value = 0;

        if(IsDigit(c)){
            type = TokenType::Number;
            value = c - '0';
        } else {
            switch (c)
            {
                case ' ':
                    type = TokenType::Empty;
                    counters->real_token_count--;
                    break;
                case 'd':
                    counters->dice++;
                    type = TokenType::Dice; break;
                case '+':
                    counters->add++;
                    type = TokenType::Add; break;
                case '-':
                    counters->sub++;
                    type = TokenType::Sub; break;
                case '*':
                    counters->mult++;
                    type = TokenType::Mult; break;
                case '/':
                    counters->div++;
                    type = TokenType::Div; break;
                case '(':
                    counters->group_start++;
                    type = TokenType::GroupStart; break;
                case ')':
                    counters->group_end++;
                    type = TokenType::GroupEnd; break;
                case 'k': {
                    char next_c = text[i+1];
                    switch(next_c)
                    {
                        case 'l': {
                            type = TokenType::KeepLow;
                        }; break;
                        case 'h': {
                            type = TokenType::KeepHigh;
                        }; break;
                        default: {
                            type = TokenType::Keep;
                        }; break;
                    }
                    counters->keep++;
                    i++;
                    counters->real_token_count--;
                }; break;
                default: {
                    type = TokenType::Unexpected;
                }; break;
            }
        }

        if(type == TokenType::Number){
            if(processing_number){
                PairToken* token = &cache->tokens[last_number_pos];
                token->value.i = (token->value.i * 10) + token->value.i;
                type = TokenType::Empty;
                counters->real_token_count--;
            } else {
                processing_number = true;
                last_number_pos = i;
                counters->numbers++;
            }
        } else {
            if(processing_number){
                processing_number = false;
                last_number_pos = -1;
            }
        }

        PairToken* token = &cache->tokens[i];
        token->type = type;
        token->value.i = value;
    }

    return 0;
}

int DiceNotation::ShrinkTokenCache()
{
    PairToken* new_tokens = (PairToken*)calloc(sizeof(PairToken), this->counters->real_token_count);
    if(new_tokens == NULL){
        SetError(DiceErrorType::FailAlloc, "Failed to allocate space for new token cache");
        return 1;
    }
    int new_token_pos = 0;
    
    for(int i = 0; i < this->length; i++){
        PairToken* base_token = &cache->tokens[i];
        if(base_token->type == TokenType::Empty){
            continue;
        }
        base_token->pos = new_token_pos;
        memcpy(&new_tokens[new_token_pos], base_token, sizeof(PairToken));
        new_token_pos++;
    }
    free(cache->tokens);
    cache->tokens = new_tokens;

    return 0;
}

int DiceNotation::DefineGroups()
{
    if(counters->group_start != counters->group_end){
        SetError(DiceErrorType::Logic, "Cannot Define Groups when group start count isn't equal to group end count\n");
        return 1;
    }
    counters->group = counters->group_start;
    cache->group = (Group*)calloc(sizeof(Group), counters->group);
    if(cache->group == NULL){
        SetError(DiceErrorType::FailAlloc, "failed to allocate space for groups\n");
        return 1;
    }

    for(int i = 0; i < counters->group_start; i++){
        Group* group = &cache->group[i];
        GroupPlacement* startPlacement = &cache->group_start[i];
        group->start = startPlacement->token;
        startPlacement->group = group;
        group->group_priority = startPlacement->priority;
        group->end = NULL;
        for(int j = 0; j < counters->group_end; j++){
            GroupPlacement* endPlacement = &cache->group_end[j];
            if(endPlacement->claimed) {
                continue;
            }
            if(endPlacement->priority != startPlacement->priority){
                continue;
            }
            if(endPlacement->token < startPlacement->token){
                continue;
            }
            group->end = endPlacement->token;
            endPlacement->group = group;
            break;
        }
    }


    return 0;
}

int DiceNotation::AssertGroupDominance()
{
    for(int i = 0; i < counters->group; i++){
        Group* group = &cache->group[i];
        PairToken* startToken = group->start;
        PairToken* endToken = group->end;

        int start_pos = cache->IndexOfToken(startToken) + 1;
        int end_pos = cache->IndexOfToken(endToken);

        int size = end_pos - start_pos;

        printf("[%p] Group(size: %d)\n", group, size);

        group->tokens.count = size;
        group->tokens.values = (PairToken*)calloc(sizeof(PairToken), size);

        int group_pos = 0;
        for(int i = start_pos; i < end_pos; i++){
            PairToken* token = &cache->tokens[i];
            PairToken* groupToken = &group->tokens.values[group_pos];
            memcpy(groupToken, token, sizeof(PairToken));
            groupToken->pos = group_pos;
            token->type = TokenType::Empty;
            counters->real_token_count--;
            group_pos++;
        }

        endToken->type = TokenType::Empty;
        counters->real_token_count--;

        startToken->type = TokenType::TypeGroup;
        startToken->value.ptr = group;
    }

    return 0;
}

int DiceNotation::DefineIndexCaches()
{
    int dice = 0;
    int keep = 0;
    int numbers = 0;
    int add = 0;
    int sub = 0;
    int mult = 0;
    int div = 0;
    int group_start = 0;
    int group_end = 0;

    int group_priority = 0;

    for(int i = 0; i < counters->real_token_count; i++){
        PairToken* token = &cache->tokens[i];
        GroupPlacement* placement;
        switch(token->type){
            case TokenType::Dice:
                cache->dice[dice] = token;
                dice++;
                break;
            case TokenType::Number:
                cache->numbers[numbers] = token;
                numbers++;
                break;
            case TokenType::Add:
                cache->add[add] = token;
                add++;
                break;
            case TokenType::Sub:
                cache->sub[sub] = token;
                sub++;
                break;
            case TokenType::Mult:
                cache->mult[mult] = token;
                mult++;
                break;
            case TokenType::Div:
                cache->div[div] = token;
                div++;
                break;
            case TokenType::GroupStart:
                group_priority++;
                placement = &cache->group_start[group_start];
                placement->token = token;
                placement->priority = group_priority;
                group_start++;
                break;
            case TokenType::GroupEnd:
                placement = &cache->group_end[group_end];
                placement->token = token;
                placement->priority = group_priority;
                group_priority--;
                group_end++;
                break;
            case TokenType::KeepHigh:
            case TokenType::KeepLow:
                cache->keep[keep] = token;
                keep++;
                break;
            case TokenType::Empty: break;
            default: {
                printf("unhandled token type when defining index caches: %s\n", DiceUtil::TokenTypeToString(token->type));
            }; break;
        }
    }


    return 0;
}

int DiceNotation::DefineRanges()
{
    counters->ranges = CalcRangeCount();
    cache->ranges = (RangeToken*)calloc(sizeof(RangeToken), counters->ranges);

    counters->range_pos = 0;

    DefineNumberRanges();
    DefineKeepRanges();
    DefineDiceRanges();
    DefineAddRanges();
    // DefineSubRanges();
    // DefineMultRanges();
    // DefineDivRanges();

    return 0;
}

int DiceNotation::Process()
{
    int retvalue = 0;

    retvalue = ProcessText();
    if(retvalue != 0){
        printf("failed to process text got error:\n\t%s", error.errmsg);
        return 1;
    }

    retvalue = ShrinkTokenCache();
    if(retvalue != 0){
        printf("failed to shrink token cache, got error:\n\t%s", error.errmsg);
        return 1;
    }

    retvalue = cache->AllocateIndexes(counters);
    if(retvalue != 0){
        printf("failed to allocate space for cache indexes, got error:\n\t%s", error.errmsg);
        return 1;
    }

    retvalue = DefineIndexCaches();
    if(retvalue != 0){
        printf("failed to define index caches, got error:\n\t%s", error.errmsg);
        return 1;
    }

    retvalue = DefineGroups();
    if(retvalue != 0){
        printf("failed to define groups, got error:\n\t%s", error.errmsg);
        return 1;
    }

    retvalue = SortGroups();
    if(retvalue != 0){
        printf("failed to organize groups, got error:\n\t%s", error.errmsg);
        return 1;
    }

    retvalue = AssertGroupDominance();
    if(retvalue != 0){
        printf("failed to assert group dominance, got error:\n\t%s", error.errmsg);
        return 1;
    }

    retvalue = ShrinkTokenCache();
    if(retvalue != 0){
        printf("failed to shrink token cache after assigning group dominance, got error:\n\t%s", error.errmsg);
        return 1;
    }

    // retvalue = DefineRanges();
    // if(retvalue != 0){
    //     printf("failed to define ranges, got error:\n\t%s", error.errmsg);
    //     return 1;
    // }
    // printf("done defining ranges\n");

    DebugPrint();

    return 0;
}

DiceResult *DiceNotation::Run()
{
    RangeToken* token = DetermineStartingRange();

    if(token == NULL){
        printf("failed to find starting range token\n");
        return NULL;
    }

    DiceResult* result = RunRange(token);
    if(result == NULL){
        printf("failed to produce a result when running range on starting range token\n");
        return NULL;
    }

    return result;
}

// RangeToken *DiceNotation::DefineRangesForTokenList(DiceError* error, PairToken *tokens, int size)
// {
//     RangeToken* ranges = (RangeToken*)calloc(sizeof(RangeToken), size);
//     for(int i = 0; i < size; i++){

//     }

//     return NULL;
// }

DiceResult *DiceNotation::RunRange(RangeToken *token)
{
    switch(token->type){
        case RangeNumber:
            return RunNumber(token);
        case RangeAdd:
            return RunAdd(token);
        case RangeDice:
            return RunDice(token);
        default: {
            printf("RunRange, Unhandled RangeType: %s\n", DiceUtil::RangeTokenTypeToString(token->type));
        }; break;
    }

    return NULL;
}

DiceResult *DiceNotation::RunNumber(RangeToken *token)
{
    DiceResult* result = DiceResult_Create();
    if(result == NULL){
        return NULL;
    }
    result->type = DiceNumber;
    result->value = token->main->value.i;
    return result;
}

DiceResult *DiceNotation::RunDice(RangeToken *token)
{
    DiceResult* result = DiceResult_Create();
    if(result == NULL){
        return NULL;
    }
    DiceResult* before = RunRange((RangeToken*)token->start->range_pos);
    DiceResult* after = RunRange((RangeToken*)token->end->range_pos);

    int amount = before->value;
    int sides = after->value;

    free(before);
    before = NULL;

    free(after);
    after = NULL;

    int* values = (int*)calloc(sizeof(int), amount);
    for(int i = 0; i < amount; i++){
        values[i] = (rand() % sides) + 1;
    }

    long out_value = 0;

    for(int i = 0; i < amount; i++){
        out_value = out_value + values[i];
    }

    
    result->type = DiceResultType::DiceNumber;
    result->value = out_value;

    return result;
}

DiceResult *DiceNotation::RunAdd(RangeToken *token)
{
    DiceResult* result = DiceResult_Create();
    if(result == NULL){
        return NULL;
    }
    DiceResult* result1 = RunRange((RangeToken*)token->start->range_pos);
    DiceResult* result2 = RunRange((RangeToken*)token->end->range_pos);
    result->type = DiceResultType::DiceNumber;
    result->value = result1->value + result2->value;
    free(result1);
    free(result2);
    return result;
}

RangeToken *DiceNotation::DetermineStartingRange()
{
    for(int i = 0; i < counters->ranges; i++){
        RangeToken* range = &cache->ranges[i];
        if(!range->main->claimed){
            return range;
        }
    }
    return NULL;
}

int DiceNotation::DefineNumberRanges()
{
    printf("defining number ranges\n");
    for(int i = 0; i < counters->numbers; i++){
        PairToken* token = (PairToken*)cache->numbers[i];
        RangeToken* range = &cache->ranges[counters->range_pos];
        range->start = token;
        range->main = token;
        range->end = token;
        range->type = RangeNumber;
        token->range_pos = range;
        token->claimed = 0;
        counters->range_pos++;
    }
    return 0;
}

int DiceNotation::DefineKeepRanges()
{
    printf("defining keep ranges\n");
    for(int i = 0; i < counters->keep; i++){
        PairToken* token = (PairToken*)cache->keep[i];
        RangeToken* range = &cache->ranges[counters->range_pos];
        
        PairToken* next_token = &cache->tokens[token->pos+1];
        range->start = token;
        range->main = next_token;
        range->end = next_token;
        token->range_pos = range;
        range->type = RangeKeep;
        token->claimed = 1;
        counters->range_pos++;
    }
    return 0;
}

int DiceNotation::DefineDiceRanges()
{
    printf("defining dice ranges\n");
    for(int i = 0; i < counters->dice; i++){
        PairToken* token = (PairToken*)cache->dice[i];
        RangeToken* range = &cache->ranges[counters->range_pos];

        range->main = token;

        PairToken* prev = &cache->tokens[token->pos-1];
        PairToken* next = &cache->tokens[token->pos+1];

        range->start = prev;
        range->end = next;

        prev->claimed = 1;
        next->claimed = 1;

        range->type = RangeDice;
        token->range_pos = range;
        counters->range_pos++;
    }

    return 0;
}

int DiceNotation::DefineMathRanges()
{
    return 0;
}

int DiceNotation::DefineAddRanges()
{
    printf("defining add ranges\n");
    for(int i = 0; i < counters->add; i++){
        PairToken* token = (PairToken*)cache->add[i];
        RangeToken* range = &cache->ranges[counters->range_pos];

        int pos = token->pos;

        range->start = token;
        for(int j = pos-1; j > -1; j--){
            PairToken* beforeToken = &cache->tokens[j];
            if(!beforeToken->claimed){
                range->start = beforeToken;
                beforeToken->claimed = 1;
                break;
            }
        }

        range->end = token;
        for(int j = pos+1; j < counters->real_token_count; j++){
            PairToken* afterToken = &cache->tokens[j];
            if(!afterToken->claimed){
                range->end = afterToken;
                afterToken->claimed = 1;
                break;
            }
        }

        range->main = token;
        range->type = RangeAdd;

        counters->range_pos++;
    }
    return 0;
}

int DiceNotation::DefineSubRanges()
{
    for(int i = 0; i < counters->sub; i++){
        PairToken* token = (PairToken*)cache->sub[i];
        RangeToken* range = &cache->ranges[counters->range_pos];

        int pos = token->pos;

        range->start = token;
        for(int j = pos-1; j > -1; j--){
            PairToken* beforeToken = &cache->tokens[j];
            if(!beforeToken->claimed){
                range->start = beforeToken;
                break;
            }
        }

        range->end = token;
        for(int j = pos+1; j < counters->real_token_count; j++){
            PairToken* afterToken = &cache->tokens[j];
            if(!afterToken->claimed){
                range->end = afterToken;
                break;
            }
        }
        range->main = token;
        range->type = RangeSub;

        counters->range_pos++;
    }
    return 0;
}

int DiceNotation::DefineMultRanges()
{
    for(int i = 0; i < counters->sub; i++){
        PairToken* token = (PairToken*)cache->sub[i];
        RangeToken* range = &cache->ranges[counters->range_pos];

        int pos = token->pos;

        range->start = token;
        for(int j = pos-1; j > -1; j--){
            PairToken* beforeToken = &cache->tokens[j];
            if(!beforeToken->claimed){
                range->start = beforeToken; 
                break;
            }
        }

        range->end = token;
        for(int j = pos+1; j < counters->real_token_count; j++){
            PairToken* afterToken = &cache->tokens[j];
            if(!afterToken->claimed){
                range->end = afterToken;
                break;
            }
        }
        range->main = token;
        range->type = RangeMult;

        counters->range_pos++;
    }
    return 0;
}

int DiceNotation::DefineDivRanges()
{
    for(int i = 0; i < counters->sub; i++){
        PairToken* token = (PairToken*)cache->sub[i];
        RangeToken* range = &cache->ranges[counters->range_pos];

        int pos = token->pos;

        range->start = token;
        for(int j = pos-1; j > -1; j--){
            PairToken* beforeToken = &cache->tokens[j];
            if(!beforeToken->claimed){
                range->start = beforeToken; 
                break;
            }
        }

        range->end = token;
        for(int j = pos+1; j < counters->real_token_count; j++){
            PairToken* afterToken = &cache->tokens[j];
            if(!afterToken->claimed){
                range->end = afterToken;
                break;
            }
        }
        range->main = token;
        range->type = RangeDiv;

        counters->range_pos++;
    }
    return 0;
}

int DiceNotation::SortGroups()
{
    for(int i = 0; i < counters->group; i++){
        Group* group_i = &cache->group[i];
        for(int j = i+1; j < counters->group; j++){
            Group* group_j = &cache->group[j];
            uint8_t swap = 0;
            if(group_i->group_priority < group_j->group_priority){
                swap = 1;
            } else if(group_i->group_priority == group_j->group_priority){
                if(group_i->start->pos < group_j->start->pos){
                    swap = 1;
                }
            }

            if(swap == 1){
                Group temp;
                memcpy(&temp, group_i, sizeof(Group));
                memcpy(group_i, group_j, sizeof(Group));
                memcpy(group_j, &temp, sizeof(Group));
            }
        }
    }
    return 0;
}

int DiceNotation::DefineGroupRanges()
{
    for(int i = 0; i < counters->group; i++){
        Group* group = &cache->group[i];
        RangeToken* range = &cache->ranges[counters->range_pos];

        range->start = group->start;
        range->end = group->end;

        counters->range_pos++;
    }
    return 0;
}

int DiceNotation::CalcRangeCount()
{
    return counters->dice + counters->keep + counters->numbers + counters->add + counters->sub + counters->mult + counters->div + counters->group;
}

void DiceNotation::DebugPrintTokens()
{
    printf("real_token_count: %d\nTokens: [", counters->real_token_count);
    for(int i = 0; i < counters->real_token_count; i++){
        PairToken* token = &cache->tokens[i];
        printf("\n\t");
        DiceUtil::PrintPairToken(token);
        if(i+1 < counters->real_token_count){
            printf(",");
        }
    }
    printf("\n];\n");
}

void DiceNotation::DebugPrintRangeToken(RangeToken *token)
{
    int size = DiceUtil::RangeTokenSize(token);
    printf("[%p] RangeToken(start: %p, end: %p, main: %p, type: %s, size: %d,\n\ttokens: [", token, token->start, token->end, token->main, DiceUtil::RangeTokenTypeToString(token->type), size);
    for(int i = 0; i < size; i++){
        PairToken* pairToken = &cache->tokens[token->start->pos+i];
        if(pairToken == NULL){
            printf("\n\t\tNULL");
        } else {
            printf("\n\t\t");
            DiceUtil::PrintPairToken(pairToken);
        }
        if(i+1 < size){
            printf(",");
        } else {
            printf("\n\t");
        }
    }
    printf("])");
}

void DiceNotation::DebugPrintData(const char *name, void **data, int size)
{
    printf("%s: [", name);
    if(size == 0){
        printf("];\n");
        return;
    }
    for(int i = 0; i < size; i++){
        void* value = data[i];
        if(value == NULL){
            printf("\n\tNULL");
        } else {
            printf("\n\t[%p]", value);
        }
        if(i+1 < size){
            printf(",");
        }
    }
    printf("\n];\n");
}

void DiceNotation::DebugPrintRanges()
{
    printf("Ranges: [");
    if(counters->ranges == 0){
        printf("];\n");
    } else {
        for(int i = 0; i < counters->ranges; i++){
            RangeToken* token = &cache->ranges[i];
            printf("\n\t");
            DebugPrintRangeToken(token);
            if(i+1 < counters->ranges){
                printf(",");
            }
        }
        printf("\n];\n");
    }
}

void DiceNotation::DebugPrintGroupPlacements(const char* name, GroupPlacement *placements, int size)
{
    printf("%s: [", name);
    for(int i = 0; i < size; i++){
        GroupPlacement* placement = &placements[i];
        if(placement == NULL){
            printf("\n\tNULL");
        } else {
            printf("\n\t");
            DiceUtil::PrintGroupPlacement(placement);
        }
        if(i+1 < size){
            printf(",");
        } else {
            printf("\n");
        }
    }
    printf("]\n");
}

void DiceNotation::DebugPrintGroup(Group *group)
{
    printf("[%p] Group(start: %p, end: %p, main: %p, priority: %d,\n", group, group->start, group->end, group->main, group->group_priority);

    printf("\tGroupTokens: [");
    for(int i = 0; i < group->tokens.count; i++){
        PairToken* token = &group->tokens.values[i];
        if(token == NULL){
            printf("\n\t\tNULL");
        } else {
            printf("\n\t\t");
            DiceUtil::PrintPairToken(token);
        }
        if(i + 1 < group->tokens.count){
            printf(",");
        }
    }
    printf("\n\t\t]\n");
    printf("\t)");
}

void DiceNotation::DebugPrintGroups()
{
    printf("Groups: [");
    for(int i = 0; i < counters->group; i++){
        Group* group = &cache->group[i];
        if(group == NULL){
            printf("\n\tNULL");
        } else {
            printf("\n\t");
            DebugPrintGroup(group);
        }
        if(i+1 < counters->group){
            printf(",");
        } else {
            printf("\n");
        }
    }
    printf("]\n");
}

void DiceNotation::DebugPrint()
{
    printf("DiceNotation:");
    
    DebugPrintData("Dice", cache->dice, counters->dice);
    DebugPrintData("Numbers", cache->numbers, counters->numbers);
    DebugPrintData("Keep", cache->keep, counters->keep);
    DebugPrintData("Add", cache->add, counters->add);
    DebugPrintData("Sub", cache->sub, counters->sub);
    DebugPrintData("Mult", cache->mult, counters->mult);
    DebugPrintData("Div", cache->div, counters->div);

    DebugPrintGroupPlacements("GroupStart", cache->group_start, counters->group_start);
    DebugPrintGroupPlacements("GroupEnd", cache->group_end, counters->group_end);    

    DebugPrintGroups();

    DebugPrintTokens();

    DebugPrintRanges();


    printf("---\n");
}
