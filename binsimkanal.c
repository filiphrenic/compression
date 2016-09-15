/*****************************************************
 * binsimkanal -- program to flip bits in input file *
 *                                                   *
 * Author:  Filip Hrenić                             *
 *                                                   *
 * Purpose:  TINF lab 2015/2016                      *
 *                                                   *
 * Usage:                                            *
 *      binsimkanal input error output               *
 *            - input: input file                    *
 *            - error: number [0,1]                  *
 *            - output: output file                  *
 *****************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

typedef struct BinStream {
    FILE* file;
    int idx;
    char buffer;
} BinStream;

BinStream* newBinStream(FILE* file){
    BinStream* b = (BinStream*) malloc(sizeof(BinStream));
    b->file = file;
    b->idx = 0;
    b->buffer = 0;
    return b;
}

int readBit(BinStream* in, int* bit){
    int s = sizeof(in->buffer);
    if (in->idx == 0)
        if (!fread(&(in->buffer), s, 1, in->file)) return 0;
    *bit = ((1<<(8*s-1 - in->idx)) & in->buffer) != 0;
    in->idx = (in->idx + 1) % (8*s);
    return 1;
}

void writeBit(BinStream* out, int bit){
    int s = sizeof(out->buffer);
    out->buffer *= 2;
    out->buffer += bit;
    if (++out->idx == 8 * s){
        out->idx = 0;
        fwrite(&(out->buffer), s, 1, out->file);
        out->buffer = 0;
    }
}

void flush(BinStream* out){
    if (!out->idx) return;
    int s = sizeof(out->buffer);
    while(out->idx){
        out->buffer *= 2;
        out->idx = (out->idx + 1) % (8*s);
    }
    fwrite(&(out->buffer), s, 1, out->file);
    out->buffer = 0;
}

int main(int argc, char *argv[]){
    if (argc != 4){
        fprintf(stderr, "Have to provide input and output file with error rate.\nExample: %s input_file error_rate output_file\n", argv[0]);
        return -1;
    }

    time_t t;
    srand((unsigned) time(&t));

    FILE* input = fopen(argv[1], "rb");
    FILE* output = fopen(argv[3], "wb");
    double e       = atof(argv[2]);

    if (e<0 || e>1){
        fprintf(stderr, "Error rate must be in range [0,1]\n");
        return -1;
    }

    fprintf(stderr, "Channeling...\n");
    BinStream* in  = newBinStream(input);
    BinStream* out = newBinStream(output);
    int bound      = e * RAND_MAX;

    int bit;
    while(readBit(in, &bit)){
        if (rand() < bound) bit ^= 1; // change bit
        writeBit(out, bit);
    }
    flush(out);

    fprintf(stderr, "Done!\n");

    fclose(input);
    fclose(output);

    return 0;
}
