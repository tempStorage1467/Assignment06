/**********************************************************
 * File: LZWWLibrary.h
 *
 * Eric Beach
 * Assignment 6 (Extension)
 */

/*
 * I defaulted to the following implementation of LZW. This file is taken
 *   in its entirity from http://rosettacode.org/wiki/LZW_compression#C.2B.2B
 *   and plugged into my wrapper function. To be very clear, none of the work
 *   in this particular file is mine.
 */

#ifndef Huffman_Encoding_LZW2_h
#define Huffman_Encoding_LZW2_h

#include <string>
#include <map>

// Compress a string to a list of output symbols.
// The result will be written to the output iterator
// starting at "result"; the final iterator is returned.
template <typename Iterator>
Iterator compressString(const std::string &uncompressed, Iterator result) {
    // Build the dictionary.
    int dictSize = 256;
    std::map<std::string,int> dictionary;
    for (int i = 0; i < 256; i++)
        dictionary[std::string(1, i)] = i;
    
    std::string w;
    for (std::string::const_iterator it = uncompressed.begin();
         it != uncompressed.end(); ++it) {
        char c = *it;
        std::string wc = w + c;
        if (dictionary.count(wc))
            w = wc;
        else {
            *result++ = dictionary[w];
            // Add wc to the dictionary.
            dictionary[wc] = dictSize++;
            w = std::string(1, c);
        }
    }
    
    // Output the code for w.
    if (!w.empty())
        *result++ = dictionary[w];
    return result;
}

// Decompress a list of output ks to a string.
// "begin" and "end" must form a valid range of ints
template <typename Iterator>
std::string decompress(Iterator begin, Iterator end) {
    // Build the dictionary.
    int dictSize = 256;
    std::map<unsigned long,std::string> dictionary;
    for (int i = 0; i < 256; i++)
        dictionary[i] = std::string(1, i);
    
    std::string w(1, *begin++);
    std::string result = w;
    std::string entry;
    for ( ; begin != end; begin++) {
        int k = *begin;
        if (dictionary.count(k))
            entry = dictionary[k];
        else if (k == dictSize)
            entry = w + w[0];
        else
            throw "Bad compressed k";
        
        result += entry;
        
        // Add w+entry[0] to the dictionary.
        dictionary[dictSize++] = w + entry[0];
        
        w = entry;
    }
    return result;
}


#endif
