#include "dice.h"

// local type definition
void dice_notation_set_error(DiceNotation* notation, int errcode, const char* errmsg);
void dice_notation_debug();


// definitions
void dice_notation_set_error(DiceNotation* notation, int errcode, const char* errmsg){
    notation->errcode = errcode;
    notation->errmsg = errmsg;
}

void dice_notation_init()
{
    srand(time(NULL));
}

void dice_notation_debug()
{
    printf("# Sizes\n\tDiceNotation: %lu\n\tDiceNotationState: %lu\n\tDiceNotationCache: %lu\n\tDiceNotationCounters: %lu\n", sizeof(DiceNotation), sizeof(DiceState), sizeof(DiceNotationCache), sizeof(DiceCounters));
}

RangeCache* RangeCache_Create();
int RangeCache_Alloc(RangeCache* cache, uint8_t size);
int RangeCache_Add(RangeCache* cache, RangeToken* token);
void RangeCache_Destroy(RangeCache* cache);

long dice_roll(DiceToken* dice_token){
    unsigned int amount = 0;
    unsigned int sides = 0;

    unsigned int keep_high = 0;
    unsigned int keep_low = 0;
    
    if(dice_token->amount != NULL){
        amount = dice_token->amount->value;
    } else {
        amount = 1;
    }
    if(dice_token->sides != NULL){
        sides = dice_token->sides->value;
    } else {
        printf("failed to get information for dice sides\n");
        sides = 4;
    }

    if(amount < 1) {
        printf("you can't roll <1 dice\n");
        amount = 1;
    }

    uint8_t keep = 0;

    if(dice_token->keep_high != NULL || dice_token->keep_low != NULL){
        keep = 1;
    }

    if(keep == 1){
        if(dice_token->keep_high != NULL){
            RealToken* keep_high_token = dice_token->keep_high->value;
            if(keep_high_token->type == TYPE_NUM){
                NumberToken* number_token = keep_high_token->special;
                if(number_token == NULL){
                    printf("got null reference when trying to get a number token during dice rolls\n");
                    keep_high = 0;
                } else {
                    keep_high = number_token->value;
                }
            } else {
                printf("unhandled type in dice rolling\n");
            }
            
        } else {
            keep_high = 0;
        }
        if(dice_token->keep_low != NULL){
            RealToken* keep_low_token = dice_token->keep_low->value;
            if(keep_low_token->type == TYPE_NUM){
                NumberToken* number_token = keep_low_token->special;
                if(number_token == NULL){
                    printf("got null reference when trying to get a number token during dice rolls\n");
                    keep_low = 0;
                } else {
                    keep_low = number_token->value;
                }
            } else {
                printf("unhandled type in dice rolling\n");
            }
        } else {
            keep_low = 0;
        }
    }

    long result = 0;

    int* values = calloc(sizeof(int), amount);
    if(values == NULL){
        printf("failed to allocate memory for amount of dice specified: %d\n", amount);
        return 0;
    }

    for(unsigned int i = 0; i < amount; i++){
        values[i] = (int)((rand() % sides) + 1);
    }

    if(keep == 1){
        for(unsigned int i = 0; i < amount; i++){
            for(unsigned int j = i++; j < amount; j++){
                if(values[i] > values[j]){
                    int temp_value = values[i];
                    values[j] = values[i];
                    values[i] = temp_value;
                }
            }
        }

        // to-do: make sure we don't need to flip the logic in the keep loops.
        for(unsigned int i = 0; i < keep_high; i++){
            int temp = values[amount - i];
            // we'll check this later, but put a print statement here to remind ourselves later
            printf("[DEBUG] (confirm that keep_high is working) rolling dice, got temp value: %d\n", temp);
            result = result + temp;
        }

        for(unsigned int i = 0; i < keep_low; i++){
            result = result + values[i];
        }

    } else {
        for(unsigned int i = 0; i < amount; i++){
            result = result + values[i];
        }
    }

    free(values);
    values = NULL;

    return result;
}


// token setters
void SetPairToken(PairToken* token, enum TokenType type, int value){
    token->type = type;
    token->value = value;
}

void SetRealToken(RealToken* token, uint64_t pos, enum TokenType type, enum TokenCategory category, void* ref){
    token->pos = pos;
    token->type = type;
    token->special = ref;
    token->category = category;
    token->used = 0;
}

const char* TokenType_as_string(enum TokenType type){
    switch(type){
        case TYPE_SPACE: return "SPACE";
        case TYPE_DICE: return "DICE";
        case TYPE_MATH: return "MATH";
        case TYPE_ADD: return "ADD";
        case TYPE_SUB: return "SUB";
        case TYPE_DIV: return "DIV";
        case TYPE_MULT: return "MULT";
        case TYPE_NUM: return "NUM";
        case TYPE_EXP: return "EXP";
        case TYPE_GROUP_START: return "GROUP_START";
        case TYPE_GROUP_END: return "GROUP_END";
        case TYPE_KEEP: return "KEEP";
        case TYPE_KEEP_HIGH: return "KEEP_HIGH";
        case TYPE_KEEP_LOW: return "KEEP_LOW";
        case TYPE_EMPTY: return "EMPTY";
        default: return "UknownType";
    }
}

