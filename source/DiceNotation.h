#pragma once
#include "stdio.h"
#include "stdlib.h"
#include "stdint.h"
#include "string.h"
#include "time.h"

#include "Tokens.h"
#include "DiceCache.h"
#include "DiceCounters.h"

#define DICENOTATION_SZ sizeof(DiceNotation)

class DiceNotation {
public:
    DiceNotation();
    ~DiceNotation();

    static bool IsDigit(char c);

    static void Init();

    void SetText(const char* text);

    void SetError(DiceErrorType type, const char* errmsg);

    int ProcessText();

    int ShrinkTokenCache();

    int DefineGroups();

    int AssertGroupDominance();

    int DefineIndexCaches();

    int DefineRanges();

    int Process();

    DiceResult* Run();


    // static RangeToken* DefineRangesForTokenList(DiceError* error, PairToken* tokens, int size);
    
private:

    DiceResult* RunRange(RangeToken* token);

    DiceResult* RunNumber(RangeToken* token);

    DiceResult* RunDice(RangeToken* token);

    DiceResult* RunAdd(RangeToken* token);

    RangeToken* DetermineStartingRange();

    int DefineNumberRanges();
    int DefineKeepRanges();
    int DefineDiceRanges();

    int DefineMathRanges();

    int DefineAddRanges();
    int DefineSubRanges();
    int DefineMultRanges();
    int DefineDivRanges();

    int SortGroups();

    int DefineGroupRanges();

    int CalcRangeCount();

    void DebugPrintTokens();

    void DebugPrintRangeToken(RangeToken* token);

    void DebugPrintData(const char* name, void** data, int size);

    void DebugPrintRanges();

    void DebugPrintGroupPlacements(const char* name, GroupPlacement* placements, int size);
    
    void DebugPrintGroup(Group* group);

    void DebugPrintGroups();

    void DebugPrint();

    DiceCache* cache;
    DiceCounters* counters;
    DiceError error;
    const char* text;
    size_t length;
};