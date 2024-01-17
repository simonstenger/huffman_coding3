//Huffman coding
// By Simon Stenger
// Date: 17/01/2023
// Version: 3.0

/*Description-----------------------------------------------------------------------------------*/
//Write later

/*Header----------------------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "types.h" //contains structure definitions for Frequency, Node and Code
#include "functions.h" //contains function declarations

#define INT_MAX 2147483647 //Used for initialisation in findMinNode function, max integer value
#define ASCII_SIZE 256 //Number of ASCII characters (ASCII Extended)

/*Main function---------------------------------------------------------------------------------*/
int main(){
    printf("-----------------------------Huffman (de-)compression-----------------------------");
    printf("\n");
    char filename[101];  //filename can be up to 100 characters long
    FILE *input, *output;

    /*get input details from user*/
    printf("Only .txt files are supported for compression and .bin files for decompression. Please ensure that the entered file path is no longer than 100 characters.\n");
    printf("The output file will be named output.bin for compression and output.txt for decompression. It will appear in the same directory as the input file.\n");
    printf("Type the name of the file to process:");
    scanf("%s",filename);
    int compress = getFileType(filename);

    switch (compress) {
        case 1:
            input = fopen(filename, "r");
            output = fopen("output.bin", "wb");
            compressFile(input, output);
            printf("-----------------------------Compression complete-----------------------------\n");
            break;
        case 2:
            input = fopen(filename, "rb");
            output = fopen("output.txt", "w");
            decompressFile(input, output);
            printf("-----------------------------Decompression complete-----------------------------\n");
            break;
        default:
            printf("Invalid input. Please use either a .txt or a .bin file\n");
            break;
    }

    fclose(input);
    fclose(output);

    return 0;
}