void PairToken_Print(PairToken* token){
    printf("PairToken(type: %s, %d)\n", TokenType_as_string(token->type), token->value);
}

void PairTokens_PrintList(PairToken* tokens, uint64_t length){
    for(uint64_t i = 0; i < length; i++){
        PairToken_Print(&tokens[i]);
    }
}

// helpers
uint8_t MathToken_GetPriority(enum TokenType type){
    switch(type){
        case TYPE_ADD: return 1;
        case TYPE_SUB: return 2;
        case TYPE_DIV: return 3;
        case TYPE_MULT: return 4;
        case TYPE_EXP: return 5;
        default: return 10;
    }
}



// main operations
int dice_notation_parse_text(DiceNotation* notation){
    DiceState* state = &notation->state;
    DiceNotationCache* cache = &notation->cache;
    DiceCounters* counters = &notation->counters;

    cache->tokens = calloc(sizeof(PairToken), state->length);

    if(cache->tokens == NULL){
        dice_notation_set_error(notation, DN_ERROR_FAILED_ALLOC_CACHE, "failed to allocate space in cache for tokens");
        return -1;
    }

    counters->real_token_count = state->length;

    signed int last_number_index = -1;
    uint8_t processing_number = 0;
    
    for(uint64_t i = 0; i < state->length; i++){
        char c = state->text[i];
        uint8_t handling_number = 0;
        PairToken* token = &cache->tokens[i];
        int value = 0;

        switch(c){
            case ' ':
                SetPairToken(token, TYPE_EMPTY, 0);
                counters->real_token_count--;
                break;
            case 'd':
                SetPairToken(token, TYPE_DICE, 0);
                counters->dice_count++;
                break;
            case 'k':
                SetPairToken(&cache->tokens[i+1], TYPE_EMPTY, 0);
                counters->real_token_count--;
                counters->keep_count++;
                switch(state->text[i+1]) {
                    case 'h':
                        SetPairToken(token, TYPE_KEEP, TYPE_KEEP_HIGH);
                        i++;
                        break;
                    case 'l':
                        SetPairToken(token, TYPE_KEEP, TYPE_KEEP_LOW);
                        i++;
                        break;
                    default:
                        SetPairToken(token, TYPE_KEEP, 0);
                        break;
                }
                break;
            case '(':
                SetPairToken(token, TYPE_GROUP_START, 0);
                counters->group_start_count++;
                break;
            case ')':
                SetPairToken(token, TYPE_GROUP_END, 0);
                counters->group_end_count++;
                break;
            // math token character received.
            case '+':
                SetPairToken(token, TYPE_MATH, TYPE_ADD);
                counters->math_count++;
                break;
            case '-': {
                SetPairToken(token, TYPE_MATH, TYPE_SUB);
                counters->math_count++;
            }; break;
            case '/':
                SetPairToken(token, TYPE_MATH, TYPE_DIV);
                counters->math_count++;
                break;
            case '*':
                SetPairToken(token, TYPE_MATH, TYPE_MULT);
                counters->math_count++;
                break;
            // keep high & keep low character identification.
            case 'h':
            case 'l':
                printf("for some reason you processed an h or an l\n");
                break;
            default: {
                if ('0' <= c <= '9'){
                    handling_number = 1;
                    value = (c - '0');
                } else {
                    printf("unknown token received: %c\n", c);
                }
            }; break;
        }

        

        if(handling_number == 1){
            if(processing_number == 1) {
                cache->tokens[last_number_index].value = (cache->tokens[last_number_index].value * 10) + value;
                SetPairToken(token, TYPE_EMPTY, 0);
                counters->real_token_count--;
            } else {
                last_number_index = i;
                processing_number = 1;
                counters->number_count++;
                SetPairToken(token, TYPE_NUM, value);
            }
        } else {
            if(processing_number == 1){
                processing_number = 0;
                last_number_index = -1;
            }
        }
    }

    return 0;
}

