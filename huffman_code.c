//Huffman coding
// By Simon Stenger
// Date: 04/01/2023
// Version: 2.0

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define INT_MAX 2147483647

// Node of the huffman tree
struct node{
    int value;
    char letter;
    int left, right;
};

struct char_freqs{
    char character;
    int frequency;
};

// Code structure to store the binary codes
struct code{
    char letter;
    char* code;
    int length;
};

typedef struct node Node;
typedef struct char_freqs Frequency;
typedef struct code Code;

/*Read input file into variable in a function*/
void readFile(char *filename, char *input) /*inputs: pointer to filename array, pointer to input array*/
{
    FILE *fp;
    char c;
    int i = 0;

    fp = fopen(filename, "r");
    if (fp == NULL)
    {
        printf("Error opening file\n");
        exit(1);
    }

    while ((c = fgetc(fp)) != EOF) //copy characters into input array until end of file is reached
    {
        input[i] = c;
        i++;
    }
    input[i] = '\0';
    fclose(fp);
}

void countFrequencies(char *initial, Frequency *char_frequency) 
{    
    for (int i = 0; initial[i] != '\0'; i++)
    {
        char_frequency[(unsigned char)initial[i]].character = initial[i]; /*assign character to huffman node*/
        char_frequency[(unsigned char)initial[i]].frequency++; /*update frequency of huffman node*/
    }
}

/*replace sorted array with one excluding the zeros*/
Frequency *removeZeroElements(Frequency *char_frequency, int new_size)
{
    // Allocate memory for the new array
    Frequency *new_char_frequency = (Frequency *)malloc(new_size * sizeof(Frequency));
    // Copy non-zero elements to the new array
    int j = 0;
    for (int i = 0; i < 128; i++)
    {
        if (char_frequency[i].frequency != 0)
        {
            new_char_frequency[j++] = char_frequency[i];
        }
    }
    free(char_frequency);
    return new_char_frequency;
}

/*count non-zero elements*/
int count_non_zero(Frequency *char_frequency, int size) {
    int count = 0;
    for (int i = 0; i < size; i++) {
        if (char_frequency[i].frequency != 0) {
            count++;
        }
    }
    return count;
}

void freeHuffmanTree(Node* tree) {
    if (tree == NULL) {
        return;
    }
    free(tree);
}

/*finds the smallest node in the array*/
int findMinNode(Node* tree, int excludedIndex, int size) {
    int minIndex = -1;
    int minValue = INT_MAX;

    for (int i = 0; i < size; ++i) {
        // Ignore nodes with a value of -1 or the excluded index
        if (tree[i].value != -1 && i != excludedIndex) {
            if (tree[i].value < minValue) {
                minValue = tree[i].value;
                minIndex = i;
            }
        }
    }

    return minIndex;
}

/*builds the huffman tree and returns its address by reference*/
Node* buildHuffmanTree(Node *tree, int *new_size, Frequency *char_frequency){
    int i = 0;
    int nodes_left = *new_size;
    int smallOne,smallTwo;

    for (i=0;i<*new_size;i++){
        tree[i].value = char_frequency[i].frequency;
        tree[i].letter = char_frequency[i].character;
        tree[i].left = -1;
        tree[i].right = -1;

        printf("Node Frequency %d  ", tree[i].value);
        printf("Node Character %c  ", tree[i].letter);
        printf("Number of nodes so far %d\n", i);
    }

    while (nodes_left>1){
        smallOne=findMinNode(tree,-1, *new_size);
        smallTwo=findMinNode(tree,smallOne, *new_size);

        tree = realloc(tree, sizeof(Node) * ((*new_size)+1));

        tree[*new_size].value=tree[smallOne].value+tree[smallTwo].value;
        tree[*new_size].letter=129;
        tree[*new_size].left=smallTwo;
        tree[*new_size].right=smallOne;
        tree[smallOne].value=-1;
        tree[smallTwo].value=-1;
        nodes_left--;
        (*new_size)++;
    }
    printf("Huffman tree built\n");
    return tree;
}

