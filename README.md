Huffman Compression/Decompression
This C program implements Huffman coding for file compression and decompression. It was written by Simon Stenger between the 01.10.2023 and the 21.01.2024 as part of the Programming and C language course in the ASNAT program at the Ecole Nationale d'Aviation Civile.
It compresses simple text files into a compressed binary file and vice versa.

Table of Contents
1.Introduction
2.Usage
3.File Structure
4.Structure of Compressed Files
5.Functions
6.Compilation
7.Execution
8.Sample Run
9.Example Files

1.Introduction
The program provides a command-line interface for compressing and decompressing text files using Huffman coding. It supports both compression (.txt to .bin) and decompression (.bin to .txt) based on the file extension.

2.Usage
  1.Include .txt files you would like to compress or .bin files you would like to decompress in the same directory as the files of this project.
  2.Compile the program.
  3.Run the executable in the command line.
  4.Follow the prompts to enter the filename for processing, examples are available, try 'example1.txt' for instance.
  
3.File Structure
  huffman_code.c: Contains the main function at the start, followed by the compression and decompression function, and then all smaller functions.
  types.h: Header file with structure definitions (Frequency, Node, Code).
  functions.h: Header file with function declarations.

4.Structure of Compressed Files
  Once compressed the output.bin file consists of three parts:
    1.Size of code table: Describes the length of the code table.
    2.The code table: For each character, the character itself, the length of the code and the binary representation are available.
    3.The huffman encoded content of the .txt input file.

5.Functions
  1.Compression Function
    compressFile: Compresses a text file using Huffman coding.
  2.Decompression Function
    decompressFile: Decompresses a binary file using the reconstructed code table.
  3.Frequency Calculation Functions
    countFrequencies: Counts the frequency of each character in the input file.
    countNonZero: Counts the number of non-zero elements in the frequency array.
    removeZeroElements: Removes elements with zero frequency from the frequency array.
  4.Huffman Tree Construction Functions
    findMinNode: Finds the smallest node in the tree, excluding a specified index.
    buildHuffmanTree: Builds the Huffman tree and returns its root address.
  5.Code Table Construction Functions
    buildCodeTable: Recursively builds the code table from the Huffman tree.
  6.Binary File I/O Functions
    writeCodeTable2FileBinary: Writes the code table and its size to the header of a binary output file.
    writeBinaryString2File: Writes a binary string to a binary file.
  7.Decompression-Specific Functions
    reconstructCodeTableFromFileBinary: Reconstructs the code table from a binary file.
    decodeBinaryFile: Decodes a binary file using the Huffman coding table.
  8.Other Functions
    getFileType: Determines the type of file based on its name.
    summarizeCompression: Prints compression statistics.
    
6.Compilation
  To compile the program, use the following command:
  gcc main.c functions.c -o huffman

7.Execution
  Run the compiled executable with the following command:
  ./huffman
  Then follow the on-screen prompts to specify the file for compression or decompression.

8.Sample Run
  The following outlines what you should see upon running the executable:
  1.Compression:
  
    -----------------------------Huffman (de-)compression-----------------------------
    
    Only .txt files are supported for compression and .bin files for decompression. Please ensure that the entered file path is no longer than 100 characters.
    The output file will be named output.bin for compression and output.txt for decompression. It will appear in the same directory as the input file.
    Type the name of the file to process: sample.txt
    
    Compressing file
    
    ...
    
    -----------------------------Compression complete-----------------------------
    Original bits = 272
    Compressed bits = 156
    Compression ratio of 57.35%
    Saved 42.65% of memory
    
  2.Decompression:
  
    -----------------------------Huffman (de-)compression-----------------------------
    Only .txt files are supported for compression and .bin files for decompression. Please ensure that the entered file path is no longer than 100 characters.
    The output file will be named output.bin for compression and output.txt for decompression. It will appear in the same directory as the input file.
    Type the name of the file to process:sample.bin
    Decompressing file
    
    -----------------------------Decompression complete-----------------------------

9.Example Files
  The project contains 3 example files containing 'Lorem Ipsum' text. All examples are available in their .txt and .bin form.
  Example 1 contains 200 words.
  Example 2 contains 500 words.
  Example 3 contains 1000 words.