int dice_notation_allocate_space_for_cache(DiceNotation* notation){
    DiceNotationCache* cache = &notation->cache;
    DiceCounters* counters = &notation->counters;

    cache->dice = calloc(sizeof(DiceToken), counters->dice_count);
    if(cache->dice == NULL){
        dice_notation_set_error(notation, DN_ERROR_GOT_NULL, "failed to allocate space for dice cache");
        return 1;
    }
    cache->math = calloc(sizeof(MathToken), counters->math_count);
    if(cache->math == NULL){
        dice_notation_set_error(notation, DN_ERROR_GOT_NULL, "failed to allocate space for math cache");
        return 1;
    }
    cache->keep = calloc(sizeof(KeepToken), counters->keep_count);
    if(cache->keep == NULL){
        dice_notation_set_error(notation, DN_ERROR_GOT_NULL, "failed to allocate space for keep cache");
        return 1;
    }
    cache->number = calloc(sizeof(NumberToken), counters->number_count);
    if(cache->number == NULL){
        dice_notation_set_error(notation, DN_ERROR_GOT_NULL, "failed to allocate space for number cache");
        return 1;
    }

    cache->start_tokens = calloc(sizeof(GroupPlacementToken), counters->group_start_count);
    if(cache->start_tokens == NULL){
        dice_notation_set_error(notation, DN_ERROR_GOT_NULL, "failed to allocate space for start_tokens cache");
        return 1;
    }
    cache->end_tokens = calloc(sizeof(GroupPlacementToken), counters->group_end_count);
    if(cache->end_tokens == NULL){
        dice_notation_set_error(notation, DN_ERROR_GOT_NULL, "failed to allocate space for end_tokens cache");
        return 1;
    }

    cache->real_tokens = calloc(sizeof(RealToken), counters->real_token_count);
    if(cache->real_tokens == NULL){
        dice_notation_set_error(notation, DN_ERROR_GOT_NULL, "failed to allocate space for real_tokens cache");
        return 1;
    }

    return 0;
}

int dice_notation_organize_tokens(DiceNotation* notation){
    DiceNotationCache* cache = &notation->cache;
    DiceCounters* counters = &notation->counters;

    counters->group_priority = 0;

    counters->real_token_pos = 0;
    for(uint64_t i = 0; i < notation->state.length; i++){
        PairToken* token = &cache->tokens[i];
        if(token == NULL){
            dice_notation_set_error(notation, DN_ERROR_GOT_NULL, "failed to get token from cache, received null value");
            return -1;
        }
        // skip these ones
        if(token->type == TYPE_EMPTY){
            continue;
        }

        RealToken* real_token = &cache->real_tokens[counters->real_token_pos];

        switch(token->type){
            case TYPE_EMPTY:
                continue;
                break;
            case TYPE_DICE: {
                DiceToken* dice_token = &cache->dice[counters->dice_pos];
                dice_token->location = real_token;
                SetRealToken(real_token, counters->real_token_pos, token->type, TOKEN_DICE, dice_token);
                counters->dice_pos++;
            }; break;
            case TYPE_NUM: {
                NumberToken* number_token = &cache->number[counters->number_pos];
                number_token->location = real_token;
                SetRealToken(real_token, counters->real_token_pos, token->type, TOKEN_NUMBER, number_token);
                number_token->value = token->value;
                counters->number_pos++;
            }; break;
            case TYPE_MATH: {
                MathToken* math_token = &cache->math[counters->math_pos];
                math_token->location = real_token;
                SetRealToken(real_token, counters->real_token_pos, token->type, TOKEN_MATH, math_token);
                math_token->type = token->value;
                math_token->group_priority = counters->group_priority;
                counters->math_pos++;
            }; break;
            case TYPE_KEEP: {
                KeepToken* keep_token = &cache->keep[counters->keep_pos];
                keep_token->location = real_token;
                SetRealToken(real_token, counters->real_token_pos, token->type, TOKEN_KEEP, keep_token);
                keep_token->type = token->value;
                keep_token->value = NULL;
                counters->keep_pos++;
            }; break;
            case TYPE_GROUP_START: {
                GroupPlacementToken* start_token = &cache->start_tokens[counters->group_start_pos];
                start_token->location = real_token;
                SetRealToken(real_token, counters->real_token_pos, token->type, TOKEN_GROUP_PLACEMENT, start_token);
                start_token->used = 0;
                counters->group_priority++;
                start_token->priority = counters->group_priority;
                counters->group_start_pos++;
            }; break;
            case TYPE_GROUP_END: {
                GroupPlacementToken* end_token = &cache->end_tokens[counters->group_end_pos];
                end_token->location = real_token;
                SetRealToken(real_token, counters->real_token_pos, token->type, TOKEN_GROUP_PLACEMENT, end_token);
                end_token->priority = counters->group_priority;
                counters->group_priority--;
                end_token->used = 0;
                counters->group_end_pos++;
            }; break;
            default: {
                printf("unexpected token with type: %s\n", TokenType_as_string(token->type));
            }; break;
        }
        counters->real_token_pos++;
    }

    return 0;
}

