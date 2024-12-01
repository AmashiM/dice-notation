#include "DiceError.h"

void DiceError::SetError(DiceErrorType type, const char *errmsg)
{
    this->errmsg = errmsg;
    this->type = type;
}
