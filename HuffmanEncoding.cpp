/**********************************************************
 * File: HuffmanEncoding.cpp
 *
 * Eric Beach
 * Assignment 6
 *
 * Implementation of the functions from HuffmanEncoding.h.
 *
 * This assignment contains three significant extensions:
 *   (1) Encryption - Encryption is provided by scrambling the frequency table.
 *   (2) LZW Compression - LZW compression is provided.
 *   (3) Additional Unit Tests - Many additional unit tests for these extensions.
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
    
    // create a frequency map to build and store the computed result
    Map<ext_char, int> freqMap;
    
    // iterate over the input file character-by-character;
    //   with each new character, set it as the proper ext_char value
    //   and then increase the frequency map value for the character
    while (!file.eof()) {
        nextChar = file.get();
        if (nextChar == -1) continue;
        
        if (nextChar < 256 && nextChar >= 0) {
            nextExtChar = nextChar;
        } else {
            nextExtChar = NOT_A_CHAR;
        }
        
        // update the frequency map with the next character, which now
        //   should have one more instance than it previously did
        freqMap.put(nextExtChar, freqMap.get(nextExtChar) + 1);
    }

    // add the PSEUDO_EOF character to the map, since each encoding will use
    //   this key once
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
    
    // for each character in the frequency tree, create a singleton node
    //   and set the character value equal to the character in the frequency
    //   table; the node weight is the frequency in the table
    foreach (ext_char key in frequencies) {
        Node* cNode = new Node;
        cNode->character = key;
        
        // need to explicitly initialize zero and one nodes as null
        //   or serious errors will happen later
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
        
        // new weight is sum of other cells' weight
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
 * Deallocates all memory allocated for a given encoding tree.
 */
void freeTree(Node* root) {
    if (root == NULL) return;
    
    // traverse down the left branch
    freeTree(root->zero);
    
    // traverse down the right branch
    freeTree(root->one);
    
    delete root;
}

/* Function: binaryPrefixsToExtChars
 * Usage: binaryPrefixsToExtChars(encodingTree->one, extChars, newSoFar);
 * --------------------------------------------------------
 * Takes a populated binary tree (Node* encodingTree) and builds up
 *   a map from the binary represetnation to the character and that
 *   specific character.
 *
 * Major output is: Map<string, ext_char>& extChars
 *
 * Take a binary tree, which represents the encoding prefixes used
 *   to encode/compress a specific file with the Huffman encoding, and
 *   create a map that maps between binary prefixes (as strings of
 *   0s and 1s) and chars
 *
 * Note: It seems rather inelegant to use a string to represent bits
 *   but I do not recall our class covering a better way to store bits and
 *   manipulate them.
 */
void binaryPrefixsToExtChars(Node* encodingTree,
                             Map<string, ext_char>& extChars,
                             string soFar) {
    if (encodingTree->one == NULL && encodingTree->zero == NULL) {
        // Base Case: We are at the bottom of the tree in a certain branch
        extChars.put(soFar, encodingTree->character);
    } else {
        // Recursive Case: We still have further down the node to traverse
        if (encodingTree->zero != NULL) {
            // left branch of the encoding tree exists, so traverse
            //   down that branch
            string newSoFar = soFar;
            newSoFar += '0';
            binaryPrefixsToExtChars(encodingTree->zero, extChars, newSoFar);
        }
        
        if (encodingTree->one != NULL) {
            // right branch of the encoding tree exists, so traverse
            //   down that branch
            string newSoFar = soFar;
            newSoFar += '1';
            binaryPrefixsToExtChars(encodingTree->one, extChars, newSoFar);
        }
    }
}

/* Function: encTreeToBinaryPrefixes
 * Usage: encTreeToBinaryPrefixes(encodingTree->one, prefixes, soFar);
 * --------------------------------------------------------
 * Takes a populated binary tree (Node* encodingTree) and builds up
 *   a map from a character to its binary representation.
 *
 * Major output is: Map<ext_char, string>& prefixes
 *
 * Take a binary tree, which represents the encoding prefixes used
 *   to encode/compress a specific file with the Huffman encoding, and
 *   create a map that maps between chars and binary prefixes (as strings of 
 *   0s and 1s).
 *
 * Note: It seems rather inelegant to use a string to represent bits
 *   but I do not recall our class covering a better way to store bits and
 *   manipulate them.
 */
