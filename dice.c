#include "dice.h"

// local type definition
void dice_notation_set_error(DiceNotation* notation, int errcode, const char* errmsg);
void dice_notation_debug();


// definitions
void dice_notation_set_error(DiceNotation* notation, int errcode, const char* errmsg){
    notation->errcode = errcode;
    notation->errmsg = errmsg;
}

void dice_notation_debug(){
    printf("# Sizes\n\tDiceNotation: %lu\n\tDiceNotationState: %lu\n\tDiceNotationCache: %lu\n\tDiceNotationCounters: %lu\n", sizeof(DiceNotation), sizeof(DiceNotationState), sizeof(DiceNotationCache), sizeof(DiceNotationCounters));
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
    DiceNotationState* state = &notation->state;
    DiceNotationCache* cache = &notation->cache;
    DiceNotationCounters* counters = &notation->counters;

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

int dice_notation_organize_tokens(DiceNotation* notation){
    DiceNotationCache* cache = &notation->cache;
    DiceNotationCounters* counters = &notation->counters;

    cache->dice = calloc(sizeof(DiceToken), counters->dice_count);
    cache->math = calloc(sizeof(MathToken), counters->math_count);
    cache->keep = calloc(sizeof(KeepToken), counters->keep_count);
    cache->number = calloc(sizeof(NumberToken), counters->number_count);

    cache->start_tokens = calloc(sizeof(GroupPlacementToken), counters->group_start_count);
    cache->end_tokens = calloc(sizeof(GroupPlacementToken), counters->group_end_count);

    cache->real_tokens = calloc(sizeof(RealToken), counters->real_token_count);

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
    DiceNotationCounters* counters = &notation->counters;

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

int dice_notation_define_math(DiceNotation* notation){
    DiceNotationCache* cache = &notation->cache;
    DiceNotationCounters* counters = &notation->counters;

    for(uint8_t i = 0; i < counters->math_count; i++){
        MathToken* math_token = &cache->math[i];
        RealToken* real_token = math_token->location;

        math_token->priority = MathToken_GetPriority(math_token->type);
    }

    return 0;
}

int dice_notation_define_keep(DiceNotation* notation){
    DiceNotationCache* cache = &notation->cache;
    DiceNotationCounters* counters = &notation->counters;

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
    DiceNotationCounters* counters = &notation->counters;

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
            if(token_i->priority < token_j->priority){
                GroupToken temp;
                // don't you love copying memory directly :)
                memcpy(&temp, &cache->group[i], sizeof(GroupToken));
                memcpy(&cache->group[i], &cache->group[j], sizeof(GroupToken));
                memcpy(&cache->group[j], &temp, sizeof(GroupToken));
            } else if(token_i->priority == token_j->priority) {
                // this is for cases where we have matching priority levels
                if(token_i->start_pos > token_j->start_pos){
                    GroupToken temp;
                    // don't you love copying memory directly :)
                    memcpy(&temp, &cache->group[i], sizeof(GroupToken));
                    memcpy(&cache->group[i], &cache->group[j], sizeof(GroupToken));
                    memcpy(&cache->group[j], &temp, sizeof(GroupToken));
                }
            }
        }
    }

    return 0;
}

void dice_notation_clean(DiceNotation* notation){
    if(notation == NULL){
        return;
    }

    DiceNotationCache* cache = &notation->cache;

    // clean cache
    free(cache->tokens);
    cache->tokens = NULL;

    if(cache->tokens != NULL){
        free(cache->tokens);
        cache->tokens = NULL;
    }

    free(cache->dice);
    free(cache->keep);
    free(cache->math);
    free(cache->number);

    free(cache->start_tokens);
    free(cache->end_tokens);

    free(cache->real_tokens);

    // finally clear main notation
    free(notation);
    notation = NULL;
}

void dice_notation_print_data(DiceNotation* notation){
    DiceNotationCache* cache = (DiceNotationCache*)&notation->cache;
    DiceNotationCounters* counters = (DiceNotationCounters*)&notation->counters;

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
                DiceToken* dice_token = (DiceToken*)real_token->special;
                printf("[%p] Dice(sides: %p, amount: %p, keep_high: %p, keep_low: %p)", dice_token, dice_token->sides, dice_token->amount, dice_token->keep_high, dice_token->keep_low);
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
                printf("[%p] Math(op: %s)", math_token, TokenType_as_string(math_token->type));
            }; break;
            case TOKEN_KEEP: {
                KeepToken* keep_token = (KeepToken*)real_token->special;
                printf("[%p] Keep()", keep_token);
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

    memset(&notation->state, 0, sizeof(DiceNotationState));
    memset(&notation->cache, 0, sizeof(DiceNotationCache));
    memset(&notation->counters, 0, sizeof(DiceNotationCounters));

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

    retvalue = dice_notation_organize_tokens(notation);
    if(retvalue != 0){
        printf("Error Occurred when organizing tokens:\n%s\n", notation->errmsg);
        return notation;
    }

    printf("test\n");

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

    dice_notation_print_data(notation);

    return notation;
}

long dice_notation_run(DiceNotation *notation)
{
    DiceNotationCache* cache;
    DiceNotationCounters* counters;

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
