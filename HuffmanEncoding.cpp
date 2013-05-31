/**********************************************************
 * File: HuffmanEncoding.cpp
 *
 * Implementation of the functions from HuffmanEncoding.h.
 * Most (if not all) of the code that you write for this
 * assignment will go into this file.
 */

#include "HuffmanEncoding.h"

/* Function: getFrequencyTable
 * Usage: Map<ext_char, int> freq = getFrequencyTable(file);
 * --------------------------------------------------------
 * Given an input stream containing text, calculates the
 * frequencies of each character within that text and stores
 * the result as a Map from ext_chars to the number of times
 * that the character appears.
 *
 * This function will also set the frequency of the PSEUDO_EOF
 * character to be 1, which ensures that any future encoding
 * tree built from these frequencies will have an encoding for
 * the PSEUDO_EOF character.
 */
Map<ext_char, int> getFrequencyTable(istream& file) {
    int nextChar;
    ext_char nextExtChar;
    Map<ext_char, int> freqMap;
    while (!file.eof()) {
        nextChar = file.get();
        if (nextChar == -1) continue;
        
        if (nextChar < 256 && nextChar >= 0) {
            nextExtChar = nextChar;
        } else {
            nextExtChar = NOT_A_CHAR;
        }
        freqMap.put(nextExtChar, freqMap.get(nextExtChar) + 1);
    }
    freqMap.put(PSEUDO_EOF, 1);
	return freqMap;
}

/* Function: buildEncodingTree
 * Usage: Node* tree = buildEncodingTree(frequency);
 * --------------------------------------------------------
 * Given a map from extended characters to frequencies,
 * constructs a Huffman encoding tree from those frequencies
 * and returns a pointer to the root.
 *
 * This function can assume that there is always at least one
 * entry in the map, since the PSEUDO_EOF character will always
 * be present.
 */
Node* buildEncodingTree(Map<ext_char, int>& frequencies) {
	// Step 1: Create a collection of singleton trees, one for each character,
    //   with weight equal to the character frequency
    PriorityQueue<Node*> pQueue;
    
    foreach (ext_char key in frequencies) {
        Node* cNode = new Node;
        cNode->character = key;
        cNode->zero = NULL;
        cNode->one = NULL;
        cNode->weight = frequencies.get(key);
        pQueue.enqueue(cNode, double(cNode->weight));
    }
    
    // Step 2: From the collection, pick out the two trees with the smallest
    //   weight and remove them. Combine them into a new tree whose root has a
    //   weight equal to the sum of the weights of the two
    //   trees and with the two trees as its left and right subtrees.
    while (pQueue.size() > 1) {
        Node* lowest = pQueue.dequeue();
        Node* secondLowest = pQueue.dequeue();
        
        Node* parent = new Node;
        parent->zero = lowest;
        parent->one = secondLowest;
        parent->weight = (lowest->weight) + (secondLowest->weight);
        parent->character = NOT_A_CHAR;
        
        // Step 3: Add the new combined tree back into the collection
        pQueue.enqueue(parent, double(parent->weight));
        
        // Step 4: Repeat steps 2 and 3 until there is only one tree left
    }
    
    // Step 5: The remaining node is the root of the optimal encoding tree
	return pQueue.peek();
}

/* Function: freeTree
 * Usage: freeTree(encodingTree);
 * --------------------------------------------------------
 * Deallocates all memory allocated for a given encoding
 * tree.
 */
void freeTree(Node* root) {
    if (root == NULL) return;
    
    freeTree(root->zero);
    freeTree(root->one);
    
    delete root;
}

void binaryPrefixsToExtChars(Node* encodingTree,
                             Map<string, ext_char>& extChars,
                             string soFar) {
    if (encodingTree->one == NULL && encodingTree->zero == NULL) {
        // Base Case: We are at the bottom of the tree in a certain branch
        extChars.put(soFar, encodingTree->character);
    } else {
        // Recursive Case: We still have further down the node to traverse
        if (encodingTree->zero != NULL) {
            string newSoFar = soFar;
            newSoFar += '0';
            binaryPrefixsToExtChars(encodingTree->zero, extChars, newSoFar);
        }
        
        if (encodingTree->one != NULL) {
            string newSoFar = soFar;
            newSoFar += '1';
            binaryPrefixsToExtChars(encodingTree->one, extChars, newSoFar);
        }
    }
}

