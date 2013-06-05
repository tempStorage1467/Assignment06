/**********************************************************
 * File: LZWWrapper.h
 *
 * Eric Beach
 * Assignment 6 (Extension)
 *
 * These functions serve as a wrapper around the functions that perform
 *   the Lempel–Ziv–Welch (LZW) encoding scheme.
 *
 * The compression algorithm implemented here is not good for very small files
 *   such as the poem file in ./test/encodeDecode/poem;
 * This compression algorithm also will fail for very large files as it will
 *   overrun the maximum allowed size of integers. From more research,
 *   the code needs a reset dictionary function. I attempted to make
 *   this slightly better by changing to an unsigned long.
 *
 * I also wrote a substantative extension in the form of a series of tests in
 *   HuffmanEncodingTest.cpp to test this algorithm.
 */

/*
 * In order to research LZW and implement it, I read a number of articles.
 * I relied heavily on the articles below.
 * Credits:
 *   http://www.cplusplus.com/articles/iL18T05o/
 *   https://en.wikipedia.org/wiki/Lempel%E2%80%93Ziv%E2%80%93Welch
 *   http://www.cs.duke.edu/csed/curious/compression/lzw.html
 *   http://www.perlmonks.org/?node_id=270016
 *   http://rosettacode.org/wiki/LZW_compression#C.2B.2B
 */

#include "vector.h"
#include "map.h"
#include "simpio.h"
#include <string>
#include <iostream>
#include <sstream>
#include <iterator>
#include <vector>
#include <map>
#include "LZWLibrary.h"

using namespace std;

/*
 * Function: readFileToString
 * Usage: vector<unsigned long> compressed = readFileIntoCompressedVector(inputFile);
 * --------------------------------------------------------
 * Read a file from disk that consists of compressed data (in the form of
 *   a vector of integers) and return a vector of integers.
 */
vector<unsigned long> readFileIntoCompressedVector(ifstream& inputFile) {
    vector<unsigned long> compressed;
    string encodedLine;
    while (getline(inputFile, encodedLine)) {
        // the for loop is terribly inefficient; however, i cannot find a way
        //   to eliminate the NULL characters that are being written to
        //   disk, which is causing a lot of problems in reading and
        //   especially in converting to C-string.
        // hexdump outputTest.txt
        //   37 37 0a 00 31 32 31 00
        //   Notice the two instances of "00". We need to remove these.
        string strInt = "";
        for (int i = 0; i < encodedLine.size(); i++) {
            if (int(encodedLine[i]) != 0) strInt += encodedLine[i];
        }
        int elem = atoi(strInt.c_str());
        compressed.push_back(elem);
    }
    return compressed;
}

/*
 * Function: readFileToString
 * Usage: string rawInputContents = readFileToString(inputFile);
 * --------------------------------------------------------
 * Take a file and read in its contents one line at a time, building
 *   up a string representation of the file's contents.
 */
string readFileToString(ifstream& file) {
    string fileContents;
    string temp;
    while (getline(file, temp)) {
        // Note: this will admittedly break for systems requiring "\r\n"
        fileContents += temp + "\n";
    }
    fileContents = fileContents.substr(0, fileContents.size() - 1);
    return fileContents;
}

/*
 * Function: convertInt
 * Usage: convertInt(int number);
 * --------------------------------------------------------
 * Convert and integer into a string.
 * http://www.cplusplus.com/forum/beginner/7777/
 */
string convertInt(int number) {
    // create a stringstream
    stringstream ss;
    
    //add number to the stream
    ss << number;
    
    //return a string with the contents of the stream
    return ss.str();
}

/*
 * Function: writeToFile
 * Usage: writeToFile(ofstream& outFile, Vector<int>& content);
 * --------------------------------------------------------
 * Take a vector of ints, which represent the compressed data, and
 *  write those integers to an output file.
 */
void writeToFile(ofstream& outFile, vector<unsigned long>& content) {
    for (int i = 0; i < content.size(); i++) {
        string toWrite = convertInt(content[i]);
        if (i < content.size() - 1) toWrite += "\n";
        outFile.write(toWrite.c_str(), sizeof(toWrite.c_str()));
    }
}

