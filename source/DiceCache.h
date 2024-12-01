#pragma

#include "stdio.h"
#include "stdlib.h"
#include "stdint.h"
#include "string.h"

#include "Tokens.h"
#include "DiceCounters.h"
#include "DiceError.h"

class DiceCache {
public:
    DiceCache();
    ~DiceCache();

    DiceError* error;

    PairToken* tokens;

    void** dice;
    void** numbers;
    void** keep;
    void** add;
    void** sub;
    void** mult;
    void** div;

    GroupPlacement* group_start;
    GroupPlacement* group_end;

    Group* group;

    RangeToken* ranges;

    int AllocateIndexes(DiceCounters* counters);
    
    void** GetCacheFromTokenType(TokenType type);

    int IndexOfToken(PairToken* token);
    PairToken* GetPreviousUnclaimed(int pos);
    PairToken* GetNextUnclaimed(int pos, int length);

private:

    void SetError(DiceErrorType type, const char* errmsg);

    void CleanTokens();

    void Clean();
};