#!/bin/bash

for i in {1..5}; do
    echo "Training model_0${i} with $1 iteration"
    ./train $1 model_init.txt seq_model_0${i}.txt model_0${i}.txt
done