int dice_notation_define_dice(DiceNotation* notation){
    DiceNotationCache* cache = &notation->cache;
    DiceCounters* counters = &notation->counters;

    for(uint8_t j = 0; j < counters->dice_count; j++){
        printf("handling dice at index: %d\n", j);
        DiceToken* dice_token = &cache->dice[j];
        if(dice_token == NULL){
            dice_notation_set_error(notation, DN_ERROR_GOT_NULL, "got null when trying to get a dice token");
            return -1;
        }

        RealToken* real_token = dice_token->location;
        if(real_token == NULL){
            dice_notation_set_error(notation, DN_ERROR_GOT_NULL, "got null when trying to grab real_token reference from dice_token");
            return -1;
        }


        uint64_t pos = dice_token->location->pos;

        uint64_t back_steps = 0;
        uint64_t forward_steps = 0;

        for(uint64_t i = pos-1; 0 <= i; i--){
            RealToken* temp_token = &cache->real_tokens[i];
            if(temp_token == NULL){
                // to-do: correctly handle the null value;
                continue;
            }
            if(temp_token->used == 1){
                continue;
            }
            enum TokenType cur_type = temp_token->type;
            printf("scanning back type: %s\n", TokenType_as_string(cur_type));
            if(cur_type != TYPE_NUM && cur_type != TYPE_KEEP){
                break;
            }
            back_steps++;
        }

        for(uint64_t i = pos+1; i < counters->real_token_count; i++){
            RealToken* temp_token = &cache->real_tokens[i];
            if(temp_token == NULL){
                // to-do: correctly handle the null value;
                continue;
            }
            enum TokenType cur_type = temp_token->type;
            printf("scanning forward type: %s\n", TokenType_as_string(cur_type));
            if(cur_type != TYPE_NUM && cur_type != TYPE_KEEP){
                break;
            }
            forward_steps++;
        }
        
        printf("got steps\n");

        // this will help determine how many steps are needed to process dice.
        uint64_t length = back_steps + forward_steps + 1;
        printf("length: %ld\n", length);

        // printf("")

        uint64_t start_pos = pos - back_steps;
        printf("start pos: %lu\n", start_pos);
        for(uint64_t range_i = start_pos; range_i < (start_pos + length); range_i++){
            printf("working through range, currently at index: %ld\n", range_i);
            if(range_i == pos){
                continue;
            }
            RealToken* cur_token = &cache->real_tokens[range_i];
            if(cur_token == NULL){
                dice_notation_set_error(notation, DN_ERROR_GOT_NULL, "failed to get cur_token during indexing");
                return -1;
            }

            if(cur_token->used == 1){
                continue;
            }

            switch(cur_token->type){
                case TYPE_NUM: {
                    // to-do: add a check for the appropriate type that we're assigning here.
                    NumberToken* number_token = cur_token->special;
                    printf("attempting to assign number to dice with location: %p\n", number_token);
                    if(range_i < pos){
                        dice_token->amount = number_token;
                    } else {
                        dice_token->sides = number_token;
                    }
                    cur_token->used = 1;
                }; break;
                case TYPE_KEEP: {
                    KeepToken* keep_token = cur_token->special;
                    switch(keep_token->type){
                        case TYPE_KEEP_HIGH: {
                            dice_token->keep_high = keep_token;
                            keep_token->used = 1;
                        }; break;
                        case TYPE_KEEP_LOW: {
                            dice_token->keep_low = keep_token;
                            keep_token->used = 1;
                        }; break;
                        default: {
                            printf("got unknown keep operation\n");
                        }; break;
                    }
                }; break;
                default: {
                    printf("got unknown token when trying to parse dice: %s\n", TokenType_as_string(cur_token->type));
                }; break;
            }
        }


    }

    
    return 0;
}

