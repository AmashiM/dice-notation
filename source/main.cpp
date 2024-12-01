
#include "DiceNotation.h"

int main(void){
    DiceNotation* notation = new DiceNotation();
    notation->SetText("(1d1 + 1) + 1");
    notation->Process();

    // for(int i = 0; i < 5; i++){
    //     DiceResult* result = notation->Run();
    //     if(result == NULL){
    //         printf("error occurred when trying to produce a dice result\n");
    //     } else {
    //         printf("got result: %ld\n", result->value);
    //     }
    //     free(result);
    // }

    return 0;
}