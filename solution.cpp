#include "lib.h"
#include <assert.h>
#include <stdlib.h>
#include <iostream>

// Configure RLBox
#define RLBOX_SINGLE_THREADED_INVOCATIONS

// Configure RLBox for noop sandbox
#define RLBOX_USE_STATIC_CALLS() rlbox_noop_sandbox_lookup_symbol
#include "rlbox_noop_sandbox.hpp"

using sandbox_type_t = rlbox::rlbox_noop_sandbox;

#include "rlbox.hpp"
using namespace rlbox;

template<typename T>
using tainted_img = rlbox::tainted<T, sandbox_type_t>;

// Define and load any structs needed by the application
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

void image_parsing_progress(rlbox_sandbox<sandbox_type_t>& sandbox, tainted_img<unsigned int> progress) {
    auto checked_progress = progress.copy_and_verify([](unsigned int value) {
        // progress is expected to be between 1 and 100, so we check this
        // However, in this case, even if we didn't check for this condition,
        // and the library returned an out of range value like 1000, no memory
        // safety issue will occur. The application would just print a very
        // confusing message "Image parsing: 1000 out of 100".
        assert(value >=1 && value <= 100);
        return value;
    });
    std::cout << "Image parsing: " << checked_progress << " out of 100\n";
}

void get_image_bytes(char* input_stream) {
    // Get the bytes of the image from the file into input stream
    // This is just a toy example, so we will leave this empty for now
}

// An example application that simulates a typical image parsing program
// The library simulates a typilcal image decoding library such as libjpeg
int main(int argc, char const *argv[])
{
    // create an rlbox sandbox
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

    // Create a buffer that will hold the input bytes inside the sandbox
    auto tainted_input_stream = sandbox.malloc_in_sandbox<char>(100);
    if (!input_stream) {
        std::cerr << "Error: " << PROGRAM_STATUS_MSG[MEMORY_ALLOC_ERR_MSG] << "\n";
        return 1;
    }
    // Copy the input bytes into the buffer inside the sandbox
    rlbox::memcpy(sandbox, tainted_input_stream, input_stream, 100u);

    // Parse header of the image to get its dimensions
    // invoke the function inside the sandbox
    // We pass in the tainted_input_stream instead of input_stream
    // This is because the sandbox cannot access input_stream since it is in
    // the application's memory
    auto header = sandbox_invoke(sandbox, parse_image_header, tainted_input_stream);

    // We make a copy of the tainted status_code in a local variable
    // This is good practice since we are reading it twice below
    // Making a copy will prevent time of check time of use style attacks
    tainted_img<unsigned int> tainted_status_code = header->status_code;

    // Below we check an error code and early exit.
    // Note the result of checking of a tainted value is itself tainted
    // But in this case we can remove the tainting since the *worst* thing that
    // can happen is that we just safely exit the program.
    // We use unverified_safe_because to remove tainting. It takes a string as
    // a parameter which is basically just a comment (the string is ignored,
    // but required for documentation).
    if ((tainted_status_code != HEADER_PARSING_STATUS_SUCCEEDED)
        .unverified_safe_because("Worst case we will early exit"))
    {
        // We index an array with tainted_status_code so it needs to be verified else we get a compile error
        auto verified_status_code = tainted_status_code.copy_and_verify([](unsigned int value){
            // since status code is being used to index an array below, we need
            // to make sure that the value is less than that array's size
            auto program_status_length = sizeof(PROGRAM_STATUS_MSG)/sizeof(PROGRAM_STATUS_MSG[0]);
            // just abort if the value is out of bounds
            assert(value < program_status_length);
            // if safe, return the value
            return value;
        });
        std::cerr << "Error: " << PROGRAM_STATUS_MSG[verified_status_code] << "\n";
        return 1;
    }

    // Again width and height are used multiple times in the below code, so let's copy it a local variable first
    tainted_img<unsigned int> tainted_height = header->height;
    tainted_img<unsigned int> tainted_width = header->width;

    // we need to allocate output_stream inside the sandbox so that it is accessible to the sandbox
    // let's also rename it to tainted_output_stream
    // we need to untaint the expression tainted_height * tainted_width
    // Since this is being used only for an allocation pretty much any value is safe
    // In fact, this is fine even if the multiply results in an integer overflow
    // Alternately, if our application assumes there is a maximum size of the image, we should check this value here.
    auto output_size = (tainted_height * tainted_width).unverified_safe_because("Any value is safe for allocation");
    auto tainted_output_stream = sandbox.malloc_in_sandbox<char>(output_size);
    if (!tainted_output_stream) {
        std::cerr << "Error: " << PROGRAM_STATUS_MSG[MEMORY_ALLOC_ERR_MSG] << "\n";
        return 1;
    }

    // we need to pass a callback to parse_image_body, so we register it here
    auto cb_image_parsing_progress = sandbox.register_callback(image_parsing_progress);

    // invoke via sandbox_invoke and pass in tainted versions of the paramters
    sandbox_invoke(sandbox, parse_image_body, tainted_input_stream, header, cb_image_parsing_progress, tainted_output_stream);

    std::cout << "Image pixels: " << std::endl;
    // Loop now iterates over the tainted height and width of the image
    // Thus the loop exit condition "i < tainted_height" and "j < tainted_width"
    // return tainted values. We need to untaint them.
    // Untainting these variables gives the library influence over how many
    // loop iterations are executed.
    // On inspection, we can see that this loop is safe no matter how many iterations are run
    // So we can remove the tainting
    for (auto i = 0; i < tainted_height.unverified_safe_because("safe for any value"); i++) {
        for (auto j = 0; j < tainted_width.unverified_safe_because("safe for any value"); j++) {
            auto index = i * tainted_width + j;
            // note the expression "tainted_output_stream[index]" indexes an
            // array with a tainted value here. But the array itself is tainted.
            // When indexing tainted arrays, rlbox inserts a dynamic check to
            // ensure the memory access is restricted to the sandbox.
            // Thus there is no compile error, and no further verification
            // needed.
            std::cout << (unsigned int) tainted_output_stream[index].unverified_safe_because("pixel value can be anything") << " ";
        }
        std::cout << std::endl;
    }
    std::cout << "\n";

    sandbox.free_in_sandbox(header);
    delete[] input_stream;
    sandbox.free_in_sandbox(tainted_input_stream);
    sandbox.free_in_sandbox(tainted_output_stream);

    cb_image_parsing_progress.unregister();
    sandbox.destroy_sandbox();

    return 0;
}