// this should be called after groups are defined.
int dice_notation_define_math(DiceNotation* notation){
    DiceNotationCache* cache = &notation->cache;
    DiceCounters* counters = &notation->counters;

    uint8_t check_groups = 0;

    // if there's more than the default group count then we need to assign the appropriate group priority
    if(counters->group_count > 1){
        printf("checking groups\n");
        check_groups = 1;
    }

    for(uint8_t i = 0; i < counters->math_count; i++){
        MathToken* math_token = &cache->math[i];
        RealToken* real_token = math_token->location;

        // what we're doing here is we're assigning how much we should care
        switch(math_token->type){
            case TYPE_ADD:
            case TYPE_SUB:
                math_token->priority = 4;
                break;
            case TYPE_MULT:
            case TYPE_DIV:
                math_token->priority = 3;
                break;
            case TYPE_EXP:
                math_token->priority = 2;
                break;
            default: {
                math_token->priority = 10;
                printf("unknown math operator: %s\n", TokenType_as_string(math_token->type));
            }; break;
        }

        if(check_groups == 0){
            // in this case, assign them all to the default group
            math_token->group = &cache->group[0];
            continue;
        }

        uint64_t pos = math_token->location->pos;

        // this is assuming the groups have been sorted correctly. (highest priority to lowest priority)
        for(uint8_t group_pos = 0; group_pos < counters->group_count; group_pos++){
            GroupToken* group_token = &cache->group[group_pos];
            if(group_token == NULL){
                dice_notation_set_error(notation, DN_ERROR_GOT_NULL, "got null when trying to get a group token, this is required to figure out what order we do math in");
                return 1;
            }
            if(group_token->priority != math_token->group_priority){
                continue; // these should match
            }

            if(group_token->start_pos->pos <= pos <= group_token->end_pos->pos){
                math_token->group = group_token;
                break;
            }
        }
    }

    // now that we've defined some basic priority the fun part is defining what order the math tokens should be in

    for(uint8_t i = 0; i < counters->math_count; i++){
        MathToken* math_i = &cache->math[i];
        for(uint8_t j = i+1; j < counters->math_count; j++){
            MathToken* math_j = &cache->math[j];
            uint8_t swap = 0;
            if(math_i->group_priority < math_j->group_priority){
                swap = 1;
            } else if (math_i->group_priority == math_j->group_priority){
                // check if they have equal pemdas priority
                if(math_i->priority == math_j->priority){
                    // sort for position
                    if(math_i->location->pos > math_j->location->pos){
                        swap = 1;
                    }
                } else if(math_i->priority < math_j->priority){
                    swap = 1;
                }
            }

            if(swap == 1){
                MathToken temp;
                memcpy(&temp, &cache->math[i], sizeof(MathToken));
                memcpy(&cache->math[i], &cache->math[j], sizeof(MathToken));
                memcpy(&cache->math[j], &temp, sizeof(MathToken));
            }
        }
    }

    // now that we've changed the order of the math tokens, lets update the real token references so they can back refrence the math tokens
    for(uint8_t i = 0; i < counters->math_count; i++){
        MathToken* math_token = &cache->math[i];
        // very weird, but very much needed.
        math_token->location->special = math_token;
    }

    // now that we know for a fact what order we need to process the math tokens in. we now need to assign ownership to the math operators on what goes before and after them.

    for(uint8_t i = 0; i < counters->math_count; i++){
        MathToken* math_token = &cache->math[i];
        uint64_t pos = math_token->location->pos;

        math_token->before = NULL;
        math_token->after = NULL;

        GroupToken* group_token = math_token->group;
        for(uint64_t i = group_token->start_pos->pos; i < group_token->end_pos->pos+1; i++){
            if(i == pos){
                continue;
            }
            RealToken* real_token = &cache->real_tokens[i];
            if(real_token == NULL){
                dice_notation_set_error(notation, DN_ERROR_GOT_NULL, "failed to get real token, while iterating through real tokens, to assign ownership of math tokens");
                return 1;
            }
            
            uint8_t can_assign = 1;
            uint8_t check_used = 0;

            // this is a scan for what data needs to be verified
            switch(real_token->type){
                case TYPE_NUM:
                    check_used = 1;
                    can_assign = 1;
                    break;
                case TYPE_GROUP_START:
                case TYPE_GROUP_END: {
                    check_used = 0;
                    can_assign = 1;
                }; break;
                case TYPE_DICE:
                    check_used = 1;
                    can_assign = 1;
                    break;
                default: {
                    can_assign = 0;
                    continue;
                }; break;
            }

            if(check_used == 1){
                if(real_token->used == 1){
                    continue;
                }
            }

            if(can_assign == 1){
                if(i < pos){
                    math_token->before = real_token;
                    real_token->used = 1;
                } else {
                    math_token->after = real_token;
                    real_token->used = 1;
                    break; // should break out here since our job is done.
                }
            }

        }
    }

    return 0;
}

int dice_notation_define_keep(DiceNotation* notation){
    DiceNotationCache* cache = &notation->cache;
    DiceCounters* counters = &notation->counters;

    for(uint8_t i = 0; i < counters->keep_count; i++){
        KeepToken* keep_token = &cache->keep[i];
        if(keep_token == NULL){
            dice_notation_set_error(notation, DN_ERROR_GOT_NULL, "got null when trying to get a keep token");
            return 1;
        }

        uint64_t pos = keep_token->location->pos;
        RealToken* next_token = &cache->real_tokens[pos+1];
        if(next_token->type != TYPE_NUM){
            dice_notation_set_error(notation, DN_ERROR_SYNTAX, "syntax error, a number must follow a keep token");
            return 2;
        }
        keep_token->value = next_token;
        next_token->used = 1;
    }

    return 0;
}

