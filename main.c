#include "dice.h"

int main(void){

    dice_notation_debug();

    DiceNotation* notation = dice_notation("1d4 + 1");
    if(notation == NULL){
        return 1;
    }
    if(notation->errcode != 0){
        printf("Got Error[%d]: %s\n", notation->errcode, notation->errmsg);
        return 2;
    }


    free(notation);

    return 0;
}