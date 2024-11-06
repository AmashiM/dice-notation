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

void SetTokenLocation(TokenLocation* location, uint64_t pos, void* ref){
    location->pos = pos;
    location->ref = ref;
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

// main operations
int dice_notation_parse_text(DiceNotation* notation){
    DiceNotationState* state = &notation->state;
    DiceNotationCache* cache = &notation->cache;
    DiceNotationCounters* counters = &notation->counters;

    cache->tokens = calloc(sizeof(PairToken), state->length);

    if(cache->tokens == NULL){
        dice_notation_set_error(notation, DN_ERROR_FAILED_ALLOC_CACHE, "failed to allocate space in cache for tokens\n");
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
    for(uint64_t i = 0; i < counters->real_token_count; i++){
        PairToken* token = &cache->tokens[i];
        if(token == NULL){
            dice_notation_set_error(notation, DN_ERROR_GOT_NULL, "failed to get token from cache, received null value\n");
            return -1;
        }
        // skip these ones
        if(token->type == TYPE_EMPTY){
            continue;
        }

        RealToken* real_token = &cache->real_tokens[i];

        switch(token->type){
            case TYPE_EMPTY:
                continue;
                break;
            case TYPE_DICE: {
                DiceToken* dice_token = &cache->dice[counters->dice_pos];
                SetTokenLocation(&dice_token->location, counters->real_token_pos, real_token);
                SetRealToken(real_token, counters->real_token_pos, token->type, TOKEN_DICE, dice_token);
                counters->dice_pos++;
            }; break;
            case TYPE_NUM: {
                NumberToken* number_token = &cache->number[counters->number_pos];
                SetTokenLocation(&number_token->location, counters->real_token_pos, real_token);
                SetRealToken(real_token, counters->real_token_pos, token->type, TOKEN_NUMBER, number_token);
                number_token->value = token->value;
                counters->number_pos++;
            }; break;
            case TYPE_MATH: {
                MathToken* math_token = &cache->math[counters->math_pos];
                SetTokenLocation(&math_token->location, counters->real_token_pos, real_token);
                math_token->type = token->value;
                counters->math_pos++;
            }; break;
            default:
                break;
        }
        counters->real_token_pos++;
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

    PairTokens_PrintList(notation->cache.tokens, notation->state.length);

    return notation;
}