void encTreeToBinaryPrefixes(Node* encodingTree,
                      Map<ext_char, string>& prefixes,
                      string soFar) {
    if (encodingTree->one == NULL && encodingTree->zero == NULL) {
        // Base Case: We are at the bottom of the tree in a certain branch
        prefixes.put(encodingTree->character, soFar);
    } else {
        // Recursive Case: We still have further down the node to traverse
        if (encodingTree->zero != NULL) {
            // left branch of the encoding tree exists, so traverse
            //   down that branch
            string newSoFar = soFar;
            
            // we are walking down the left tree, so the prefix is
            //   appended with a 0
            newSoFar += '0';
            encTreeToBinaryPrefixes(encodingTree->zero, prefixes, newSoFar);
        }

        if (encodingTree->one != NULL) {
            // right branch of the encoding tree exists, so traverse
            //   down that branch
            string newSoFar = soFar;
            
            // we are walking down the right tree, so the prefix is
            //   appended with a 1
            newSoFar += '1';
            encTreeToBinaryPrefixes(encodingTree->one, prefixes, newSoFar);
        }
    }
}

/* Function: writeEncodingPrefix
 * Usage: writeEncodingPrefix(prefix, outfile);
 * --------------------------------------------------------
 * Take an encoding prefix, which is represented as a string of 0s and 1s and
 *   write it to the output file. This is used to write a specific character,
 *   which has been compressed down using a binary prefix, to the output file.
 */
