#include "hmm.h"
#include <math.h>

// #define DEBUG

#define observ2int(c) ((int)(c - 'A'))

int forward(HMM* hmm, char* sample, double alpha[][MAX_STATE], const int numberOfState, const int lengthOfSeq) {
    // initialization
    for (int i = 0; i < numberOfState; i++) {
        alpha[0][i] = hmm->initial[i] * \
            hmm->observation[observ2int(sample[0])][i];
    }

    // induction
    double sum = 0;
    for (int t = 0; t < lengthOfSeq-1; t++) {
        for (int j = 0; j < numberOfState; j++) {
            sum = 0;
            for (int i = 0; i < numberOfState; i++) {
                sum += alpha[t][i] * hmm->transition[i][j];
            }
            alpha[t+1][j] = sum * hmm->observation[observ2int(sample[t+1])][j];
        }
    }

    return 0;
}

int backward(HMM* hmm, char* sample, double beta[][MAX_STATE], const int numberOfState, const int lengthOfSeq) {
    // initialization
    for (int i = 0; i < numberOfState; i++) {
        beta[lengthOfSeq-1][i] = 1;
    }

    // induction
    double sum = 0;
    for (int t = lengthOfSeq-2; t >= 0; t--) {
        for (int i = 0; i < numberOfState; i++) {
            sum = 0;
            for (int j = 0; j < numberOfState; j++) {
                sum += beta[t+1][j] * hmm->transition[i][j] * hmm->observation[observ2int(sample[t+1])][j];
            }
            beta[t][i] = sum;
        }
    }
    
    return 0;
}

int calc_gamma(char* sample, double alpha[][MAX_STATE], double beta[][MAX_STATE], double gamma[][MAX_STATE], double sum_gamma[][MAX_STATE], double sum_bjk[][MAX_STATE], const int numberOfState, const int lengthOfSeq) {
    double denominator = 0;
    for (int t = 0; t < lengthOfSeq; t++) {
        denominator = 0;
        for (int i = 0; i < numberOfState; i++) {
            denominator += alpha[t][i] * beta[t][i];
        }
        for (int i = 0; i < numberOfState; i++) {
            gamma[t][i] = (alpha[t][i] * beta[t][i]) / denominator;
            sum_gamma[t][i] += gamma[t][i];
            sum_bjk[i][observ2int(sample[t])] += gamma[t][i];
        }
    }
    
    return 0;
}

int calc_epsilon(HMM* hmm, char* sample, double alpha[][MAX_STATE], double beta[][MAX_STATE], double epsilon[][MAX_STATE][MAX_STATE], double sum_epsilon[][MAX_STATE][MAX_STATE], const int numberOfState, const int lengthOfSeq) {
    double denominator = 0;
    for (int t = 0; t < lengthOfSeq-1; t++) {
        denominator = 0;
        for (int i = 0; i < numberOfState; i++) {
            for (int j = 0; j < numberOfState; j++) {
                denominator += alpha[t][i] * hmm->transition[i][j] * hmm->observation[observ2int(sample[t+1])][j] * beta[t+1][j];
            }
        }
        for (int i = 0; i < numberOfState; i++) {
            for (int j = 0; j < numberOfState; j++) {
                epsilon[t][i][j] = (alpha[t][i] * hmm->transition[i][j] * hmm->observation[observ2int(sample[t+1])][j] * beta[t+1][j]) / denominator;
                sum_epsilon[t][i][j] += epsilon[t][i][j];
            }
        }
    }
    return 0;
}