// Function to build the code table
void buildCodeTable(Node* tree, int index, Code* codeTable, char* currentCode, int depth) {
    if (tree == NULL) {
        return;
    }

    // If the node is a leaf, assign the binary code to the corresponding character
    int LeftIndex = tree[index].left;
    if (tree[LeftIndex].left == -1 && tree[LeftIndex].right == -1) {
        currentCode[depth] = '0';
        codeTable[tree[LeftIndex].letter].letter = tree[LeftIndex].letter;
        codeTable[tree[LeftIndex].letter].code = malloc((depth+1)*sizeof(char));
        memcpy(codeTable[tree[LeftIndex].letter].code, currentCode, (depth+1)*sizeof(char));
        codeTable[tree[LeftIndex].letter].length = depth+1;
        
    }
    else {
        currentCode[depth] = '0';
        buildCodeTable(tree, LeftIndex, codeTable, currentCode, depth + 1);
    }

    int RightIndex = tree[index].right;
    if (tree[RightIndex].right == -1 && tree[RightIndex].left == -1) {
        currentCode[depth] = '1';
        codeTable[tree[RightIndex].letter].letter = tree[RightIndex].letter;
        codeTable[tree[RightIndex].letter].code = malloc((depth+1)*sizeof(char));
        memcpy(codeTable[tree[RightIndex].letter].code, currentCode, (depth+1)*sizeof(char));
        codeTable[tree[RightIndex].letter].length = depth+1;
    }
    else {
        currentCode[depth] = '1';
        buildCodeTable(tree, RightIndex, codeTable, currentCode, depth + 1);
    }
}

// Function to write the code table and its size to the header of a binary output file
void writeCodeTable2FileBinary(FILE* file, Code* codeTable, int tableSize) {
    // Write the size of the table
    fwrite(&tableSize, sizeof(int), 1, file);

    for (int i = 0; i < 128; ++i) {
        if (codeTable[i].code != NULL) {
            // Write the letter
            fwrite(&codeTable[i].letter, sizeof(char), 1, file);

            // Write the length of the code
            int length = strlen(codeTable[i].code);
            fwrite(&length, sizeof(int), 1, file);

            // Write the code
            fwrite(codeTable[i].code, sizeof(char), length, file);
        }
    }
}

// Reconstruct code table from binary file
Code* reconstructCodeTableFromFileBinary(FILE* file, Code* codeTable, int tableSize) {
    // Read code table size
    if (fread(&tableSize, sizeof(int), 1, file) != 1) {
        fprintf(stderr, "Error reading code table size from file\n");
        return NULL;  // Return NULL on error
    }

    // Read code table entries
    for (int i = 0; i < tableSize; ++i) {
        // Read the letter
        fread(&codeTable[i].letter, sizeof(char), 1, file);

        // Read the length of the code
        int length;
        fread(&length, sizeof(int), 1, file);

        // Read the code
        codeTable[i].code = malloc(length + 1);
        fread(codeTable[i].code, sizeof(char), length, file);
        codeTable[i].code[length] = '\0';  // Null-terminate the string

        codeTable[i].length = length;
    }

    return codeTable;
}


void writeBinaryString2File(FILE* file, char index, Code* codeTable) {
    const char* binaryString = codeTable[index].code;
    int length = codeTable[index].length;

    unsigned char byte = 0;
    int bitIndex = 7; // Start from the most significant bit

    for (int i = 0; i < length; ++i) {
        if (binaryString[i] == '1') {
            byte |= (1 << bitIndex);
        }
        // Move to the next bit
        bitIndex--;

        // Write the byte to the file when all bits are set
        if (bitIndex < 0) {
            fwrite(&byte, sizeof(unsigned char), 1, file);
            byte = 0;
            bitIndex = 7;
        }
    }

    // If there are remaining bits in the last byte, write it to the file
    if (bitIndex != 7) {
        fwrite(&byte, sizeof(unsigned char), 1, file);
    }
}

// Function to decode a binary file using Huffman coding table
void decodeBinaryFile(FILE *input, FILE *output, Code *codeTable, int tableSize) {

    if (input == NULL || output == NULL) {
        printf("Error opening files\n");
        exit(1);
    }

    int buffer = 0; // Buffer to store bits
    int bufferLength = 0; // Number of bits in the buffer

    while (1) {
        // Read one byte from the binary file
        unsigned char byte;
        if (fread(&byte, 1, 1, input) == 0) {
            break; // Reached EOF
        }

        // Process each bit in the byte
        for (int i = 7; i >= 0; i--) {
            // Append the current bit to the buffer
            buffer = (buffer << 1) | ((byte >> i) & 1);
            bufferLength++;

            // Check for a match in the Huffman coding table
            for (int j = 0; j < tableSize; j++) {
                if (bufferLength == codeTable[j].length && buffer == strtol(codeTable[j].code, NULL, 2)) {
                    // Match found, print the letter to the output file
                    fprintf(output, "%c", codeTable[j].letter);

                    // Remove the processed bits from the buffer
                    bufferLength -= codeTable[j].length;
                    buffer &= ~(0xFFFFFFFF << bufferLength);
                    break;
                }
            }
        }
    }
}  

