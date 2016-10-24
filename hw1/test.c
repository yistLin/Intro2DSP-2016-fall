#include "hmm.h"
#include <math.h>

// #define DEBUG

#define observ2int(c) ((int)(c - 'A'))
#define LZERO  (0.000001)
#define LSMALL (0.00001)
#define log_prob(x) (x < LSMALL ? log(LZERO) : log(x))

double viterbi(HMM* hmm, char* sample, const int numberOfState) {
    double delta[MAX_SEQ][MAX_STATE];
    const int lengthOfSeq = strlen(sample);
    double max_source = 0, tmp_source;
    double prob_logged = -0.5E10;

    // initialization
    for (int i = 0; i < numberOfState; i++) {
        delta[0][i] = log_prob(hmm->initial[i]) + log_prob(hmm->observation[observ2int(sample[0])][i]);
    }

    // induction
    for (int t = 1; t < lengthOfSeq; t++) {
        for (int j = 0; j < numberOfState; j++) {
            max_source = -0.5E10;
            for (int i = 0; i < numberOfState; i++) {
                tmp_source = delta[t-1][i] + log_prob(hmm->transition[i][j]);
                if (tmp_source > max_source)
                    max_source = tmp_source;
            }
            // printf("max_source = %lf, ", max_source);
            delta[t][j] = max_source + log_prob(hmm->observation[observ2int(sample[t])][j]);
            // printf("delta[%d][%d] = %lf\n", t, j, delta[t][j]);

            // termination
            if (t == lengthOfSeq-1 && delta[t][j] > prob_logged)
                prob_logged = delta[t][j];
        }
        // printf("\n");
    }

    // printf("exp(%lf) = %e\n", prob_logged, exp(prob_logged));
    return exp(prob_logged);
}

int main(int argc, char* const argv[]) {

    // check program params
    if (argc != 4) {
        fprintf(stderr, "usage: ./train [model list] [testing data] [result.txt]\n");
        exit(-1);
    }

    // setup basic var
    const char* modellist = argv[1];
    const char* testingdata = argv[2];
    const char* resultfile = argv[3];
#ifdef DEBUG
    fprintf(stderr, "modellist = %s\ntestingdata = %s\nresultfile = %s\n", modellist, testingdata, resultfile);
#endif

    // load model list
    const int MAX_MODEL = 20;
    HMM hmmlist[MAX_MODEL];
    const int numberOfModel = load_models(modellist, hmmlist, MAX_MODEL);
#ifdef DEBUG
    fprintf(stderr, "loaded model_list.txt\n");
    for (int i = 0; i < numberOfModel; i++) {
        fprintf(stderr, "model name = %s\n", hmmlist[i].model_name);
        dumpHMM(stderr, &hmmlist[i]);
        fprintf(stderr, "\n");
    }
#endif

    // open sequence file
    char** seq_arr = (char**)malloc(sizeof(char*) * 10000);
    if (seq_arr == NULL) {
        fprintf(stderr, "Cannot allocate seq_arr\n");
        exit(-1);
    }
    char input_buf[MAX_LINE];
    int numberOfLine = 0;
    FILE* seq_fp = open_or_die(testingdata, "r");
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

    // open output file
    FILE* output_fp = open_or_die(resultfile, "w");

    // run viterbi algorithm
    int likely_model = 0;
    double highest_prob = 0.0;
    const int numberOfState = hmmlist[0].state_num;
    for (int line = 0; line < numberOfLine; line++) {
        highest_prob = 0.0;
        for (int m = 0; m < numberOfModel; m++) {
            double prob = viterbi(&hmmlist[m], seq_arr[line], numberOfState);
            if (prob > highest_prob) {
                highest_prob = prob;
                likely_model = m;
            }
            // return 0;
        }
        // fprintf(output_fp, "%s %e\n", hmmlist[likely_model].model_name, highest_prob);
        fprintf(output_fp, "%s\n", hmmlist[likely_model].model_name);
    }

    // close output file
    fclose(output_fp);

    // release allocated memory
    for (int i = 0; i < 10000; i++) {
        free(seq_arr[i]);
    }
    free(seq_arr);

    return 0;
}