/*
 * Function: compressFileLZW
 * Usage: compressFileLZW();
 * --------------------------------------------------------
 * Compress a file by using an iteration of the LZW algorithm.
 *   Prompt the user for a file to compress, read that file into a string,
 *   perform compression, and then write the compressed data to a file.
 */
void compressFileLZW() {
    // Step 1: Prompt the user for the file to be compressed
    ifstream inputFile;
    while (true) {
		string filename = getLine("Enter a file to compress with LZW: ");
		inputFile.open(filename.c_str());
        
		if (inputFile.is_open()) break;
        
		cout << "Sorry, I couldn't open that file." << endl;
		inputFile.clear();
	}
        
    // Step 2: Read input file into string
    const string rawInputContents = readFileToString(inputFile);
    
    // Step 3: Perform the compression by reading through the input file
    //   and compressing into a dictionary, represented as a vector of integers
    vector<unsigned long> compressed;
    compressString(rawInputContents, std::back_inserter(compressed));
    
    // Step 4: Close the input file
    inputFile.close();
    
    // Step 5: Prompt the user for a file to store the compressed data, then
    //   open the file to be written to
    ofstream outFile;
    while (true) {
        string filename = getLine("Enter a name for the output file: ");
        outFile.open(filename.c_str());
        
        if (outFile.is_open()) break;
        
        cout << "Sorry, I couldn't open that file for writing" << endl;
        outFile.clear();
    }
    
    // Step 6: Write the compressed file to disk; in essense, we need to
    //   serialize a vector of integers
    writeToFile(outFile, compressed);
    outFile.close();
}

/*
 * Function: askBoolQuestion
 * Usage: askBoolQuestion(string preface, string question);
 * --------------------------------------------------------
 * Prompt the user to answer a boolean yes/no question and return
 *   a boolean value based upon the user's response.
 */
bool askBoolQuestion(string preface, string question) {
    cout << endl;
    // if a preface to the question is supplied, print it
    if (preface.length() > 0) {
        cout << preface << endl;
    }
    string input;
    while (input != "Y" && input != "YES" &&
           input != "N" && input != "NO") {
        input = getLine(question);
        input = toUpperCase(input);
    }
    return (input == "Y" || input == "YES");
}

/*
 * Function: decompressFileLZW
 * Usage: decompressFileLZW();
 * --------------------------------------------------------
 * Prompt the user for a file to decompress and then decompress it. During
 *   the decompression process, ask the user whether the specific file should
 *   be decompressed.
 */
void decompressFileLZW() {
    // Step 1: Prompt the user for the file to be decompressed
    ifstream inputFile;
    while (true) {
		string filename = getLine("Enter a file to be decompressed with LZW: ");
		inputFile.open(filename.c_str());
        
		if (inputFile.is_open()) break;
        
		cout << "Sorry, I couldn't open that file." << endl;
		inputFile.clear();
	}
    
    // Step 2: Read input file into compressed Vector<int>
    vector<unsigned long> compressed = readFileIntoCompressedVector(inputFile);
    inputFile.close();
    
    // Step 3: Decompress the compressed content
    string deCompressed = decompress(compressed.begin(), compressed.end());
    
    // Step 4: Decompress file by reading compressed Vector<int> and
    //   print it as a string
    bool toPrint = askBoolQuestion("", "Do you want to print the"
                                   " decompressed output? ");
    if (toPrint) {
        cout << deCompressed << endl;
    }
    
    // Step 5: Ask the user whether to write the decompressed data to an
    //   output file.
    bool toWriteToFile = askBoolQuestion("", "Do you want to write the"
                                         " decompressed output to disk? ");
    if (toWriteToFile) {
        ofstream outFile;
        while (true) {
            string filename = getLine("Enter a name for the output file: ");
            outFile.open(filename.c_str());
            
            if (outFile.is_open()) break;
            
            cout << "Sorry, I was unable to open that file for writing" << endl;
            outFile.clear();
        }
        outFile.write(deCompressed.c_str(), sizeof(deCompressed.length()));
        outFile.close();
    }
}