#include "lib.h"
#include <assert.h>
#include <stdlib.h>
#include <iostream>

/* Useful commands
Configure build:
----------------
cmake -S ./ -B ./build -DCMAKE_BUILD_TYPE=Debug

Build and run
------------------------------
cmake --build ./build --parallel --config Debug --target run
*/


// This is mostly boilerplate, so is in comments so you can easily use it in the course of the tutorial
// Include and configure rlbox.

// Configure RLBox
#define RLBOX_SINGLE_THREADED_INVOCATIONS

#define USE_NOOP
#ifndef USE_NOOP
    // Configure RLBox for noop sandbox
    #include "lib_wasm.h"
    #define RLBOX_USE_STATIC_CALLS() rlbox_wasm2c_sandbox_lookup_symbol
    #include "rlbox_wasm2c_sandbox.hpp"
    using sandbox_type_t = rlbox::rlbox_wasm2c_sandbox;
#else
    // Configure RLBox for noop sandbox
    #define RLBOX_USE_STATIC_CALLS() rlbox_noop_sandbox_lookup_symbol
    #include "rlbox_noop_sandbox.hpp"
    using sandbox_type_t = rlbox::rlbox_noop_sandbox;
#endif

#include "rlbox.hpp"
using namespace rlbox;

template<typename T>
using tainted_val = rlbox::tainted<T, sandbox_type_t>;

#define sandbox_fields_reflection_exampleapp_class_ImageHeader(f, g, ...)  \
f(unsigned int, status_code, FIELD_NORMAL, ##__VA_ARGS__) g()            \
f(unsigned int, width, FIELD_NORMAL, ##__VA_ARGS__) g()                  \
f(unsigned int, height, FIELD_NORMAL, ##__VA_ARGS__) g()

#define sandbox_fields_reflection_exampleapp_allClasses(f, ...)            \
  f(ImageHeader, exampleapp, ##__VA_ARGS__)

rlbox_load_structs_from_library(exampleapp);

static const char* PROGRAM_STATUS_MSG [] = {
    "Succeeded",
    "Invalid image",
    "Incomplete Image",
    "Memory allocation failure"
};

#define MEMORY_ALLOC_ERR_MSG 3

/**************** Codeblock2
// This is mostly boilerplate, so is in comments so you can easily use it in the course of the tutorial
//  Migrate the callback to use tainted types and take the sandbox as the first parameter

void image_parsing_progress(rlbox_sandbox<sandbox_type_t>& sandbox, tainted_val<unsigned int> progress) {
    auto checked_progress = progress.copy_and_verify([](unsigned int value) {
        assert(value >=1 && value <= 100);
        return value;
    });
    std::cout << "Image parsing: " << checked_progress << " out of 100\n";
}
******************/

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
    rlbox_sandbox<sandbox_type_t> sandbox;
    sandbox.create_sandbox();

    // create a buffer for input bytes
    char* input_stream = new char[100];
    if (!input_stream) {
        std::cerr << "Error: " << PROGRAM_STATUS_MSG[MEMORY_ALLOC_ERR_MSG] << "\n";
        return 1;
    }

    // Read bytes from an image file into input_stream
    get_image_bytes(input_stream);

    tainted_val<char*> tainted_input_stream = sandbox.malloc_in_sandbox<char>(100);
    rlbox::memcpy(sandbox, tainted_input_stream, input_stream, 100);

    // Parse header of the image to get its dimensions
    tainted_val<ImageHeader*> tainted_header = sandbox_invoke(sandbox, parse_image_header, tainted_input_stream);

    //UNSAFE alias
    ImageHeader* header = tainted_header.UNSAFE_unverified();

    // SECDEV CHECKPOINT 1

    if (header->status_code != HEADER_PARSING_STATUS_SUCCEEDED) {
        std::cerr << "Error: " << PROGRAM_STATUS_MSG[header->status_code] << "\n";
        return 1;
    }

    char* output_stream = new char[header->height * header->width];
    if (!output_stream) {
        std::cerr << "Error: " << PROGRAM_STATUS_MSG[MEMORY_ALLOC_ERR_MSG] << "\n";
        return 1;
    }

    // SECDEV CHECKPOINT 2

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

    /**************** Codeblock3
    // If we are short on time, we can directly look at the solution for this last bit

    std::cout << "Image pixels: " << std::endl;
    for (auto i = 0; i < tainted_header->height.unverified_safe_because("safe for any value"); i++) {
        for (auto j = 0; j < tainted_header->width.unverified_safe_because("safe for any value"); j++) {
            tainted_val<unsigned int> index = i * tainted_header->width + j;
            std::cout << (unsigned int) tainted_output_stream[index].unverified_safe_because("pixel value can be anything") << " ";
        }
        std::cout << std::endl;
    }
    std::cout << "\n";
    ******************/

    free(header);
    delete[] input_stream;
    sandbox.free_in_sandbox(tainted_input_stream);
    delete[] output_stream;

    sandbox.destroy_sandbox();

    return 0;
}
