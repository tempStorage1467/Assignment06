/*******************************************************
 * File: HuffmanEncoding.h
 *
 * Eric Beach
 * Assignment 6
 *
 * Definitions for the functions necessary to build a
 * Huffman encoding system.
 */

#ifndef HuffmanEncoding_Included
#define HuffmanEncoding_Included

#include "HuffmanTypes.h"
#include "map.h"
#include "bstream.h"
#include "pqueue.h"
#include <cmath>
#include "set.h"

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
Map<ext_char, int> getFrequencyTable(istream& file);

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
Node* buildEncodingTree(Map<ext_char, int>& frequencies);

/* Function: freeTree
 * Usage: freeTree(encodingTree);
 * --------------------------------------------------------
 * Deallocates all memory allocated for a given encoding
 * tree.
 */
void freeTree(Node* root);

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
void encodeFile(istream& infile, Node* encodingTree, obstream& outfile);

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
void decodeFile(ibstream& infile, Node* encodingTree, ostream& file);

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
void writeFileHeader(obstream& outfile, Map<ext_char, int>& frequencies);

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
Map<ext_char, int> readFileHeader(ibstream& infile);

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
void compress(ibstream& infile, obstream& outfile);

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
void decompress(ibstream& infile, ostream& outfile);

////////// ADDED HELPER FUNCTIONS //////////

/* Function: binaryPrefixsToExtChars
 * Usage: binaryPrefixsToExtChars(encodingTree->one, extChars, newSoFar);
 * --------------------------------------------------------
 * Take a binary tree, which represents the encoding prefixes used
 *   to encode/compress a specific file with the Huffman encoding, and
 *   create a map that maps between binary prefixes (as strings of
 *   0s and 1s) and chars
 */
void binaryPrefixsToExtChars(Node* encodingTree,
                             Map<string, ext_char>& extChars,
                             string soFar);

/* Function: encTreeToBinaryPrefixes
 * Usage: encTreeToBinaryPrefixes(encodingTree->one, prefixes, soFar);
 * --------------------------------------------------------
 * Take a binary tree, which represents the encoding prefixes used
 *   to encode/compress a specific file with the Huffman encoding, and
 *   create a map that maps between chars and binary prefixes (as strings of
 *   0s and 1s).
 */
void encTreeToBinaryPrefixes(Node* encodingTree,
                             Map<ext_char, string>& prefixes,
                             string soFar);

/* Function: writeEncodingPrefix
 * Usage: writeEncodingPrefix(prefix, outfile);
 * --------------------------------------------------------
 * Take an encoding prefix, which is represented as a string of 0s and 1s and
 *   write it to the output file. This is used to write a specific character,
 *   which has been compressed down using a binary prefix, to the output file.
 */
void writeEncodingPrefix(string prefix, obstream& outfile);

/* Function: scrambleTable
 * Usage: scrambleTable(frequencies);
 * --------------------------------------------------------
 * Extension
 * An extension to provide encryption to the Huffman compression algorithm.
 * Scrambles the frequency map.
 */
void scrambleTable(Map<ext_char, int>& frequencies);

/* Function: descrambleTable
 * Usage: descrambleTable(result);
 * --------------------------------------------------------
 * Extension
 * An extension to provide encryption to the Huffman compression algorithm.
 * Descrambles the frequency map.
 */
void descrambleTable(Map<ext_char, int>& frequencies);

#endif
