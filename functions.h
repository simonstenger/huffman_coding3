#ifndef FUNCTIONS_H // prevents the inclusion of  header file more than once
#define FUNCTIONS_H

#include <stdio.h> // Has to be included to use FILE
#include "types.h" // Has to be included to use Frequency, Node and Code

/*Function declarations--------------------------------------------------------------------------*/
//Function to count the frequency of each character in the input file
void countFrequencies(FILE *file, Frequency *char_frequency);
//Function to remove all elements with a frequency of 0 from the array
Frequency *removeZeroElements(Frequency *char_frequency, int new_size);
//Function to count the number of non-zero elements in the array -> number of leaf nodes in the tree
int countNonZero(Frequency *char_frequency, int size);
//Function to get file type from name and decide to compress or decompress
int getFileType(char* filename);
//Function to summarize compression statistics
void summarizeCompression(FILE* input, FILE* output);
//Function to find the smallest node in an array of nodes
int findMinNode(Node* tree, int excludedIndex, int size);
//Function to build the Huffman tree
Node* buildHuffmanTree(Node *tree, int *new_size, Frequency *char_frequency);
//Function to build the code table containing character, code and code length
void buildCodeTable(Node* tree, int index, Code* codeTable, char* currentCode, int depth);
//Function to write the code table and its size to the header of a binary output file
void writeCodeTable2FileBinary(FILE* file, Code* codeTable, int tableSize);
//Function to reconstruct code table from binary file
Code* reconstructCodeTableFromFileBinary(FILE* file, int* tableSize);
//Function to write a binary string to a file
void writeBinaryString2File(FILE* file, char index, Code* codeTable, int new_size);
//Function to decode a binary file using Huffman coding table
void decodeBinaryFile(FILE *input, FILE *output, Code *codeTable, int tableSize);
//Function to compress the input file
void compressFile(FILE *input, FILE *output);
//Function to decompress the input file
void decompressFile(FILE *input, FILE *output);

#endif