/*function to compress the input*/
void compressFile(FILE *input, FILE *output, Code* codeTable, int new_size){
    printf("Compressing file");
    printf("\n");
    //write the codeTable into the header of the output file
    writeCodeTable2FileBinary(output, codeTable, new_size);
    //loop over characters in input file, take the code from the codeTable and write it to the output file
    char c;
    while ((c = fgetc(input)) != EOF) {
        writeBinaryString2File(output, c, codeTable);
        // int length = strlen(codeTable[c].code);
        printf("Length of code: %d", codeTable[c].length);
        printf("\n");
        printf("Character: %c", c);
        printf("\n");
        printf("Code: %.*s", codeTable[c].length, codeTable[c].code);
        printf("\n");
    }
    if (c == EOF) {
        //write EOF to the output file
        fprintf(output, "%c", EOF);
    }
    
    //calculate input file size
    fseek(input, 0L, SEEK_END);
    int originalBits = 8* ftell(input);
    //calculate output file size
    fseek(output, 0L, SEEK_END);
    int compressedBits = 8* ftell(output);
    /*print details of compression on the screen*/
    fprintf(stderr,"Original bits = %d",originalBits*8);
    printf("\n");
    fprintf(stderr,"Compressed bits = %d",compressedBits);
    printf("\n");
    fprintf(stderr,"Compression ratio of %.2f%%",(((float)((float)compressedBits)/(float)(originalBits*8))*100));
    printf("\n");
    fprintf(stderr,"Saved %.2f%% of memory",(((float)(originalBits*8)-(float)compressedBits)/(float)(originalBits*8))*100);
    printf("\n");

    return;
}

/*function to decompress the input*/ //DOES NOT WORK YET
void decompressFile(FILE *input, FILE *output, Code* codeTable) {
    printf("Decompressing file\n");
    //reconstruct code table from file
    int tableSize = 0;
    codeTable = reconstructCodeTableFromFileBinary(input, codeTable, tableSize);
    //read binary string from file
    decodeBinaryFile(input, output, codeTable, tableSize);
}

int main(){
    printf("-----------------------------Huffman compression-----------------------------");
    printf("\n");
    int compress;
    char filename[20];
    FILE *input, *output;

    /*get input details from user*/
    printf("Type the name of the file to process:");
    scanf("%s",filename);
    printf("Type 1 to compress and 2 to decompress:");
    scanf("%d",&compress);

    if (compress==1){
        input = fopen(filename, "r");
        output = fopen("output.bin","wb");
        Frequency *char_frequency0 = (Frequency *)calloc(128, sizeof(Frequency));
        char initial[3000];
        readFile(filename, initial);
        countFrequencies(initial, char_frequency0);
        int new_size = count_non_zero(char_frequency0, 128);
        printf("new size: %d\n", new_size);
        Frequency *char_frequency = removeZeroElements(char_frequency0, new_size);

        Node* tree = malloc(sizeof(Node) * new_size);
        int number_leaf_nodes;
        memcpy(&number_leaf_nodes, &new_size, sizeof(int));
        tree = buildHuffmanTree(tree, &new_size, char_frequency);
        Code *codeTable = malloc(128 * sizeof(Code));

        // Initialize the code table
        for (int i = 0; i < 128; ++i) {
            codeTable[i].letter = 0;
            codeTable[i].code = NULL;
            codeTable[i].length = 0;
        }

        char currentCode[50] = {0};
        buildCodeTable(tree, new_size-1 , codeTable, currentCode, 0);
        compressFile(input, output, codeTable, number_leaf_nodes);

        // Clean up memory for all variables
        freeHuffmanTree(tree);
        for (int i = 0; i < 128; ++i) {
            free(codeTable[i].code);
        }
        free(codeTable);       
    }
    else{
        input = fopen(filename, "rb");
        output = fopen("output.txt","w");
        Code* codeTable = malloc(128 * sizeof(Code));
        decompressFile(input,output, codeTable);

        // Clean up memory for all variables
        for (int i = 0; i < 128; ++i) {
            free(codeTable[i].code);
        }
        free(codeTable);
    }

    fclose(input);
    fclose(output);

    return 0;
}