/****************************************************
 * lzwdekoder -- program to decode input file using *
 *               LZW method                         *
 *                                                  *
 * Author:  Filip Hrenić                            *
 *                                                  *
 * Purpose:  TINF lab 2015/2016                     *
 *                                                  *
 * Usage:                                           *
 *      lzwdekoder input output                     *
 *          - input: input file                     *
 *          - output: output file                   *
 ****************************************************/

#include <stdio.h>
#include <stdlib.h>

#define R (1<<8)
#define WORD_CAPACITY (1<<3)
#define DICT_CAPACITY (1<<16)

typedef unsigned short trie_t; // value stored in trie
typedef unsigned char trie_key_t;

typedef struct string {
	trie_key_t* buffer;
	int capacity;
	int size;
} string;

string* _newString(int capacity){
	string* s = (string*) malloc(sizeof(string));
	s->capacity = capacity;
	s->size = 0;
	s->buffer = (trie_key_t*) malloc(capacity * sizeof(trie_key_t));
	return s;
}

string* newString() { return _newString(WORD_CAPACITY); }

void destroyString(string* s){
	free(s->buffer);
	free(s);
}

void append(string* s, trie_key_t c){
	if (s->size == s->capacity){
		s->capacity *= 2;
		s->buffer = (trie_key_t*) realloc(s->buffer, s->capacity * sizeof(trie_key_t));
	}
	s->buffer[s->size++] = c;
}

string* concatFirst(string* s1, string* s2){
	string* s = _newString(s1->size + 1);
	int idx;
	for(idx=0;idx<s1->size;idx++) append(s, s1->buffer[idx]);
	append(s, s2->buffer[0]);
	return s;
}

void print(string* s){
	int i;
	for(i=0;i<s->size;i++) printf("%c", s->buffer[i]);
	printf("\n");
}

void sWrite(FILE* out, string* s){
	fwrite(s->buffer, sizeof(trie_key_t), s->size, out);
}

void decode(FILE* input, FILE* output){
	// create dictionary
	string* dictionary[DICT_CAPACITY];
	trie_key_t c = 0;
	do {
		string* sim = newString();
		append(sim, c);
		dictionary[c] = sim;
	} while (++c);
	trie_t dictSize = R;

	trie_t dictIdx;
	string* radna_rijec = newString();
	if (!fread(&dictIdx, sizeof(trie_t), 1, input)) return;
	radna_rijec = concatFirst(radna_rijec, dictionary[dictIdx]);
	sWrite(output, radna_rijec);


    int cnt = 0;
	while (fread(&dictIdx, sizeof(trie_t), 1, input)) {

		string* nova_rijec;
		if (dictIdx < dictSize){
			nova_rijec = dictionary[dictIdx];
		} else {
			nova_rijec = concatFirst(radna_rijec, radna_rijec);
		}

		sWrite(output, nova_rijec);

        if (dictSize != (DICT_CAPACITY-1)) {
            dictionary[dictSize++] = concatFirst(radna_rijec, nova_rijec);
        }


		radna_rijec = nova_rijec;

	};

	// destroy
	do {
		destroyString(dictionary[dictSize-1]);
	} while(--dictSize);
}


int main(int argc, char *argv[]){
	if (argc != 3){
		fprintf(stderr, "Have to provide input and output file.\nExample: %s input_file output_file\n", argv[0]);
		return 0;
	}

	FILE* input  = fopen(argv[1], "rb");
	FILE* output = fopen(argv[2], "wb");

	fprintf(stderr, "Decoding...\n");
	decode(input, output);
	fprintf(stderr, "Done!\n");

	fclose(input);
	fclose(output);

	return 0;
}