int dice_notation_define_groups(DiceNotation* notation){
    DiceNotationCache* cache = &notation->cache;
    DiceCounters* counters = &notation->counters;

    if(counters->group_start_count != counters->group_end_count){
        dice_notation_set_error(notation, DN_ERROR_LOGIC, "group start count should be equal to group end count");
        return 1;
    }

    counters->group_count = counters->group_start_count + 1;

    cache->group = calloc(sizeof(GroupToken), counters->group_count);

    if(cache->group == NULL){
        dice_notation_set_error(notation, DN_ERROR_FAILED_ALLOC_CACHE, "failed to allocate memory for group cache");
        return 2;
    }

    cache->group[0].start_pos = &cache->real_tokens[0];
    cache->group[0].end_pos = &cache->real_tokens[counters->real_token_count-1];
    cache->group[0].priority = 0;
    cache->group[0].used = 0;

    for(uint8_t i = 0; i < counters->group_start_count; i++){
        GroupPlacementToken* start_token = &cache->start_tokens[i];
        GroupToken* group_token = &cache->group[i+1];
        group_token->start_pos = start_token->location;
        group_token->priority = start_token->priority;
        for(uint8_t j = 0; j < counters->group_end_count; j++){
            GroupPlacementToken* end_token = &cache->end_tokens[j];
            if(end_token->used == 1){
                continue;
            }
            if(end_token->priority != start_token->priority){
                continue;
            }
            if(end_token->location->pos < start_token->location->pos){
                continue;
            }
            // there should be a 3rd check here, but this is enough for now.
            group_token->end_pos = end_token->location;
            end_token->used = 1;
            break;
        }
        start_token->used = 1;
    }


    // sort the groups

    for(uint8_t i = 0; i < counters->group_count; i++){
        GroupToken* token_i = &cache->group[i];
        for(uint8_t j = i+1; j < counters->group_count; j++){
            GroupToken* token_j = &cache->group[j];
            uint8_t swap = 0;
            if(token_i->priority < token_j->priority){
                swap = 1;
            } else if(token_i->priority == token_j->priority) {
                // this is for cases where we have matching priority levels
                if(token_i->start_pos > token_j->start_pos){
                    swap = 1;
                }
            }

            // this is better than writing it out multiple times
            if(swap == 1){
                GroupToken temp;
                // don't you love copying memory directly :)
                memcpy(&temp, &cache->group[i], sizeof(GroupToken));
                memcpy(&cache->group[i], &cache->group[j], sizeof(GroupToken));
                memcpy(&cache->group[j], &temp, sizeof(GroupToken));
            }
        }
    }

    // now that we've sorted our groups, lets make sure to update our placement tokens so they actually know what group they belong to.
    for(uint8_t i = 0; i < counters->group_count; i++){
        GroupToken* group_token = &cache->group[i];
        if(group_token->priority == 0){
            continue; // skip the default group
        }
        GroupPlacementToken* placement_token = group_token->start_pos->special;
        // this may seem weird, but it is required since we did just change the order that the memory was sorted in.
        placement_token->group = group_token;
    }

    return 0;
}

int dice_notation_define_ranges(DiceNotation* notation){
    DiceNotationCache* cache = &notation->cache;
    DiceCounters* counters = &notation->counters;

    uint64_t total_ranges = counters->number_count + counters->math_count + counters->dice_count + counters->group_count;
    RangeToken* ranges = calloc(sizeof(RangeToken), total_ranges);
    uint64_t range_pos = 0;
    
    for(uint8_t i = 0; i < counters->number_count; i++){
        RangeToken* token = &ranges[range_pos + i];
        NumberToken* number_token = &cache->number[i];
        token->start = number_token->location;
        token->end = number_token->location;
        token->type = RESULT_NUMBER;
        token->out_value = (long)number_token->value;
    }
    range_pos = range_pos + counters->number_count;

    for(uint8_t i = 0; i < counters->math_count; i++){
        RangeToken* token = &ranges[range_pos+i];
        MathToken* math_token = &cache->math[i];
        token->start = math_token->before;
        token->end = math_token->after;
        token->type = RESULT_MATH;
        token->out_value = 0;
    }
    range_pos = range_pos + counters->math_count;

    for(uint8_t i = 0; i < counters->dice_count; i++){
        RangeToken* token = &ranges[range_pos+i];
    }

    return 0;
}

void dice_notation_clean(DiceNotation* notation){
    if(notation == NULL){
        return;
    }

    DiceNotationCache* cache = &notation->cache;

    // clean cache
    if(cache->tokens != NULL){
        free(cache->tokens);
        cache->tokens = NULL;
    }

    if(cache->dice != NULL){
        free(cache->dice);
        cache->dice = NULL;
    }

    if(cache->keep != NULL){
        free(cache->keep);
        cache->keep = NULL;
    }

    if(cache->math != NULL){
        free(cache->math);
        cache->math = NULL;
    }
    
    if(cache->number != NULL){
        free(cache->number);
        cache->number = NULL;
    }

    if(cache->start_tokens != NULL){
        free(cache->start_tokens);
        cache->start_tokens = NULL;
    }

    if(cache->end_tokens != NULL){
        free(cache->end_tokens);
        cache->end_tokens = NULL;
    }

    if(cache->group != NULL){
        free(cache->group);
        cache->group = NULL;
    }

    if(cache->real_tokens != NULL){
        free(cache->real_tokens);
    }

    // finally clear main notation
    free(notation);
    notation = NULL;
}

