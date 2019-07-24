#include <iostream>
#include <string>
#include "ArgParseStandalone.h"
#include <unistd.h>
#include <cstdint>

// CRC32 implementation from: https://rosettacode.org/wiki/CRC-32#C 
#define BUFFER_SIZE 1024

uint32_t rc_crc32(FILE* the_file, char* buf, size_t word_loc, uint32_t generator) {
    uint32_t table[256];
    //static bool have_table = false;
    uint32_t rem = 0;
    uint8_t octet = 0;

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

    // Seek to beginning of file.
    if(fseek(the_file, 0, SEEK_SET) != 0) {
        std::cerr << "There was a problem seeking to the beginning!" << std::endl;
        return 0;
    }

    size_t num_bytes = 0;
    size_t buff_base = 0;
    uint32_t crc = 0;
    crc = ~crc;
    while(true) {
        // Read the next BUFFER_SIZE worth of bytes
        num_bytes = fread(buf, 1, BUFFER_SIZE, the_file);

        for(size_t i = 0; i < num_bytes; ++i) {
            if (((i+buff_base) >= (word_loc+4))||((i+buff_base) < word_loc)) {
                octet = *((uint8_t*)(buf+i));
            } else {
                octet = 0;
            }
            crc = (crc >> 8) ^ table[(crc & 0xff) ^ octet];
        }

        buff_base += num_bytes;

        if(feof(the_file) != 0) {
            // at end of file
            break;
        }
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
    char buf[BUFFER_SIZE];
    if(fread(buf, 1, BUFFER_SIZE, infile) == 0) {
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

    // Seek to new header location file
    if(fseek(infile, new_header_location, SEEK_SET) != 0) {
        std::cerr << "Couldn't seek to new header location!" << std::endl;
        fclose(infile);
        return 1;
    } 
    // Read new buffer data
    if(fread(buf, 1, BUFFER_SIZE, infile) == 0) {
        std::cerr << "There was a problem reading the input file!" << std::endl;
        fclose(infile);
        return 1;
    }

    // Check that we have an NE binary
    if((buf[0] != 'N')||(buf[1] != 'E')) {
        std::cerr << "This is not an NE binary!" << std::endl;
        fclose(infile);
        return 1;
    }

    uint32_t crc_location = new_header_location+0x8;

    uint32_t gen_1 = 0xEDB88320;
    uint32_t gen_2 = 0x82F63B78;
    uint32_t gen_3 = 0xEB31D82E;
    uint32_t gen_4 = 0x992C1A4C;
    uint32_t gen_5 = 0xD5828281;

    uint32_t new_crc_1 = rc_crc32(infile, buf, crc_location, gen_1);
    std::cout << "Generator: " << std::hex << gen_1 << " -> " << new_crc_1 << std::endl;
    uint32_t new_crc_2 = rc_crc32(infile, buf, crc_location, gen_2);
    std::cout << "Generator: " << std::hex << gen_2 << " -> " << new_crc_2 << std::endl;
    uint32_t new_crc_3 = rc_crc32(infile, buf, crc_location, gen_3);
    std::cout << "Generator: " << std::hex << gen_3 << " -> " << new_crc_3 << std::endl;
    uint32_t new_crc_4 = rc_crc32(infile, buf, crc_location, gen_4);
    std::cout << "Generator: " << std::hex << gen_4 << " -> " << new_crc_4 << std::endl;
    uint32_t new_crc_5 = rc_crc32(infile, buf, crc_location, gen_5);
    std::cout << "Generator: " << std::hex << gen_5 << " -> " << new_crc_5 << std::endl;


    fclose(infile);
    return 0;
}
