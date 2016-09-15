/***************************************************
 * huffkoder -- program to encode input file using *
 *              Huffman coding                     *
 *                                                 *
 * Author:  Filip Hrenić                           *
 *                                                 *
 * Purpose:  TINF lab 2015/2016                    *
 *                                                 *
 * Usage:                                          *
 *      huffkoder input table output               *
 *          - input: input file                    *
 *          - table: huffman table output file     *
 *          - output: output file                  *
 ***************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define R 256
#define BUFF 1<<4

typedef unsigned char huff_t;
typedef unsigned int huff_freq_t;

typedef struct node_t {
    huff_t data; // only leafs
    huff_freq_t freq;
    struct node_t* left;
    struct node_t* right;
} Node;

typedef struct minpq_t {
    int size;
    int capacity;
    struct node_t** array;
} MinPQ;

typedef struct BinOut {
    FILE* out;
    int idx;
    huff_t buffer;
} BinOut;

void write(BinOut* b, char* code){
    int s = sizeof(b->buffer);
    for( ; *code; code++ ){
        b->buffer *= 2;
        b->buffer += *code=='1';

        if (++b->idx == 8 * s){
            b->idx = 0;
            fwrite(&(b->buffer), s, 1, b->out);
            b->buffer = 0;
        }
    }
}

void flush(BinOut* b){
    if (!b->idx) return;
    int s = sizeof(b->buffer);
    while(b->idx){
        b->buffer *= 2;
        b->idx = (b->idx + 1) % (8*s);
    }
    fwrite(&(b->buffer), s, 1, b->out);
    b->buffer = 0;
}

Node* newNode(huff_t data, huff_freq_t freq){
    Node* node  = (Node*) malloc (sizeof(Node));
    node->data  = data;
    node->freq  = freq;
    node->left  = NULL;
    node->right = NULL;
    return node;
}

Node* merge(Node* left, Node* right){
    Node* parent  = (Node*) malloc (sizeof(Node));
    parent->data  = 0;
    parent->left  = left;
    parent->right = right;
    return parent;
}

// priority queue functions

MinPQ* createMinPQ(int capacity){
    MinPQ* pq = (MinPQ*) malloc(sizeof(MinPQ));
    pq->size = 0;
    pq->capacity = capacity;
    pq->array = (Node**) malloc (capacity * sizeof(Node*));
    return pq;
}

int less(MinPQ* pq, int i, int j){
    return pq->array[i]->freq < pq->array[j]->freq;
}

void exch(MinPQ* pq, int i, int j){
    Node* tmp = pq->array[i];
    pq->array[i] = pq->array[j];
    pq->array[j] = tmp;
}

void heapify(MinPQ* pq, int idx){
    int child;
    while ( (child = 2*idx+1) < pq->size ){
        if (child+1 < pq->size && less(pq, child+1, child)) child++;
        if (!less(pq, child, idx)) break;
        exch(pq, idx, child);
        idx = child;
    }
}

Node* extractMin(MinPQ* pq){
    if (pq->size==0) return NULL;
    pq->size--;
    Node* min = pq->array[0];
    exch(pq, 0, pq->size);
    heapify(pq, 0);
    return min;
}

void insert(MinPQ* pq, Node* node){
    int idx = pq->size++;
    pq->array[idx] = node;
    while (idx) {
        huff_t parent = idx / 2;
        if (!less(pq, idx, parent)) break;
        exch(pq, idx, parent);
        idx = parent;
    }
}

Node* buildTrie(huff_freq_t* freqs){
    MinPQ* pq = createMinPQ(R);
    huff_t c=0;

    // TODO, try first fill then sort
    do insert(pq, newNode(c, freqs[c]));
    while (++c);

    while (pq->size != 1){
        Node* left = extractMin(pq);
        Node* right = extractMin(pq);
        Node* parent = merge(left,right);
        insert(pq, parent);
    }

    Node* trie = extractMin(pq);
    return trie;
}

char* str(char* s, int len){
    char* ret = (char*) malloc((len+1)*sizeof(char));
    int i;
    for(i=0;i<len;i++) ret[i] = s[i];
    ret[len] = '\0';
    return ret;
}

void createCodes(Node* node, char* tmp, int level, char* codes[R]){
    if (node->left) {
        tmp[level] = '0';
        createCodes(node->left,  tmp, level+1, codes);
    }

    if (node->right) {
        tmp[level] = '1';
        createCodes(node->right, tmp, level+1, codes);
    }

    if (!node->left && !node->right){ // leaf
        codes[node->data] = str(tmp,level);
    }

}

huff_freq_t* findFrequencies(FILE* in){
    huff_freq_t* freqs = (huff_freq_t*) malloc(R * sizeof(huff_freq_t));
    int i;
    for( i=0; i<R; i++ ) freqs[i] = 0;

    huff_t symbol;
    while( fread(&symbol, sizeof(symbol), 1, in) )
        freqs[symbol]++;

    return freqs;
}

void compress(FILE* input, FILE* output, char* codes[R]){
    huff_t symbol;
    BinOut* b = (BinOut*) malloc(sizeof(BinOut));
    b->out = output;
    b->idx = 0;
    while(fread(&symbol, sizeof(huff_t), 1, input))
        write(b, codes[symbol]);
    flush(b);
    free(b);
}

void writeCode(FILE* table, char* code){
    fprintf(table, "%s\n", code);
}

char** getCodes(FILE* input){
    huff_freq_t* freqs = findFrequencies(input);
    Node* trie = buildTrie(freqs);
    char** codes = (char**) malloc(R*sizeof(char*)) ;
    char tmp[R];
    createCodes(trie, tmp, 0, codes);
    return codes;
}

int main(int argc, char *argv[]){
    if (argc != 4){
        fprintf(stderr, "Have to provide input, table and output file.\nExample: %s input_file table_file output_file\n", argv[0]);
        return 0;
    }

    FILE* input  = fopen(argv[1], "rb");
    FILE* table  = fopen(argv[2], "w");
    FILE* output = fopen(argv[3], "wb");

    fprintf(stderr, "Compressing...\n");
    char** codes = getCodes(input);
    int i;
    for(i=0;i<R;i++) writeCode(table, codes[i]);
    long int size = ftell(input);
    fseek(input, 0L, SEEK_SET);
    fwrite(&size, sizeof(size), 1, output);
    compress(input, output, codes);
    fprintf(stderr, "Done!\n");

    for(i=0;i<R;i++) free(codes[i]);
    fclose(input);
    fclose(table);
    fclose(output);

    return 0;
}