void DiceToken_Print(DiceToken* dice_token){
    printf("[%p] Dice(sides: %p, amount: %p, keep_high: %p, keep_low: %p)", dice_token, dice_token->sides, dice_token->amount, dice_token->keep_high, dice_token->keep_low);
}

void dice_notation_print_data(DiceNotation* notation){
    DiceNotationCache* cache = (DiceNotationCache*)&notation->cache;
    DiceCounters* counters = (DiceCounters*)&notation->counters;

    for(uint64_t i = 0; i < (uint64_t)counters->real_token_count; i++){
        RealToken* real_token = (RealToken*)&cache->real_tokens[i];
        if(real_token == NULL){
            printf("GOT NULL\n");
        }

        printf("[%p] RealToken(pos: %lu, type: %s, used: %d, value: \n\t", real_token, real_token->pos,TokenType_as_string(real_token->type), real_token->used);
        switch(real_token->category){
            case TOKEN_NONE:
                printf("NONE");
                break;
            case TOKEN_DICE: {
                DiceToken_Print((DiceToken*)real_token->special);
            }; break;
            case TOKEN_GROUP: {
                GroupToken* group_token = (GroupToken*)real_token->special;
                printf("[%p] Group()", group_token);
            }; break;
            case TOKEN_NUMBER: {
                NumberToken* number_token = (NumberToken*)real_token->special;
                printf("[%p] Number(value: %d, used: %d)", number_token, number_token->value, number_token->used);
            }; break;
            case TOKEN_MATH: {
                MathToken* math_token = (MathToken*)real_token->special;
                int math_index = math_token - cache->math;
                printf("[%p] Math(index: %d, op: %s, group: %p, group_priority: %d, priority: %d, before: %p, after: %p)", math_token, math_index, TokenType_as_string(math_token->type), math_token->group, math_token->group_priority, math_token->priority, math_token->before, math_token->after);
            }; break;
            case TOKEN_KEEP: {
                KeepToken* keep_token = (KeepToken*)real_token->special;
                printf("[%p] Keep(value: %p)", keep_token, keep_token->value);
            }; break;
            case TOKEN_GROUP_PLACEMENT: {
                GroupPlacementToken* placement_token = (GroupPlacementToken*)real_token->special;
                printf("[%p] GroupPlacement(type: %s, priority: %d, used: %d, group: %p)", placement_token, TokenType_as_string(real_token->type), placement_token->priority, placement_token->used, placement_token->group);
            }; break;
            default: {
                printf("unknown real token category\n");
            }; break;
        }
        printf("\n)\n");
    }

    printf("Groups:\n");
    for(uint8_t i = 0; i < counters->group_count; i++){
        GroupToken* group_token = &cache->group[i];
        GroupPlacementToken* start_token = group_token->start_pos->special;
        printf("\t[%p] Group(priority: %d, start: [%p] %ld, end: [%p] %ld, used: %d)\n", group_token, group_token->priority, group_token->start_pos, group_token->start_pos->pos, group_token->end_pos, group_token->end_pos->pos, group_token->used);
    }
}

DiceNotation* dice_notation(const char *text)
{
    DiceNotation* notation = calloc(sizeof(DiceNotation), 1);

    if(notation == NULL){
        printf("failed to allocate space for dice notation\n");
        return notation;
    }

    dice_notation_set_error(notation, 0, NULL);

    memset(&notation->state, 0, sizeof(DiceState));
    memset(&notation->cache, 0, sizeof(DiceNotationCache));
    memset(&notation->counters, 0, sizeof(DiceCounters));

    if(text == NULL){
        dice_notation_set_error(notation, DN_ERROR_BAD_ARG, "text cannot be null\n");
        return notation;
    }

    notation->state.text = text;
    notation->state.length = strlen(text);

    if(notation->state.length == 0){
        dice_notation_set_error(notation, DN_ERROR_BAD_ARG, "string does not mean minimum length requirements\n");
        return notation;
    }

    int retvalue = 0;

    retvalue = dice_notation_parse_text(notation);
    if(retvalue != 0){
        printf("Error Occurred when parsing text:\n%s\n", notation->errmsg);
        return notation;
    }

    PairTokens_PrintList(notation->cache.tokens, notation->state.length);

    retvalue = dice_notation_allocate_space_for_cache(notation);
    if(retvalue != 0){
        printf("Error Occurred when allocating space for cache:\n%s\n", notation->errmsg);
        return notation;
    }


    retvalue = dice_notation_organize_tokens(notation);
    if(retvalue != 0){
        printf("Error Occurred when organizing tokens:\n%s\n", notation->errmsg);
        return notation;
    }

    printf("test\n");

    retvalue = dice_notation_define_keep(notation);
    if(retvalue != 0){
        printf("Error Occurred when defining keep:\n%s\n", notation->errmsg);
        return notation;
    }


    retvalue = dice_notation_define_dice(notation);

    if(retvalue != 0){
        printf("Error Occurred when defining dice:\n%s\n", notation->errmsg);
        return notation;
    }

    retvalue = dice_notation_define_groups(notation);

    if(retvalue != 0){
        printf("Error Occurred when defining groups:\n%s\n", notation->errmsg);
        return notation;
    }

    retvalue = dice_notation_define_math(notation);

    if(retvalue != 0){
        printf("Error Occurred when defining math tokens:\n%s\n", notation->errmsg);
        return notation;
    }

    retvalue = dice_notation_define_ranges(notation);

    dice_notation_print_data(notation);

    return notation;
}