void encTreeToBinaryPrefixes(Node* encodingTree,
                      Map<ext_char, string>& prefixes,
                      string soFar) {
    if (encodingTree->one == NULL && encodingTree->zero == NULL) {
        // Base Case: We are at the bottom of the tree in a certain branch
        prefixes.put(encodingTree->character, soFar);
    } else {
        // Recursive Case: We still have further down the node to traverse
        if (encodingTree->zero != NULL) {
            string newSoFar = soFar;
            newSoFar += '0';
            encTreeToBinaryPrefixes(encodingTree->zero, prefixes, newSoFar);
        }

        if (encodingTree->one != NULL) {
            string newSoFar = soFar;
            newSoFar += '1';
            encTreeToBinaryPrefixes(encodingTree->one, prefixes, newSoFar);
        }
    }
}


void writeEncodingPrefix(string prefix, obstream& outfile) {
    for (int i = 0; i < prefix.length(); i++) {
        if (prefix[i] == '0') {
            outfile.writeBit(0);
        } else {
            outfile.writeBit(1);
        }
    }
}

/* Function: encodeFile
 * Usage: encodeFile(source, encodingTree, output);
 * --------------------------------------------------------
 * Encodes the given file using the encoding specified by the
 * given encoding tree, then writes the result one bit at a
 * time to the specified output file.
 *
 * This function can assume the following:
 *
 *   - The encoding tree was constructed from the given file,
 *     so every character appears somewhere in the encoding
 *     tree.
 *
 *   - The output file already has the encoding table written
 *     to it, and the file cursor is at the end of the file.
 *     This means that you should just start writing the bits
 *     without seeking the file anywhere.
 */
void encodeFile(istream& infile, Node* encodingTree, obstream& outfile) {
    // generate map between ext_char and binary encoding
    Map<ext_char, string> prefixes;
    encTreeToBinaryPrefixes(encodingTree, prefixes, "");
    
    // ensure that the file pointer is at the beginning
    infile.seekg(0);
    
    // re-read file (you might have to push pointer back to begin of file)
    //   and for each char, look up the binary encoding and write it
    //   out using writeBit
    int nextChar;
    ext_char nextExtChar;
    while (!infile.eof()) {
        nextChar = infile.get();
        if (nextChar == -1) continue;
        
        if (nextChar < 256 && nextChar >= 0) {
            nextExtChar = nextChar;
        } else {
            nextExtChar = NOT_A_CHAR;
        }
        string prefix = prefixes.get(nextExtChar);
        writeEncodingPrefix(prefix, outfile);
    }
    
    // write PSEUDO_EOF
    string prefix = prefixes.get(PSEUDO_EOF);
    writeEncodingPrefix(prefix, outfile);
}

/* Function: decodeFile
 * Usage: decodeFile(encodedFile, encodingTree, resultFile);
 * --------------------------------------------------------
 * Decodes a file that has previously been encoded using the
 * encodeFile function.  You can assume the following:
 *
 *   - The encoding table has already been read from the input
 *     file, and the encoding tree parameter was constructed from
 *     this encoding table.
 *
 *   - The output file is open and ready for writing.
 */
void decodeFile(ibstream& infile, Node* encodingTree, ostream& file) {
    long numBits = infile.size() * 8;
    long numBitsRead = 0;
    string nextPrefix = "";
    Map<string, ext_char> extChars;
    binaryPrefixsToExtChars(encodingTree, extChars, "");
    while (numBitsRead <= numBits) {
        if (infile.readBit() == 0) {
            nextPrefix += '0';
        } else {
            nextPrefix += '1';
        }

        numBitsRead++;
        if (extChars.containsKey(nextPrefix)) {
            ext_char nextChar = extChars.get(nextPrefix);
            nextPrefix = "";
            if (nextChar == PSEUDO_EOF) {
                break;
            } else {
                file.put(char(nextChar));
            }
        }
    }
}

/* Function: writeFileHeader
 * Usage: writeFileHeader(output, frequencies);
 * --------------------------------------------------------
 * Writes a table to the front of the specified output file
 * that contains information about the frequencies of all of
 * the letters in the input text.  This information can then
 * be used to decompress input files once they've been
 * compressed.
 *
 * This function is provided for you.  You are free to modify
 * it if you see fit, but if you do you must also update the
 * readFileHeader function defined below this one so that it
 * can properly read the data back.
 */
