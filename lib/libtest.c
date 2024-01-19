#include "physics.h"

#include "raylib.h"
#include "raymath.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

int main(void){

    printf("Testing Queue...\n");
    Queue myQ;
    init_queue(&myQ, 10);

    for (int i=0; i<10; i++){
        enqueue(&myQ, (Vector2){i,i});
    }
    printQueue(myQ);

    return 0;
}