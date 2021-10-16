#include "lib.h"
#include <stdlib.h>
#include <iostream>

static const char* PROGRAM_STATUS_MSG [] = {
    "Succeeded",
    "Invalid image",
    "Incomplete Image",
    "Memory allocation failure"
};

#define MEMORY_ALLOC_ERR_MSG 3

void image_parsing_progress(unsigned int progress) {
    std::cout << "Image parsing: " << progress << " out of 100\n";
}

void get_image_bytes(char* input_stream) {
    // Get the bytes of the image from the file into input stream
    // This is just a toy example, so we will leave this empty for now
}

// An example application that simulates a typical image parsing program
// The library simulates a typilcal image decoding library such as libjpeg
int main(int argc, char const *argv[])
{
    // create a buffer for input bytes
    char* input_stream = new char[100];
    if (!input_stream) {
        std::cerr << "Error: " << PROGRAM_STATUS_MSG[MEMORY_ALLOC_ERR_MSG] << "\n";
        return 1;
    }

    // Read bytes from an image file into input_stream
    get_image_bytes(input_stream);

    // Parse header of the image to get its dimensions
    ImageHeader* header = parse_image_header(input_stream);

    if (header->status_code != HEADER_PARSING_STATUS_SUCCEEDED) {
        std::cerr << "Error: " << PROGRAM_STATUS_MSG[header->status_code] << "\n";
        return 1;
    }

    char* output_stream = new char[header->height * header->width];
    if (!output_stream) {
        std::cerr << "Error: " << PROGRAM_STATUS_MSG[MEMORY_ALLOC_ERR_MSG] << "\n";
        return 1;
    }

    parse_image_body(input_stream, header, image_parsing_progress, output_stream);

    std::cout << "Image pixels: " << std::endl;
    for (unsigned int i = 0; i < header->height; i++) {
        for (unsigned int j = 0; j < header->width; j++) {
            unsigned int index = i * header->width + j;
            std::cout << (unsigned int) output_stream[index] << " ";
        }
        std::cout << std::endl;
    }
    std::cout << "\n";

    free(header);
    delete[] input_stream;
    delete[] output_stream;

    return 0;
}
