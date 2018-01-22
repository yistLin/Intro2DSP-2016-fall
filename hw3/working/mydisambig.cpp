#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <iostream>
#include <fstream>
#include <vector>
#include <unordered_map>
using namespace std;

#include "Ngram.h"

char TEXT_FILENAME[256];
char MAP_FILENAME[256];
char LM_FILENAME[256];
int ORDER = 0;

int cmd_params(char* const argv[]) {
    for (int i = 0; i < 8; i++) {
        char* arg = argv[i];
        if (strcmp(arg, "-text") == 0) {
            strcpy(TEXT_FILENAME, argv[i+1]);
        }
        else if (strcmp(arg, "-map") == 0) {
            strcpy(MAP_FILENAME, argv[i+1]);
        }
        else if (strcmp(arg, "-lm") == 0) {
            strcpy(LM_FILENAME, argv[i+1]);
        }
        else if (strcmp(arg, "-order") == 0) {
            ORDER = atoi(argv[i+1]);
        }
    }
    return 0;
}

double getBigramProb(Ngram& lm, Vocab& voc, const char *w1, const char *w2)
{
    VocabIndex wid1 = voc.getIndex(w1);
    VocabIndex wid2 = voc.getIndex(w2);

    if(wid1 == Vocab_None)  //OOV
        wid1 = voc.getIndex(Vocab_Unknown);
    if(wid2 == Vocab_None)  //OOV
        wid2 = voc.getIndex(Vocab_Unknown);

    VocabIndex context[] = { wid1, Vocab_None };
    double prob = lm.wordProb( wid2, context );
    if (isinf(prob))
        return -100;
    else
        return prob;
}

int main(int argc, char* const argv[]) {
    if (argc != 9) {
        fprintf(stderr, "usage: ./mydisambig -text [text] -map [map] -lm [lm] -order [# order]\n");
        return -1;
    }

    cmd_params(argv);

    Vocab voc;
    Ngram lm(voc, ORDER);
    {
        File lmFile(LM_FILENAME, "r");
        lm.read(lmFile);
        lmFile.close();
    }

    // build mapping
    ifstream txt;
    txt.open(MAP_FILENAME);
    string line, key, value;
    unordered_map< string, vector<string> > mapping;

    while (getline(txt, line)) {
        if (line.size() > 3) {
            vector<string> v;
            for (string::size_type i = 0; i < line.size(); i+=3) {
                if (i > 0) {
                    value = "";
                    value.push_back(line[i]);
                    value.push_back(line[i+1]);
                    // cout << value;
                    v.push_back(value);
                }
                else {
                    key = "";
                    key.push_back(line[i]);
                    key.push_back(line[i+1]);
                    // cout << key;
                }
            }
            mapping[key] = v;
        }
    }
    txt.close();

    // read from text file
    txt.open(TEXT_FILENAME);
    string token;
    vector<string> candi;
    vector< vector<double> > prob;
    vector< vector<string> > path;
    int t;

    while (getline(txt, line)) {
        if (line.size() > 2) {
            t = 0;
            prob.clear();
            path.clear();
            string::size_type i = 0;
            
            while (i < line.size()) {
                if (line[i] == ' ')
                    i++;
                else {
                    token.clear();
                    token.push_back(line[i]);
                    token.push_back(line[i+1]);
                    candi = mapping[token];

                    if (t == 0) {
                        vector<double> p;
                        vector<string> s;
                        for (int k = 0; k < candi.size(); k++) {
                            s.push_back("<s> " + candi[k]);
                            p.push_back(getBigramProb(lm, voc, "<s>", candi[k].c_str()));
                        }
                        prob.push_back(p);
                        path.push_back(s);
                    }
                    else {
                        vector<double> p;
                        vector<string> s;
                        string prev;
                        double pk2j, p2j;
                        for (int j = 0; j < candi.size(); j++) {
                            double maxp = -10000000.0;
                            int maxk = 0;
                            for (int k = 0; k < path[t-1].size(); k++) {
                                prev = path[t-1][k].substr(path[t-1][k].size()-2, 2);
                                pk2j = getBigramProb(lm, voc, prev.c_str(), candi[j].c_str());
                                p2j = prob[t-1][k] + pk2j;
                                if (p2j > maxp) {
                                    maxp = p2j;
                                    maxk = k;
                                }
                            }
                            s.push_back(path[t-1][maxk] + " " + candi[j]);
                            p.push_back(maxp);
                        }
                        prob.push_back(p);
                        path.push_back(s);
                    }

                    i += 2;
                    t++;
                }
            }
            // find max-prob path
            double maxp = -10000000.0;
            int maxi = 0;
            string prev;
            double p2end;
            for (int i = 0; i < prob[t-1].size(); i++) {
                prev = path[t-1][i].substr(path[t-1][i].size()-2, 2);
                p2end = getBigramProb(lm, voc, prev.c_str(), "</s>");
                if ((prob[t-1][i] + p2end) > maxp) {
                    maxp = prob[t-1][i] + p2end;
                    maxi = i;
                }
            }
            cout << path[t-1][maxi] << " </s>" << endl;
        }
        // only run one line
        // break;
    }
    txt.close();

    return 0;
}

