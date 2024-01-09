//Huffman coding
// By Simon Stenger
// Date: 04/01/2023
// Version: 2.0

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define INT_MAX 2147483647

/* Node of the huffman tree */
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
struct code {
    char letter;
    char* code;
    int length;
};

typedef struct node Node;
typedef struct char_freqs Frequency;
typedef struct code Code;

/*Read input file into variable in a function*/
void read_file(char *filename, char *input) /*inputs: pointer to filename array, pointer to input array*/
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

    while ((c = fgetc(fp)) != EOF) /*copy characters into input array until end of file is reached*/
    {
        input[i] = c;
        i++;
    }
    input[i] = '\0';
    fclose(fp);
}

void count_freq(char *initial, Frequency *char_frequency) 
{    
    for (int i = 0; initial[i] != '\0'; i++)
    {
        char_frequency[(unsigned char)initial[i]].character = initial[i]; /*assign character to huffman node*/
        char_frequency[(unsigned char)initial[i]].frequency++; /*update frequency of huffman node*/
    }
}

/*replace sorted array with one excluding the zeros*/
Frequency *remove_zeros(Frequency *char_frequency, int new_size)
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

    //freeHuffmanTree(tree->left);
    //freeHuffmanTree(tree->right);
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
    printf("Entered build Huffman");
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
        printf("Number of nodes so far %d", i);
        printf("\n");
    }

    while (nodes_left>1){
        smallOne=findMinNode(tree,-1, *new_size);
        smallTwo=findMinNode(tree,smallOne, *new_size);
        //temp = *tree[smallOne];
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
    printf("Huffman tree built");
    return tree;
}

// Function to build the code table
void buildCodeTable(Node* tree, int index, Code* codeTable, char* currentCode, int depth) {
    printf("Entered build code table");
    if (tree == NULL) {
        return;
    }

    // If the node is a leaf, assign the binary code to the corresponding character

    // Traverse left and append '0' to the current code
    int LeftIndex = tree[index].left;
    if (tree[LeftIndex].left == -1 && tree[LeftIndex].right == -1) {
        codeTable[tree[LeftIndex].letter].letter = tree[LeftIndex].letter;
        codeTable[tree[LeftIndex].letter].code = malloc(depth*sizeof(char));
        memcpy(codeTable[tree[LeftIndex].letter].code, currentCode, depth*sizeof(char));
        // codeTable[tree->letter].code = currentCode;
        codeTable[tree[LeftIndex].letter].length = depth;
        
    }
    else {
        // char* leftCode = malloc((depth + 1) * sizeof(char));
        // currentCode = realloc(currentCode, (depth + 1)*sizeof(char));
        // snprintf(leftCode, depth + 1, "%s0", currentCode);
        currentCode[depth] = '0';
        buildCodeTable(tree, LeftIndex, codeTable, currentCode, depth + 1);
        // free(leftCode);
    }

    // Traverse right and append '1' to the current code
    int RightIndex = tree[index].right;
    if (tree[RightIndex].right == -1 && tree[RightIndex].left == -1) {
        codeTable[tree[RightIndex].letter].letter = tree[RightIndex].letter;
        codeTable[tree[RightIndex].letter].code = malloc(depth*sizeof(char));
        memcpy(codeTable[tree[RightIndex].letter].code, currentCode, depth*sizeof(char));
        // codeTable[tree->letter].code = currentCode;
        codeTable[tree[RightIndex].letter].length = depth;
    }
    else {
        // char* rightCode = malloc((depth + 1) * sizeof(char));
        // snprintf(rightCode, depth + 1, "%s1", currentCode);
        currentCode[depth] = '1';
        buildCodeTable(tree, RightIndex, codeTable, currentCode, depth + 1);
        // free(rightCode);
    }

    printf("Code table built");
    // Free the memory allocated for the current code and huffman tree
    //free(currentCode);
    //freeHuffmanTree(tree);
}

// Function to write the code table to the header of a text output file
void writeCodeTableToFile(FILE* file, Code* codeTable) {
    fprintf(file, "CodeTable\n");

    for (int i = 0; i < 128; ++i) {
        if (codeTable[i].code != NULL) {
            fprintf(file, "%d %.*s\n", codeTable[i].letter, codeTable[i].length, codeTable[i].code);
        }
    }
    fprintf(file, "EndCodeTable\n");
}

// Function to read and restore the code table from a compressed file
void readCodeTableFromFile(FILE* file, Code* codeTable) {
    char line[256];  // Adjust the size accordingly based on your requirements

    // Search for the beginning of the code table section
    while (fgets(line, sizeof(line), file) != NULL) {
        if (strcmp(line, "CodeTable\n") == 0) {
            break;
        }
    }

    // Read code table entries until the end of the code table section
    while (fgets(line, sizeof(line), file) != NULL) {
        if (strcmp(line, "EndCodeTable\n") == 0) {
            break;
        }

        int letter;
        char code[128];  // Adjust the size accordingly based on your requirements
        sscanf(line, "%d %s", &letter, code);

        // Dynamically allocate memory for codeTable[letter].code
        codeTable[letter].letter = (unsigned char)letter;
        codeTable[letter].code = strdup(code);
    }
}

// Function to print the code table DOES NOT WORK
void printCodeTable(Code* codeTable, int new_size) {
    printf("Code table:");
    printf("\n");
    for (int i = 0; i < new_size; ++i) {
        if (codeTable[i].code != NULL) {
            printf("Character %d     Code: %s", codeTable[i].letter, codeTable[i].code);
            printf("\n");
        }
    }
}

/*function to compress the input*/
void compressFile(FILE *input, FILE *output, Code* codeTable, int new_size){
    printf("Compressing file");
    printf("\n");
    //write the codeTable into the header of the output file
    writeCodeTableToFile(output, codeTable);
    //loop over characters in input file, take the code from the codeTable and write it to the output file
    char c;
    while ((c = fgetc(input)) != EOF) {
        fprintf(output, "%.*s", codeTable[c].length, codeTable[c].code);
        // int length = strlen(codeTable[c].code);
        printf("Length of code: %d", codeTable[c].length);
        printf("\n");
        printf("Character: %c", c);
        printf("\n");
        printf("Code: %.*s", codeTable[c].length, codeTable[c].code);
        printf("\n");
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
    printf("Decompressing file");
    printf("\n");

    int byte;
    int bit;

    // Read each byte from the file
    while ((byte = fgetc(input)) != EOF) {
        // Process each bit within the byte
        for (bit = 7; bit >= 0; --bit) {
            int currentBit = (byte >> bit) & 1;
            printf("%d", currentBit); // Replace this with your actual processing logic
        }
    }
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

    input = fopen(filename, "r");
    output = fopen("output.txt","w");

    if (compress==1){
        Frequency *char_frequency0 = (Frequency *)calloc(128, sizeof(Frequency));
        char initial[3000];
        read_file(filename, initial);
        count_freq(initial, char_frequency0);
        int new_size = count_non_zero(char_frequency0, 128);
        printf("new size: %d\n", new_size);
        Frequency *char_frequency = remove_zeros(char_frequency0, new_size);

        Node* tree = malloc(sizeof(Node) * new_size);
        int number_nodes;
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
        compressFile(input, output, codeTable, new_size);

        // Clean up memory for all variables
        freeHuffmanTree(tree);
        for (int i = 0; i < 128; ++i) {
            free(codeTable[i].code);
        }
        free(codeTable);       
    }
    else{
        Code* codeTable = malloc(128 * sizeof(Code));
        readCodeTableFromFile(input, codeTable);
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