/*Compress and Decompress function definitions--------------------------------------------------------------------------*/
/*function to compress the input*/
void compressFile(FILE *input, FILE *output){
    printf("Compressing file");
    printf("\n");

    Frequency *characterFrequency0 = (Frequency *)calloc(ASCII_SIZE, sizeof(Frequency));
    countFrequencies(input, characterFrequency0);
    int new_size = countNonZero(characterFrequency0, ASCII_SIZE);
    printf("new size: %d\n", new_size);
    Frequency *characterFrequency = removeZeroElements(characterFrequency0, new_size);

    Node* tree = malloc(sizeof(Node) * new_size);
    int number_leaf_nodes;
    memcpy(&number_leaf_nodes, &new_size, sizeof(int));
    tree = buildHuffmanTree(tree, &new_size, characterFrequency);
    Code *codeTable = malloc(ASCII_SIZE * sizeof(Code));

    // Initialize the code table
    for (int i = 0; i < ASCII_SIZE; ++i) {
        codeTable[i].letter = 0;
        codeTable[i].code = NULL;
        codeTable[i].length = 0;
    }

    char currentCode[50] = {0};
    buildCodeTable(tree, new_size-1 , codeTable, currentCode, 0);

    //write the codeTable into the header of the output file
    writeCodeTable2FileBinary(output, codeTable, number_leaf_nodes);
    //loop over characters in input file, take the code from the codeTable and write it to the output file
    char c;
    while ((c = fgetc(input)) != EOF) {
        writeBinaryString2File(output, c, codeTable, new_size);
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

    // Clean up memory for all variables
    freeHuffmanTree(tree);
    for (int i = 0; i < ASCII_SIZE; ++i) {
        free(codeTable[i].code);
    }
    free(codeTable);
    free(characterFrequency0);
    free(characterFrequency);

    summarizeCompression(input, output);

    return;
}

/*function to decompress the input*/ //DOES NOT WORK YET
void decompressFile(FILE *input, FILE *output) {
    printf("Decompressing file\n");
    //reconstruct code table from file
    int tableSize = 0;
    Code* codeTable = reconstructCodeTableFromFileBinary(input, &tableSize);
    //read binary string from file
    decodeBinaryFile(input, output, codeTable, tableSize);

    // Clean up memory for all variables
        for (int i = 0; i < ASCII_SIZE; ++i) {
            free(codeTable[i].code);
        }
        free(codeTable);
}

/*Functions required for compression---------------------------------------------------------------------------------------*/
void countFrequencies(FILE *file, Frequency *characterFrequency) 
{    
    int c;
    while ((c = fgetc(file)) != EOF) {
        characterFrequency[c].character = c;
        characterFrequency[c].frequency++;
    }
    rewind(file);
}

/*replace sorted array with one excluding the zeros*/
Frequency *removeZeroElements(Frequency *characterFrequency, int new_size)
{
    // Allocate memory for the new array
    Frequency *new_characterFrequency = (Frequency *)malloc(new_size * sizeof(Frequency));
    // Copy non-zero elements to the new array
    int j = 0;
    for (int i = 0; i < ASCII_SIZE; i++)
    {
        if (characterFrequency[i].frequency != 0)
        {
            new_characterFrequency[j++] = characterFrequency[i];
        }
    }
    return new_characterFrequency;
}

/*count non-zero elements*/
int countNonZero(Frequency *characterFrequency, int size) {
    int count = 0;
    for (int i = 0; i < size; i++) {
        if (characterFrequency[i].frequency != 0) {
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
Node* buildHuffmanTree(Node *tree, int *new_size, Frequency *characterFrequency){
    int i = 0;
    int nodes_left = *new_size;
    int smallOne,smallTwo;

    for (i=0;i<*new_size;i++){
        tree[i].value = characterFrequency[i].frequency;
        tree[i].letter = characterFrequency[i].character;
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
        tree[*new_size].letter=-1;
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
    // Write int tableSize to the file
    fwrite(&tableSize, sizeof(char), 1, file);

    for (int i = 0; i < ASCII_SIZE; ++i) {
        if (codeTable[i].code != NULL) {
            // Write the letter
            fwrite(&codeTable[i].letter, sizeof(char), 1, file);
            // Write the length of the code
            fwrite(&codeTable[i].length, sizeof(char), 1, file); //limits the code length to 255 bits, cuz sizeof(char)
            // Write the code in actual binary format
            fwrite(codeTable[i].code, sizeof(char), codeTable[i].length, file);
        }
    }
}

void writeBinaryString2File(FILE* file, char index, Code* codeTable, int new_size) {
    static unsigned char byte = 0; // Make byte static to retain its value across function calls
    static int bitIndex = 7; // Make bitIndex static to retain its value across function calls

    const char* binaryString = codeTable[index].code;
    int length = codeTable[index].length;

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

    // If this is the last character, write the remaining bits to the file
    if (index == new_size - 1 && bitIndex != 7) {
        fwrite(&byte, sizeof(unsigned char), 1, file);
    }
}

/*Functions required for decompression---------------------------------------------------------------------------------------*/
// Reconstruct code table from binary file
Code* reconstructCodeTableFromFileBinary(FILE* file, int* tableSize) {
    // Read code table size
    if (fread(tableSize, sizeof(char), 1, file) < 1) {
        fprintf(stderr, "Error reading code table size from file\n");
        return NULL;  // Return NULL on error
    }

    // Allocate memory for the code table
    Code* codeTable = malloc((*tableSize) * sizeof(Code));

    // Read code table entries
    for (int i = 0; i < (*tableSize); ++i) {
        // Read the letter
        fread(&codeTable[i].letter, sizeof(char), 1, file);

        // Read the length of the code
        int length;
        fread(&length, sizeof(char), 1, file);

        // Read the code
        codeTable[i].code = malloc(length + 1);
        fread(codeTable[i].code, sizeof(char), length, file);
        codeTable[i].code[length] = '\0';  // Null-terminate the string

        codeTable[i].length = length;
    }

    return codeTable;
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
                    printf("%c", codeTable[j].letter);

                    // Remove the processed bits from the buffer
                    bufferLength -= codeTable[j].length;
                    buffer &= ~(0xFFFFFFFF << bufferLength);
                    break;
                }
            }
        }
    }
}

/*Other functions------------------------------------------------------------------------------------------------------------*/
int getFileType(char* filename) {
    if (strstr(filename, ".txt") != NULL) {
        return 1; // .txt file
    }
    else if (strstr(filename, ".bin") != NULL) {
        return 2; // .bin file
    }
    else {
        return 0; // unknown file type
    }
}

//summarize compression
void summarizeCompression(FILE *input, FILE *output) {
    //calculate input file size
    rewind(input);
    fseek(input, 0L, SEEK_END);
    int originalBits = 8* ftell(input);
    //calculate output file size
    rewind(output);
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
}