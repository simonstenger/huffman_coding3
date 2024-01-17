/*Definition of structs-------------------------------------------------------------------------*/
// Node of the huffman tree
typedef struct node{
    int value;
    char letter;
    int left, right;
} Node;

// Frequency of each character
typedef struct char_freqs{
    char character;
    int frequency;
} Frequency;

// Code structure to store the binary codes
typedef struct code{
    char letter;
    char* code;
    int length;
} Code;