//Huffman coding
// By Simon Stenger
// Date: 21/01/2023
// Version: 4.0

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
    FILE *input, *output; //create pointers to input and output files

    printf("Only .txt files are supported for compression and .bin files for decompression. Please ensure that the entered file path is no longer than 100 characters.\n"); //explain to user what to do
    printf("The output file will be named output.bin for compression and output.txt for decompression. It will appear in the same directory as the input file.\n"); //explain to user what to do
    printf("Type the name of the file to process:");
    scanf("%s",filename); //get filename (or path) from user

    if (strlen(filename)>100){ //check the length of the filename/filepath
        printf("Filename too long. Please try again.");
        exit(1); //exit the program if filename too long
    }
    int compress = getFileType(filename); //check if file is .txt or .bin, returns 1 for .txt, 2 for .bin, 0 for unknown file type

    switch (compress) { //switch between compression and decompression dependent on file type
        case 1: //if .txt file -> compress
            input = fopen(filename, "r"); //open input file in read mode
            output = fopen("output.bin", "wb"); //open output file in binary write mode
            compressFile(input, output); //call compress function
            printf("\n-----------------------------Compression complete-----------------------------\n");
            break; //break out of switch statement
        case 2: //if .bin file -> decompress
            input = fopen(filename, "rb"); //open input file in binary read mode
            output = fopen("output.txt", "w"); //open output file in write mode
            decompressFile(input, output); //call decompress function
            printf("\n-----------------------------Decompression complete-----------------------------\n");
            break;
        default: //if unknown file type -> error message
            printf("Invalid input. Please use either a .txt or a .bin file\n");
            break; //break out of switch statement
    }

    fclose(input); //close input file
    fclose(output); //close output file

    return 0; //return 0 to indicate successful execution
}

/*Compress and Decompress function definitions--------------------------------------------------------------------------*/
/*function to compress the input .txt file and give out a binary output file*/
void compressFile(FILE *input, FILE *output){
    printf("Compressing file");
    printf("\n");

    Frequency *characterFrequency0 = (Frequency *)calloc(ASCII_SIZE, sizeof(Frequency)); //allocate memory for array of frequencies. struct Frequency is defined in types.h
    countFrequencies(input, characterFrequency0); //count the frequency of each character in the input file
    int size = countNonZero(characterFrequency0, ASCII_SIZE); //count the number of non-zero elements in the array, size becomes very important later on
    Frequency *characterFrequency = removeZeroElements(characterFrequency0, size); //remove all elements with a frequency of 0 from the array

    Node* tree = malloc(sizeof(Node) * (size+1)); //allocate memory for the Huffman tree, size+1 to asccount for EOF character
    int NumberLeafNodes;
    memcpy(&NumberLeafNodes, &size, sizeof(int)); //copy the value of size into NumberLeafNodes, used to reconstruct the tree later on, value of size is changed in buildHuffmanTree function
    tree = buildHuffmanTree(tree, &size, characterFrequency); //build the Huffman tree and return its address

    Code *codeTable = malloc(ASCII_SIZE * sizeof(Code)); //allocate memory for the code table
    for (int i = 0; i < ASCII_SIZE; ++i) { //initialise code table
        codeTable[i].letter = 0;
        codeTable[i].code = NULL;
        codeTable[i].length = 0;
    }
    char currentCode[ASCII_SIZE] = {0}; //initialise currentCode array to 0
    buildCodeTable(tree, size-1 , codeTable, currentCode, 0); //build the code table containing character, code and code length for all ASCII characters
    writeCodeTable2FileBinary(output, codeTable, NumberLeafNodes); //write the codeTable into the header of the output file

    char c;
    while ((c = fgetc(input)) != EOF) { //read file character by character until EOF is reached
        writeBinaryString2File(output, c, codeTable, size); //write the binary code of the character to the output file
    }
    writeBinaryString2File(output, EOF, codeTable, size); // Write EOF character to the output file when all characters are processed

    // Clean up memory for all variables
    //free(tree);
    //for (int i = 0; i < ASCII_SIZE; ++i) {
    //    free(codeTable[i].code);
    //}
    //free(codeTable);
    free(characterFrequency0); //free memory allocated for characterFrequency0
    free(characterFrequency); //free memory allocated for characterFrequency

    summarizeCompression(input, output); //print compression statistics to terminal: original bits, compressed bits, compression ratio, saved memory

    return;
}

