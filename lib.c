//Small library that simulates a typical image decoding library like libjpeg

#include <string.h>
#include <stdlib.h>
#include "lib.h"

// Parse just the header from the input bytes
ImageHeader* parse_image_header(char* in) {
    // Return data to simulate parsing of a header
    ImageHeader* header = (ImageHeader*) malloc(sizeof(ImageHeader));
    header->status_code = 0;
    header->width = 10;
    header->height = 10;
    return header;
}

void parse_image_body(char* in, ImageHeader* header, OnProgress* on_progress, char* out) {
    // Simulate progress of parsing of image body
    for (unsigned int i = 1; i <= 100; i++) {
        on_progress(i);
    }

    // Simulate a write
    memset(out, 2, header->width * header->height);
}