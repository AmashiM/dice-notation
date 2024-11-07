
# Saving Data

to save the processed data for dice notations we save them to `struct DiceNotation`

## DiceNotation
DiceNotation contains error handling and 3 different structs for data.

1. `DiceNotationState` - this holds the length of the text `size_t` and the text `const char*` 
2. `DiceNotationCache` - this holds tokens for the different tokens
3. `DiceNotationCounters` - this keeps count of a lot of important numbers

---

## Tokens

### Real Tokens

1. DiceToken
2. NumberToken
3. MathToken
4. GroupPlacementToken
5. GroupToken
4. KeepToken

---
---
---

# Processing Data

need to define what belonds to what token.