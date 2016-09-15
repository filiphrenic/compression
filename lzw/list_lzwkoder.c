#include <stdio.h>
#include <stdlib.h>

#define ERR -1
#define WORD_CAPACITY 1<<3


/*
Trie has a root node
every node has children stored as a associative list (key:value node)
*/

typedef unsigned short trie_t; // value stored in trie
typedef unsigned char trie_key_t;
struct trie_node;

typedef struct trie_map_node {
	trie_key_t key;
	struct trie_node* value;
	struct trie_map_node* next;
} trie_map_node;

typedef struct trie_node {
	trie_t value;
	struct trie_map_node* children;
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
	trie_node* tn = (trie_node*) malloc(sizeof(trie_node));
	tn->value = ERR;
	tn->children = NULL;
	return tn;
}

void destroyNode(trie_node* tn){
	int i;
	trie_map_node* tmn = tn->children;
	while (tmn) {
		destroyNode(tmn->value);
		free(tmn);
		tmn = tmn->next;
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

// MAP functions

/*
	Searches for a child node that is stored under given key.
*/
trie_node* findChild(trie_node* parent, trie_key_t key){
	trie_map_node* tmn = parent->children;
	while (tmn) {
		if (tmn->key == key) return tmn->value;
		tmn = tmn->next;
	}
	return NULL;
}

/*
	First tries to find a child node that is associated under given key.
	If it finds one, than it returns that one (this allows not having duplicates)
	Otherwise, it creates a new child node and associates it under given key.
*/
trie_node* getOrCreateChild(trie_node* parent, trie_key_t key){
	trie_node* tn = findChild(parent, key);
	if (tn != NULL) return tn;

	trie_map_node* tmn = (trie_map_node*) malloc(sizeof(trie_map_node));
	tmn->key = key;
	tmn->value = newNode();
	tmn->next = parent->children;
	parent->children = tmn;

	return tmn->value;
}

// TRIE main functions

/*
	Searches the trie in order to find the value associated with the given word.
*/
trie_t search(trie* t, string* s){
	trie_node* curr = t->root;
	int i;
	for( i=0; i<s->size; i++ ){
		trie_node* next = findChild(curr, s->buffer[i]);
		curr = next;
		if (curr == NULL) return ERR;
	}
	return curr->value;
}

/*
	Inserts the given word into the trie and associates given value with it.
*/
void insert(trie* t, string* s, trie_t value){
	trie_node* curr = t->root;
	int i;
	for( i=0; i<s->size; i++ ){
		trie_node* next = getOrCreateChild(curr, s->buffer[i]);
		curr = next;
	}
	curr->value = value;
	t->count++;
}

trie* initialize(){
	trie* t = newTrie();
	trie_key_t c = 0;
	do {
		trie_node* tn = getOrCreateChild(t->root, c);
		tn->value     = (trie_t) c;
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
		trie_node* next = findChild(curr, novi_simbol);

		append(radna_rijec, novi_simbol); 
		if (!next) {
			iWrite(output, curr->value);
			insert(t, radna_rijec, t->count);
			destroyString(radna_rijec);
			radna_rijec = newString();
			append(radna_rijec, novi_simbol);
			next = findChild(t->root, novi_simbol);
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