int main(int argc, char* const argv[]) {

    // check program params
    if (argc != 5) {
        fprintf(stderr, "usage: ./train [number of iterations] [initial model] [observed sequences] [model output]\n");
        exit(-1);
    }

    // setup basic var
    int numberOfIter = atoi(argv[1]);
    const char* initModelName = argv[2];
    const char* observedSeq = argv[3];
    const char* outputModel = argv[4];
#ifdef DEBUG
    fprintf(stderr, "numberOfIter = %d\ninitModelName = %s\nobservedSeq = %s\noutputModel = %s\n", numberOfIter, initModelName, observedSeq, outputModel);
#endif

    // load initial model
    HMM hmm_initial;
    loadHMM(&hmm_initial, initModelName);
#ifdef DEBUG
    fprintf(stderr, "loaded model_init.txt\n");
    dumpHMM(stderr, &hmm_initial);
#endif

    // open sequence file
    char** seq_arr = (char**)malloc(sizeof(char*) * 10000);
    if (seq_arr == NULL) {
        fprintf(stderr, "Cannot allocate seq_arr\n");
        exit(-1);
    }
    char input_buf[MAX_LINE];
    int numberOfLine = 0;
    FILE* seq_fp = open_or_die(observedSeq, "r");
    while (fscanf(seq_fp, "%s", input_buf) != EOF) {
        seq_arr[numberOfLine] = (char*)malloc(sizeof(char) * MAX_SEQ);
        if (seq_arr[numberOfLine] == NULL) {
            fprintf(stderr, "Cannot allocate seq_arr[numberOfLine]\n");
            exit(-1);
        }
        strcpy(seq_arr[numberOfLine], input_buf);
        numberOfLine++;
    }
    fclose(seq_fp);

    // setup model info
    const int numberOfState = hmm_initial.state_num;
    const int numberOfObserv = hmm_initial.observ_num;
    const int lengthOfSeq = strlen(seq_arr[0]);
#ifdef DEBUG
    fprintf(stderr, "numberOfState = %d\nlengthOfSeq = %d\n", numberOfState, lengthOfSeq);
#endif

    // for looping
    double alpha[MAX_SEQ][MAX_STATE];
    double beta[MAX_SEQ][MAX_STATE];
    double gamma[MAX_SEQ][MAX_STATE];
    double epsilon[MAX_SEQ][MAX_STATE][MAX_STATE];

    // for accumulating
    double sum_gamma[MAX_SEQ][MAX_STATE];
    double sum_epsilon[MAX_SEQ][MAX_STATE][MAX_STATE];
    double sum_bjk[MAX_OBSERV][MAX_STATE];

    for (int iter = 0; iter < numberOfIter; iter++) {

        for (int i = 0; i < MAX_SEQ; i++)
            for (int j = 0; j < MAX_STATE; j++)
                sum_gamma[i][j] = 0.0;
        for (int i = 0; i < MAX_SEQ; i++)
            for (int j = 0; j < MAX_STATE; j++)
                for (int k = 0; k < MAX_STATE; k++)
                    sum_epsilon[i][j][k] = 0.0;
        for (int i = 0; i < MAX_OBSERV; i++)
            for (int j = 0; j < MAX_STATE; j++)
                sum_bjk[i][j] = 0.0;

        // loop through all samples
        for (int line = 0; line < numberOfLine; line++) {
            forward(&hmm_initial, seq_arr[line], alpha, numberOfState, lengthOfSeq);
            backward(&hmm_initial, seq_arr[line], beta, numberOfState, lengthOfSeq);
            calc_gamma(seq_arr[line], alpha, beta, gamma, sum_gamma, sum_bjk, numberOfState, lengthOfSeq);
            calc_epsilon(&hmm_initial, seq_arr[line], alpha, beta, epsilon, sum_epsilon, numberOfState, lengthOfSeq);
        }

        // calc new model params
        for (int i = 0; i < numberOfState; i++) {
            hmm_initial.initial[i] = sum_gamma[0][i] / numberOfLine;
        }
        double sum_up = 0, sum_down = 0;
        for (int i = 0; i < numberOfState; i++) {
            for (int j = 0; j < numberOfState; j++) {
                sum_up = sum_down = 0;
                for (int t = 0; t < lengthOfSeq-1; t++) {
                    sum_up += sum_epsilon[t][i][j];
                    sum_down += sum_gamma[t][i];
                }
                hmm_initial.transition[i][j] = sum_up / sum_down;
            }
        }
        for (int j = 0; j < numberOfState; j++) {
            for (int k = 0; k < numberOfObserv; k++) {
                sum_down = 0;
                for (int t = 0; t < lengthOfSeq; t++) {
                    sum_down += sum_gamma[t][j];
                }
                hmm_initial.observation[k][j] = sum_bjk[j][k] / sum_down;
            }
        }
    }

    // dumpHMM to see the results
    // dumpHMM(stderr, &hmm_initial);

    // dumpHMM to output file
    FILE* outputfile = open_or_die(outputModel, "w");
    dumpHMM(outputfile, &hmm_initial);
    fclose(outputfile);

    // release allocated memory
    for (int i = 0; i < 10000; i++) {
        free(seq_arr[i]);
    }
    free(seq_arr);

    return 0;
}
