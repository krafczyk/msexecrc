#include <iostream>
#include <string>
#include "ArgParseStandalone.h"

int main(int argc, char** argv) {
    std::string input_filepath;
    ArgParse::ArgParser Parser("msexecrc");
    Parser.AddArgument("-i", "The input file", &input_filepath);
    if(Parser.ParseArgs(argc, argv) < 0) {
        std::cerr << "There was a problem parsing args" << std::endl;
        return -1;
    }
    if(Parser.HelpPrinted()) {
        return 0;
    }
    return 0;
}
