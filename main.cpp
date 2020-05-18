#include "lib.h"
#include <stdlib.h>
#include <iostream>

static const char* PROGRAM_ERR_MSG [] = {
    "Succeeded",
    "Invalid image",
    "Incomplete Image",
    "Memory allocation failure"
};

#define MEMORY_ALLOC_MSG 3

void image_parsing_progress(unsigned int progress) {
    std::cout << "Image parsing: " << progress << " out of 100\n";
}

// An example application that simulates a typical image parsing program
// The library simulates a typilcal image decoding library such as libjpeg
int main(int argc, char const *argv[])
{
    // create a buffer for input bytes
    char* input_stream = new char[100];
    if (!input_stream) {
        std::cerr << "Error: " << PROGRAM_ERR_MSG[MEMORY_ALLOC_MSG] << "\n";
        return 1;
    }

    // This is where we may read bytes from an image file into input_stream
    // But this is just a toy example
    // So we will just assume that input_stream buffer has bytes we want to parse

    ImageHeader* header = parse_image_header(input_stream);

    if (header->status_code != STATUS_SUCCEEDED) {
        std::cerr << "Error: " << PROGRAM_ERR_MSG[header->status_code] << "\n";
        return 1;
    }

    char* output_stream = new char[header->width * header->height];
    if (!output_stream) {
        std::cerr << "Error: " << PROGRAM_ERR_MSG[MEMORY_ALLOC_MSG] << "\n";
        return 1;
    }

    parse_image_body(input_stream, header, image_parsing_progress, output_stream);


    free(header);
    delete input_stream;
    delete output_stream;

    return 0;
}