void writeFileHeader(obstream& outfile, Map<ext_char, int>& frequencies) {
	/* The format we will use is the following:
	 *
	 * First number: Total number of characters whose frequency is being
	 *               encoded.
	 * An appropriate number of pairs of the form [char][frequency][space],
	 * encoding the number of occurrences.
	 *
	 * No information about PSEUDO_EOF is written, since the frequency is
	 * always 1.
	 */
	 
	/* Verify that we have PSEUDO_EOF somewhere in this mapping. */
	if (!frequencies.containsKey(PSEUDO_EOF)) {
		error("No PSEUDO_EOF defined.");
	}
	
	/* Write how many encodings we're going to have.  Note the space after
	 * this number to ensure that we can read it back correctly.
	 */
	outfile << frequencies.size() - 1 << ' ';
	
	/* Now, write the letter/frequency pairs. */
	foreach (ext_char ch in frequencies) {
		/* Skip PSEUDO_EOF if we see it. */
		if (ch == PSEUDO_EOF) continue;
		
		/* Write out the letter and its frequency. */
		outfile << char(ch) << frequencies[ch] << ' ';
	}
}

/* Function: readFileHeader
 * Usage: Map<ext_char, int> freq = writeFileHeader(input);
 * --------------------------------------------------------
 * Reads a table to the front of the specified input file
 * that contains information about the frequencies of all of
 * the letters in the input text.  This information can then
 * be used to reconstruct the encoding tree for that file.
 *
 * This function is provided for you.  You are free to modify
 * it if you see fit, but if you do you must also update the
 * writeFileHeader function defined before this one so that it
 * can properly write the data.
 */
Map<ext_char, int> readFileHeader(ibstream& infile) {
	/* This function inverts the mapping we wrote out in the
	 * writeFileHeader function before.  If you make any
	 * changes to that function, be sure to change this one
	 * too!
	 */
	Map<ext_char, int> result;
	
	/* Read how many values we're going to read in. */
	int numValues;
	infile >> numValues;
	
	/* Skip trailing whitespace. */
	infile.get();
	
	/* Read those values in. */
	for (int i = 0; i < numValues; i++) {
		/* Get the character we're going to read. */
		ext_char ch = infile.get();
		
		/* Get the frequency. */
		int frequency;
		infile >> frequency;
		
		/* Skip the space character. */
		infile.get();
		
		/* Add this to the encoding table. */
		result[ch] = frequency;
	}
	
	/* Add in 1 for PSEUDO_EOF. */
	result[PSEUDO_EOF] = 1;
	return result;
}

/* Function: compress
 * Usage: compress(infile, outfile);
 * --------------------------------------------------------
 * Main entry point for the Huffman compressor.  Compresses
 * the file whose contents are specified by the input
 * ibstream, then writes the result to outfile.  Your final
 * task in this assignment will be to combine all of the
 * previous functions together to implement this function,
 * which should not require much logic of its own and should
 * primarily be glue code.
 */
void compress(ibstream& infile, obstream& outfile) {
	Map<ext_char, int> freqTable = getFrequencyTable(infile);
    Node* encodingTree = buildEncodingTree(freqTable);
    
    writeFileHeader(outfile, freqTable);

    infile.rewind();
    
    encodeFile(infile, encodingTree, outfile);
    
    freeTree(encodingTree);
}

/* Function: decompress
 * Usage: decompress(infile, outfile);
 * --------------------------------------------------------
 * Main entry point for the Huffman decompressor.
 * Decompresses the file whose contents are specified by the
 * input ibstream, then writes the decompressed version of
 * the file to the stream specified by outfile.  Your final
 * task in this assignment will be to combine all of the
 * previous functions together to implement this function,
 * which should not require much logic of its own and should
 * primarily be glue code.
 */
void decompress(ibstream& infile, ostream& outfile) {
    Map<ext_char, int> freqTable = readFileHeader(infile);
    
    Node* encodingTree = buildEncodingTree(freqTable);
    
    decodeFile(infile, encodingTree, outfile);
    
    freeTree(encodingTree);
}