void writeEncodingPrefix(string prefix, obstream& outfile) {
    // iterate over every index of the prefix string, which is either
    //   a zero or one, and write that bit to the output file
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
    // generate map between ext_char and binary encoding of 0s and 1s,
    //   which is represented as a string
    Map<ext_char, string> prefixes;
    encTreeToBinaryPrefixes(encodingTree, prefixes, "");
        
    // for each char, look up the binary encoding and write it
    //   to the output file using writeBit
    int nextChar;
    ext_char nextExtChar;
    while (!infile.eof()) {
        // read the next character from the file to be compressed
        nextChar = infile.get();
        
        // if the next char is -1, we should not write this char
        if (nextChar == -1) continue;
        
        // determine if the next char is an ASCII character; if so, encode
        //   it as such; otherwise, encode it as not a standard char
        if (nextChar < 256 && nextChar >= 0) {
            nextExtChar = nextChar;
        } else {
            nextExtChar = NOT_A_CHAR;
        }
        
        // get the encoding prefix for the next char to write compressed
        string prefix = prefixes.get(nextExtChar);
        
        // write the encoding prefix to the compressed output file
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
    // compute the total number of bits so we do not overread the file
    long numBits = infile.size() * 8;
    
    // store the number of bits we have read
    long numBitsRead = 0;
    
    // build up a potential encoding prefix one bit at a time
    string nextPrefix = "";
    
    // create a map between the prefixes (stored as a string of 0s and 1s) and
    //   ext_char that is encoded
    Map<string, ext_char> extChars;
    binaryPrefixsToExtChars(encodingTree, extChars, "");
    
    // read through the encoded file one bit at a time, building up
    //   the next possible prefix; if a valid prefix has been built up,
    //   then decode and write to the decoded output file
    while (numBitsRead <= numBits) {
        // read the next bit and add it to the prefix
        if (infile.readBit() == 0) {
            nextPrefix += '0';
        } else {
            nextPrefix += '1';
        }

        numBitsRead++;
        // check to see if the prefix we've built up thus far is a valid
        //   prefix; if so, write it 
        if (extChars.containsKey(nextPrefix)) {
            ext_char nextChar = extChars.get(nextPrefix);
            
            // clear the next prefix
            nextPrefix = "";
            if (nextChar == PSEUDO_EOF) {
                // we have reached the end of the section of the encoded file
                //   that contains the encoded data; the rest is just filler
                //   bits, so quit now
                break;
            } else {
                // take the decoded next character and write it to disk
                file.put(char(nextChar));
            }
        }
    }
}

/* Function: performScrambleOperation
 * Usage: performScrambleOperation(frequencies, false);
 * --------------------------------------------------------
 * Extension
 * An extension to provide encryption to the Huffman compression algorithm.
 * Scrambles or descrambles the frequency map, depending upon the value
 *   passed in to the decode parameter.
 */
void performScrambleOperation(Map<ext_char, int>& frequencies, bool decode) {
    // store the indicies that we've already swapped; otherwise, we will
    //   swap the indices back when we iterate over them the second time;
    //   i.e., we will swap 0 and 255 and then when we reach 255, we will
    //   wrongly swap it back with 0
    Set<int> alreadySwapped;
    foreach (ext_char ch in frequencies) {
        // do not encrypt EOF or non characters
        if (ch == PSEUDO_EOF || ch == NOT_A_CHAR) continue;
        
        // don't swap back an already swapped element
        if (alreadySwapped.contains(ch)) continue;
        
        // store the values we are going to swap
        ext_char oldChar = ch;
        int oldFreq = frequencies[ch];
        
        ext_char scrambledCh;
        if (decode) {
            scrambledCh = abs(255 - ch);
        } else {
            scrambledCh = abs(ch - 255);
        }
        
        int oldEncodedChFreq = 0;
        if (frequencies.containsKey(scrambledCh)) {
            oldEncodedChFreq = frequencies[scrambledCh];
        }
        
        if (frequencies.containsKey(scrambledCh)) {
            frequencies[scrambledCh] = oldFreq;
            frequencies[oldChar] = oldEncodedChFreq;
        } else {
            frequencies[scrambledCh] = oldFreq;
            
            // since there is no value stored at the key encodedCh,
            //   we need to remove the key oldChar so our Map does not
            //   expand in size
            frequencies.remove(oldChar);
        }
        alreadySwapped.add(scrambledCh);
    }
}

/* Function: scrambleTable
 * Usage: scrambleTable(frequencies);
 * --------------------------------------------------------
 * Extension
 * An extension to provide encryption to the Huffman compression algorithm.
 * Scrambles the frequency map.
 *
 * Input Frequency Map: {10, 2; 50, 4; 256, 1}
 * Output Frequency Map: {245, 2; 205, 4; 256, 1} // 256 is PSEUDO_EOF
 *
 */
void scrambleTable(Map<ext_char, int>& frequencies) {
    performScrambleOperation(frequencies, false);
}

/* Function: descrambleTable
 * Usage: descrambleTable(result);
 * --------------------------------------------------------
 * Extension
 * An extension to provide encryption to the Huffman compression algorithm.
 * Descrambles the frequency map.
 *
 * Input Frequency Map: {10, 2; 50, 4; 256, 1}
 * Output Frequency Map: {245, 2; 205, 4; 256, 1} // 256 is PSEUDO_EOF
 */
void descrambleTable(Map<ext_char, int>& frequencies) {
    performScrambleOperation(frequencies, true);
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

    /* Extension to encrypt the frequency table */
    scrambleTable(frequencies);
    
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
    
    /* Extension to decrypt the frequency table */
    descrambleTable(result);
    
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
    // generate a table showing the frequency of each char
	Map<ext_char, int> freqTable = getFrequencyTable(infile);
    
    // create the encoding tree based upon the character frequency table
    Node* encodingTree = buildEncodingTree(freqTable);
    
    // write the encoding tree into the header of the output file so
    //   other clients can decode
    writeFileHeader(outfile, freqTable);

    // rewind the file pointer to the beginning of the input file as
    //   this input file has been read through once to calculate the
    //   frequency table
    infile.rewind();
    
    // using the encoding tree, read through the input file and encode it
    encodeFile(infile, encodingTree, outfile);
    
    // free the memory allocated in creating the encoding tree
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
    // in order to decompress, we must have the encryption table;
    //   read the encryption table in the header of the encrypted file
    Map<ext_char, int> encodeTable = readFileHeader(infile);
    
    // take the encoding table and build the encoding tree
    Node* encodingTree = buildEncodingTree(encodeTable);
    
    // using the encoding tree, decode the encoded file and write it
    //   out to an output file
    decodeFile(infile, encodingTree, outfile);
    
    // free the memory allocated in creating the encoding tree
    freeTree(encodingTree);
}
