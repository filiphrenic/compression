/**************************************************
 * lzwkoder -- program to encode input file using *
 *             LZW method                         *
 *                                                *
 * Author:  Filip Hrenić                          *
 *                                                *
 * Purpose:  TINF lab 2015/2016                   *
 *                                                *
 * Usage:                                         *
 *      lzwkoder input output                     *
 *          - input: input file                   *
 *          - output: output file                 *
 **************************************************/

#include <stdio.h>
#include <stdlib.h>

#define R (256)
#define ERR (-1)
#define WORD_CAPACITY (1<<3)
#define DICT_SIZE (1<<16)


/*
Trie has a root node
every node has children stored as a associative list (key:value node)
*/

typedef unsigned short trie_t; // value stored in trie
typedef unsigned char trie_key_t;
struct trie_node;

typedef struct trie_node {
	trie_t value;
	struct trie_node* children[R];
} trie_node;

typedef struct trie {
	trie_node* root;
	trie_t count;
} trie;

typedef struct string {
	trie_key_t* buffer;
	int capacity;
	int size;
} string;

// "constructors"

/*
	Creates a new trie node that has no children.
*/
trie_node* newNode(){
	int i;
	trie_node* tn = (trie_node*) malloc(sizeof(trie_node));
	tn->value = ERR;
	for( i=0; i<R; i++ ) tn->children[i] = NULL;
	return tn;
}

void destroyNode(trie_node* tn){
	int i;
	for(i=0;i<R;i++){
		trie_node* child = tn->children[i];
		if (child) destroyNode(child);
	}
	free(tn);
}

/*
	Creates a new trie that only has refference to the root node
*/
trie* newTrie(){
	trie* t = (trie*) malloc(sizeof(trie));
	t->root = newNode();
	t->count = 0;
	return t;
}

void destroyTrie(trie* t){
	destroyNode(t->root);
	free(t);
}

string* newString(){
	string* s = (string*) malloc(sizeof(string));
	s->capacity = WORD_CAPACITY;
	s->size = 0;
	s->buffer = (trie_key_t*) malloc(s->capacity * sizeof(trie_key_t));
	return s;
}

void destroyString(string* s){
	free(s->buffer);
	free(s);
}

void append(string* s, trie_t c){
	if (s->size == s->capacity){
		s->capacity *= 2;
		s->buffer = (trie_key_t*) realloc(s->buffer, s->capacity * sizeof(trie_key_t));
	}
	s->buffer[s->size++] = c;
}

// TRIE main functions

/*
	Searches the trie in order to find the value associated with the given word.
*/
trie_t search(trie* t, string* s){
	trie_node* curr = t->root;
	int i;
	for( i=0; i<s->size; i++ ){
		curr = curr->children[s->buffer[i]];
		if (curr == NULL) return ERR;
	}
	return curr->value;
}

/*
	Inserts the given word into the trie and associates given value with it.
*/
void insert(trie* t, string* s){
    if (t->count == DICT_SIZE - 1) return;
	trie_node* curr = t->root;
	int i;
	for( i=0; i<s->size; i++ ){
		if (!curr->children[s->buffer[i]])
			curr->children[s->buffer[i]] = newNode();
		curr = curr->children[s->buffer[i]];
	}
	curr->value = t->count++;
}

trie* initialize(){
	trie* t = newTrie();
	trie_key_t c = 0;
	do {
		t->root->children[c]        = newNode();
		t->root->children[c]->value = (trie_t) c;
	} while (++c);
	t->count = 1<<8;
	return t;
}

void iWrite(FILE* out, trie_t idx){
	fwrite(&idx, sizeof(trie_t), 1, out);
}

void encode(FILE* input, FILE* output){
	trie* t = initialize();
	trie_node* curr = t->root;
	trie_key_t novi_simbol;

	string* radna_rijec = newString();
	while (sizeof(trie_key_t) == fread(&novi_simbol, sizeof(trie_key_t), 1, input)) {
		trie_node* next = curr->children[novi_simbol];

		append(radna_rijec, novi_simbol);
		if (!next) {
			iWrite(output, curr->value);
			insert(t, radna_rijec);
			destroyString(radna_rijec);
			radna_rijec = newString();
			append(radna_rijec, novi_simbol);
			next = t->root->children[novi_simbol];
		}
		curr = next;
	};

	iWrite(output, curr->value);

	destroyString(radna_rijec);
	destroyTrie(t);
}


int main(int argc, char *argv[]){
	if (argc != 3){
		fprintf(stderr, "Have to provide input and output file.\nExample: %s input_file output_file\n", argv[0]);
		return 0;
	}

	FILE* input  = fopen(argv[1], "rb");
	FILE* output = fopen(argv[2], "wb");

	fprintf(stderr, "Encoding...\n");
	encode(input, output);
	fprintf(stderr, "Done!\n");

	fclose(input);
	fclose(output);

	return 0;
}
