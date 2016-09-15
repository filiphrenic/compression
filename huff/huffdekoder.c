/*****************************************************
 * huffdekoder -- program to decode input file using *
 *                Huffman coding                     *
 *                                                   *
 * Author:  Filip Hrenić                             *
 *                                                   *
 * Purpose:  TINF lab 2015/2016                      *
 *                                                   *
 * Usage:                                            *
 *      huffdekoder table input output               *
 *          - table: huffman table input file        *
 *          - input: input file                      *
 *          - output: output file                    *
 *****************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define R 256
#define BUFF 1<<4

typedef unsigned char huff_t;

typedef struct node_t {
    huff_t data; // only leafs
    struct node_t* left;
    struct node_t* right;
} Node;

typedef struct BinIn {
    FILE* in;
    int idx;
    huff_t buffer;
} BinIn;

BinIn* newBinIn(FILE* in){
    BinIn* b = (BinIn*) malloc(sizeof(BinIn));
    b->in = in;
    b->idx = 0;
    b->buffer = 0;
    return b;
}

int readBit(BinIn* b){
    int s = sizeof(b->buffer);
    if (b->idx == 0)
        if (!fread(&(b->buffer), s, 1, b->in)) return -1;
    int bit = ((1<<(8*s-1-b->idx)) & b->buffer) != 0;
    b->idx = (b->idx + 1) % (8*s);
    return bit;
}

Node* newNode(huff_t data){
    Node* node  = (Node*) malloc (sizeof(Node));
    node->data  = data;
    node->left  = NULL;
    node->right = NULL;
    return node;
}

Node* insertCode(Node* node, char* code, huff_t symbol){
    if (!*code)     return newNode(symbol);
    if (!node) node = newNode(0);
    if (*code=='0')	node->left  = insertCode(node->left,  ++code, symbol);
    else 			node->right = insertCode(node->right, ++code, symbol);
    return node;
}

Node* buildTrie(FILE* table){
    Node* root = newNode(0);
    huff_t idx = 0;
    do {
        char* buffer = (char*) malloc(R*sizeof(char));
        fscanf(table, "%s", buffer);
        root = insertCode(root, buffer, idx);
    } while(++idx);
    return root;
}

void decompress(FILE* input, FILE* output, Node* trie){

    long int size;
    fread(&size, sizeof(size), 1, input);

    BinIn* b = newBinIn(input);
    Node* curr = trie;
    while(size){
        int bit = readBit(b);
        if (bit) curr = curr->right;
        else     curr = curr->left;
        if (!curr) break;
        if (!curr->left && !curr->right){
            fwrite(&(curr->data), sizeof(curr->data), 1, output);
            curr = trie;
            size--;
        }
    }
}

int main(int argc, char *argv[]){
    if (argc != 4){
        fprintf(stderr, "Have to provide input, table and output file.\nExample: %s table_file input_file output_file\n", argv[0]);
        return 0;
    }

    FILE* table  = fopen(argv[1], "r");
    FILE* input  = fopen(argv[2], "rb");
    FILE* output = fopen(argv[3], "wb");

    fprintf(stderr, "Decompressing...\n");
    Node* trie = buildTrie(table);
    decompress(input, output, trie);
    fprintf(stderr, "Done!\n");

    fclose(input);
    fclose(table);
    fclose(output);

    return 0;
}
