#include <iostream>
#include <string>
#include "ArgParseStandalone.h"
#include <unistd.h>
#include <cstdint>

// CRC32 implementation from: https://rosettacode.org/wiki/CRC-32#C 

static const uint32_t generator = 0xEDB88320;

uint32_t rc_crc32(uint32_t crc, const char* buf, size_t len, size_t word_loc) {
    static uint32_t table[256];
    static bool have_table = false;
    uint32_t rem = 0;
    uint8_t octet = 0;

    /* This check is not thread safe; there is no mutex. */
    if(have_table == false) {
        /* Calculate CRC table. */
        for(int i = 0; i < 256; i++) {
            rem = i; /* remainder from polynomial division */
            for(int j = 0; j < 8; ++j) {
                if(rem & 1) {
                    rem >>= 1;
                    rem ^= generator;
                } else {
                    rem >>= 1;
                }
            }
            table[i] = rem;
        }
        have_table = true;
    }

    crc = ~crc;
    for(size_t i = 0; i < len; ++i) {
        if ((i >= (word_loc+4))||(i < word_loc)) {
            octet = *((uint8_t*)(buf+i));
        } else {
            octet = 0;
        }
        crc = (crc >> 8) ^ table[(crc & 0xff) ^ octet];
    }
    return ~crc;
}

int main(int argc, char** argv) {
    std::string input_filepath;
    ArgParse::ArgParser Parser("msexecrc");
    Parser.AddArgument("-i", "The input file", &input_filepath, ArgParse::Argument::Required);
    if(Parser.ParseArgs(argc, argv) < 0) {
        std::cerr << "There was a problem parsing args" << std::endl;
        return 1;
    }
    if(Parser.HelpPrinted()) {
        return 0;
    }
    if(access(input_filepath.c_str(), F_OK) == -1) {
        std::cerr << "The input file " << input_filepath << " doesn't exist!" << std::endl;
        return 1;
    }
    
    // Open the input file for reading.
    FILE* infile = fopen(input_filepath.c_str(), "r");
    if(infile == NULL) {
        std::cerr << "There was a problem opening the file to be read!" << std::endl;
    }

    // Read header info.
    const size_t buf_size = 1024;
    char buf[buf_size];
    if(fread(buf, buf_size, 1, infile) == 0) {
        std::cerr << "There was a problem reading the input file!" << std::endl;
        fclose(infile);
        return 1;
    }

    // Check that this is a microsoft binary
    if((buf[0] != 'M')||(buf[1] != 'Z')) {
        std::cerr << "This is not a valid microsoft binary!" << std::endl;
        fclose(infile);
        return 1;
    }

    uint32_t new_header_location = *((uint32_t*)(buf+0x3c));
    std::cout << "New header location: " << new_header_location << std::endl;

    return 0;
}