/*function to decompress the input .bin file to a .txt file*/
void decompressFile(FILE *input, FILE *output) {
    printf("Decompressing file\n");
    //reconstruct code table from file
    int tableSize = 0;
    Code* codeTable = reconstructCodeTableFromFileBinary(input, &tableSize); //reconstruct code table from binary file

    decodeBinaryFile(input, output, codeTable, tableSize); //decode the binary string following the code table using the code table

    // Clean up memory for all variables
        for (int i = 0; i < tableSize; ++i) {
            free(codeTable[i].code); //free memory allocated for codeTable codes
        }
        free(codeTable); //free memory allocated for codeTable
}

/*Functions required for compression---------------------------------------------------------------------------------------*/
/*count the frequency of each character in the input file*/
void countFrequencies(FILE *file, Frequency *characterFrequency) 
{    
    int c;
    while ((c = fgetc(file)) != EOF) { //read file character by character until EOF is reached
        characterFrequency[c].character = c; //store the character in the array
        characterFrequency[c].frequency++; //increment the frequency of the character
    }
    rewind(file); //reset file pointer to beginning of file
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

/*replace sorted array with one excluding the zeros*/
Frequency *removeZeroElements(Frequency *characterFrequency0, int size)
{
    Frequency *characterFrequency1 = (Frequency *)malloc(size * sizeof(Frequency)); //allocate memory for the new array using size
    int j = 0;
    for (int i = 0; i < ASCII_SIZE; i++) //loop through the old array
    {
        if (characterFrequency0[i].frequency != 0) //if the frequency of the character is not 0
        { 
            characterFrequency1[j++] = characterFrequency0[i]; //copy the character and its frequency to the new array
        }
    }
    return characterFrequency1; //return the new array
}

/*finds the smallest node in the array*/
int findMinNode(Node* tree, int excludedIndex, int size) {
    int minIndex = -1; //initialise minIndex to -1 to indicate that no minimum has been found yet
    int minValue = INT_MAX; //initialise minValue to INT_MAX to ensure that the first node is always smaller

    for (int i = 0; i < size; ++i) { //loop through all nodes in the tree
        if (tree[i].value != -1 && i != excludedIndex) { //if the node is not excluded and not already used
            if (tree[i].value < minValue) { //if the value of the node is smaller than the current minimum
                minValue = tree[i].value; //replace the current minimum with the value of the node
                minIndex = i; //replace the current minimum index with the index of the node
            }
        }
    }

    return minIndex; //return the index of the smallest node
}

/*builds the huffman tree and returns its address by reference*/
Node* buildHuffmanTree(Node *tree, int *size, Frequency *characterFrequency){
    int i = 0;
    int NodesLeft = *size; //number of leaf nodes left to build the tree
    int smallOne,smallTwo; //smallest two nodes in the tree

    for (i=0;i<*size;i++){ //initialise tree leaf nodes
        tree[i].value = characterFrequency[i].frequency; //frequency of the character
        tree[i].letter = characterFrequency[i].character; //character
        tree[i].left = -1; //left child, will eventually contain the index of the left child node, so initialise to -1
        tree[i].right = -1; //right child, will eventually contain the index of the right child node, so initialise to -1
    }

    // Add EOF character to the tree
    tree[*size].value = 1; // EOF frequency
    tree[*size].letter = EOF; // EOF character
    tree[*size].left = -1; //left child, will eventually contain the index of the left child node, so initialise to -1
    tree[*size].right = -1; //right child, will eventually contain the index of the right child node, so initialise to -1
    (*size)++; //increment size of tree
    NodesLeft++; //increment number of leaf nodes left to build the tree

    while (NodesLeft>1){ //while there are still leaf nodes left to build the tree
        smallOne=findMinNode(tree,-1, *size); //find the smallest node in the tree, get the index of the smallest node
        smallTwo=findMinNode(tree,smallOne, *size); //find the second smallest node in the tree, get the index of the second smallest node

        tree = realloc(tree, sizeof(Node) * ((*size)+1)); //reallocate memory for the tree as another node is added

        tree[*size].value=tree[smallOne].value+tree[smallTwo].value; //value of the new node is the sum of the values of the two smallest nodes
        tree[*size].letter=-2; //letter of the new node is -2 to indicate that it is not a leaf node, but does not really matter
        tree[*size].left=smallTwo; //left child of the new node is the second smallest node
        tree[*size].right=smallOne; //right child of the new node is the smallest node
        tree[smallOne].value=-1; //set the value of the smallest node to -1 to indicate that it has been used
        tree[smallTwo].value=-1; //set the value of the second smallest node to -1 to indicate that it has been used
        NodesLeft--; //decrement number of leaf nodes left to build the tree
        (*size)++; //increment size of tree
    }

    return tree; //return address of final node in the tree, the root node
}

/*Function to build the code table*/
void buildCodeTable(Node* tree, int index, Code* codeTable, char* currentCode, int depth) {
    //Recursively traverses the tree to build the code table
    // If the node is a leaf, assign the binary code stored in current code to the corresponding character
    int LeftIndex = tree[index].left; //get index of left child node, done for readability
    if (tree[LeftIndex].left == -1 && tree[LeftIndex].right == -1) { //if the node is a leaf
        currentCode[depth] = '0'; //add a 0 to the current code
        codeTable[tree[LeftIndex].letter].letter = tree[LeftIndex].letter; //assign the character to the code table
        codeTable[tree[LeftIndex].letter].code = malloc((depth+1)*sizeof(char)); //allocate memory for the code
        memcpy(codeTable[tree[LeftIndex].letter].code, currentCode, (depth+1)*sizeof(char)); //copy the code from currentCode to the code table
        codeTable[tree[LeftIndex].letter].length = depth+1; //assign the length of the code to the code table variable length        
    }
    else { //if the node is not a leaf
        currentCode[depth] = '0'; //add a 0 to the current code
        buildCodeTable(tree, LeftIndex, codeTable, currentCode, depth + 1); //call function again for the left child node, increment depth
    }

    int RightIndex = tree[index].right; //get index of right child node, done for readability
    if (tree[RightIndex].right == -1 && tree[RightIndex].left == -1) { //if the node is a leaf
        currentCode[depth] = '1'; //add a 1 to the current code
        codeTable[tree[RightIndex].letter].letter = tree[RightIndex].letter; //assign the character to the code table
        codeTable[tree[RightIndex].letter].code = malloc((depth+1)*sizeof(char)); //allocate memory for the code
        memcpy(codeTable[tree[RightIndex].letter].code, currentCode, (depth+1)*sizeof(char)); //copy the code from currentCode to the code table
        codeTable[tree[RightIndex].letter].length = depth+1; //assign the length of the code to the code table variable length
    }
    else { //if the node is not a leaf
        currentCode[depth] = '1'; //add a 1 to the current code
        buildCodeTable(tree, RightIndex, codeTable, currentCode, depth + 1); //call function again for the right child node, increment depth
    }
}

/*Function to write the code table and its size to the header of a binary output file*/
void writeCodeTable2FileBinary(FILE* file, Code* codeTable, int tableSize) {

    fwrite(&tableSize, sizeof(char), 1, file); //write size of code table to the file

    for (int i = 0; i < ASCII_SIZE; ++i) { //loop through all ASCII characters
        if (codeTable[i].code != NULL) { //if the character exists in the code table
            
            fwrite(&codeTable[i].letter, sizeof(char), 1, file); //write the character to the file
            fwrite(&codeTable[i].length, sizeof(char), 1, file); //write the length of the code to the file, limits the code length to 255 bits, cuz sizeof(char)
            fwrite(codeTable[i].code, sizeof(char), codeTable[i].length, file); //write the code to the file
        }
    }
}

/*Function to write a binary string to a file*/
void writeBinaryString2File(FILE* file, char index, Code* codeTable, int size) {
    static unsigned char byte = 0; //make byte static to keep its value across function calls
    static int bitIndex = 7; //make bitIndex static to keep its value across function calls

    const char* binaryString = codeTable[index].code; //get the binary string from the code table
    int length = codeTable[index].length; //get the length of the binary string

    for (int i = 0; i < length; ++i) { //loop through all bits in the binary string
        if (binaryString[i] == '1') { //if the bit is 1
            byte |= (1 << bitIndex); //set the bit at bitIndex to 1
        }         
        bitIndex--; //decrement bitIndex

        if (bitIndex < 0) { //if all bits are set
            fwrite(&byte, sizeof(unsigned char), 1, file); //write the byte to the file
            byte = 0; //reset byte to 0
            bitIndex = 7; //reset bitIndex to 7
        }
    }

    if (index == size - 1 && bitIndex != 7) { //if this is the last character and not all bits are set
        fwrite(&byte, sizeof(unsigned char), 1, file); //write the byte to the file
    }
}

/*Functions required for decompression---------------------------------------------------------------------------------------*/
/*Function to reconstruct code table from binary file*/
Code* reconstructCodeTableFromFileBinary(FILE* file, int* tableSize) {

    if (fread(tableSize, sizeof(char), 1, file) < 1) { //read the size of the code table
        fprintf(stderr, "Error reading code table size from file\n"); //print error message if reading fails
        return NULL;  //return NULL on error
    }

    Code* codeTable = malloc((*tableSize) * sizeof(Code)); //allocate memory for the code table

    for (int i = 0; i < (*tableSize); ++i) { //loop through all entries in the code table
        fread(&codeTable[i].letter, sizeof(char), 1, file); //read the character

        int length;
        fread(&length, sizeof(char), 1, file); //read the length of the code

        codeTable[i].code = malloc(length + 1); //allocate memory for the code
        fread(codeTable[i].code, sizeof(char), length, file); //read the code
        codeTable[i].code[length] = '\0';  // Null-terminate the string

        codeTable[i].length = length; //assign the length of the code to the code table variable length
    }

    return codeTable; //return the full reconstructed code table
}

/*Function to decode a binary file using Huffman coding table*/
void decodeBinaryFile(FILE *input, FILE *output, Code *codeTable, int tableSize) {

    int buffer = 0; //buffer to store bits
    int bufferLength = 0; //number of bits in the buffer

    while (1) {
        unsigned char byte;
        if (fread(&byte, 1, 1, input) == 0) { //read one byte from the file, store in byte
            break; //and if its return value is 0, reached EOF, so break out of the loop
        }

        for (int i = 7; i >= 0; i--) { //loop through all bits in the byte
            buffer = (buffer << 1) | ((byte >> i) & 1); //add the current bit to the buffer
            bufferLength++; //increment the number of bits in the buffer

            //check for a match in the Huffman coding table
            for (int j = 0; j < tableSize; j++) { //loop through all entries in the code table
                if (bufferLength == codeTable[j].length && buffer == strtol(codeTable[j].code, NULL, 2)) { //if the buffer matches the code in the code table, and the length of the buffer matches the length of the code
                    fprintf(output, "%c", codeTable[j].letter); //write the character to the output file
                    //printf("%c", codeTable[j].letter); //print the character to the terminal

                    bufferLength -= codeTable[j].length; //decrement the number of bits in the buffer by the length of the code
                    buffer &= ~(0xFFFFFFFF << bufferLength); //reset the buffer to the remaining bits
                    break; //break out of the loop
                }
            }
        }
    }
}

/*Other functions------------------------------------------------------------------------------------------------------------*/
/*Function to get file type from name and decide to compress or decompress*/
int getFileType(char* filename) { //check if file is .txt or .bin, returns 1 for .txt, 2 for .bin, 0 for unknown file type
    if (strstr(filename, ".txt") != NULL) { //check if filename contains .txt
        return 1; // .txt file
    }
    else if (strstr(filename, ".bin") != NULL) { //check if filename contains .bin
        return 2; // .bin file
    }
    else {
        return 0; // unknown file type
    }
}

/*Summarize compression statistics*/
void summarizeCompression(FILE *input, FILE *output) {

    rewind(input); //reset file pointer to beginning of file to be sure
    fseek(input, 0L, SEEK_END); //go to end of file
    int originalBits = 8* ftell(input); //calculate input file size

    rewind(output); //reset file pointer to beginning of file to be sure
    fseek(output, 0L, SEEK_END); //go to end of file
    int compressedBits = 8* ftell(output); //calculate output file size

    fprintf(stderr,"Original bits = %d",originalBits); //print original bits to terminal
    printf("\n");
    fprintf(stderr,"Compressed bits = %d",compressedBits); //print compressed bits to terminal
    printf("\n");
    fprintf(stderr,"Compression ratio of %.2f%%",(((float)((float)compressedBits)/(float)(originalBits))*100)); //print compression ratio to terminal
    printf("\n");
    fprintf(stderr,"Saved %.2f%% of memory",(((float)(originalBits)-(float)compressedBits)/(float)(originalBits))*100); //print saved memory to terminal
    printf("\n");
}