long dice_notation_run(DiceNotation *notation)
{
    DiceNotationCache* cache = &notation->cache;
    DiceCounters* counters = &notation->counters;

    for(uint8_t group_pos = 0; group_pos < counters->group_count; group_pos++){
        GroupToken* group_token = &cache->group[group_pos];
        if(group_token == NULL){
            dice_notation_set_error(notation, DN_ERROR_GOT_NULL, "failed to get group token, got null");
            return 0;
        }

        uint64_t start_pos = group_token->start_pos->pos;
        uint64_t end_pos = group_token->end_pos->pos;

        uint64_t size = end_pos - start_pos;

        if(size <= 1){
            printf("how did we get here?\n");
            continue;
        }

        // walkthrough math
        for(uint8_t i = 0; i < counters->math_count; i++){
            MathToken* math_token = &cache->math[i];
            if(math_token->group != group_token){
                continue; // not our group so skip it.
            }

            // stash both of them in a temporary array that we can iterate through. that way we can process special types
            RealToken* temp_tokens[2] = { math_token->before, math_token->after };
            for(uint8_t temp_i = 0; temp_i < 2; temp_i++){
                RealToken* temp_token = temp_tokens[temp_i];
                if(temp_token == NULL){
                    printf("failed to accurately access temp cache for real token, got null value\n");
                    continue;
                }

                switch(temp_token->type){
                    case TYPE_DICE: {

                    }; break;
                    case TYPE_NUM: {

                    }; break;
                    case TYPE_GROUP_END: {

                    }; break;
                    case TYPE_GROUP_START: {

                    }; break;
                    default:
                        break;
                }
            }
        }

        for(uint64_t i = start_pos+1; i < end_pos; i++){
            RealToken* real_token = &cache->real_tokens[i];
            if(real_token == NULL){
                dice_notation_set_error(notation, DN_ERROR_GOT_NULL, "got null when trying to get real token");
                return 0;
            }

            if(real_token->used == 1){
                continue;
            }
        }


    }

    return 0;
}

RangeCache* RangeCache_Create(){
    RangeCache* cache = calloc(sizeof(RangeCache), 1);
    if(cache == NULL){
        printf("failed to allocate space for range cache\n");
        return cache;
    }
    cache->capacity = 1;
    cache->pos = 0;

    cache->tokens = calloc(sizeof(RangeToken), cache->capacity * DN_RANGE_CACHE_ALLOC_SZ);
    if(cache->tokens == NULL){
        printf("failed to allocate space for cache tokens\n");
        free(cache);
        cache = NULL;
        return cache;
    }

    return cache;
}

int RangeCache_Alloc(RangeCache* cache, uint8_t size){
    if(cache == NULL){
        return 1;
    }
    if(cache->tokens == NULL){
        return 2;
    }

    if(size == cache->capacity){
        // redundant
        return 0;
    }

    uint64_t new_size = sizeof(RangeToken) * (DN_RANGE_CACHE_ALLOC_SZ * size);

    if(new_size < cache->pos){
        return 4; // cannot realloc to a size smaller than our stored values
    }

    void* new_cache_tokens = realloc(cache->tokens, new_size);
    if(new_cache_tokens == NULL){
        return 3;
    }
    cache->tokens = new_cache_tokens;
    cache->capacity = size;

    return 0;
}

int RangeCache_Add(RangeCache* cache, RangeToken* token){
    if(cache == NULL){
        return 1;
    }
    if(cache->tokens == NULL){
        return 2;
    }

    if(token == NULL){
        return 3;
    }

    memcpy(&cache->tokens[cache->pos], token, sizeof(RangeToken));

    if((cache->pos + 1) > (cache->capacity - 1)){
        int retvalue = RangeCache_Alloc(cache, cache->capacity+1);
        if(retvalue != 0){
            return 10 + retvalue; // assuming we don't have more than 10 errors in the base function
        }
    }

    return 0;
}

void RangeCache_Destroy(RangeCache* cache){
    if(cache != NULL){
        if(cache->tokens != NULL){
            free(cache->tokens);
            cache->tokens = NULL;
        }

        free(cache);
        cache = NULL;
    }
}
