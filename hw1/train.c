#include "hmm.h"
#include <math.h>

#define DEBUG

int main(int argc, char* const argv[]) {

    // check program params
    if (argc != 4) {
        printf("usage: ./train [number of iterations] [initial model] [observed sequences]\n");
        exit(-1);
    }

    // setup basic var
    int numberOfIter = atoi(argv[1]);
    const char* initModelName = argv[2];
    const char* observedSeq = argv[3];
#ifdef DEBUG
    printf("numberOfIter = %d\ninitModelName = %s\nobservedSeq = %s\n", numberOfIter, initModelName, observedSeq);
#endif

    // load initial model
    HMM hmm_initial;
    loadHMM( &hmm_initial, "../model_init.txt" );
#ifdef DEBUG
    printf("loaded model_init.txt\n");
    dumpHMM( stderr, &hmm_initial );
#endif

    return 0;
}
