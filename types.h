#ifndef TYPES_H // prevents the inclusion of  header file more than once
#define TYPES_H

/*Definition of structs-------------------------------------------------------------------------*/
// Node of the huffman tree
typedef struct node{
    int value; // Frequency of the character
    char letter; // Character
    int left, right; // Index of the left and right child
} Node;

// Frequency of each character
typedef struct frequency{
    char character; // Character
    int frequency; // Frequency of the character
} Frequency;

// Code structure to store the binary codes
typedef struct code{
    char letter; // Character
    char* code; // Binary code (as a string)
    int length; // Length of the binary code
} Code;

#endif // TYPES_H