#ifndef LIB_HELLO_H
#define LIB_HELLO_H

#include <string>

namespace scout
{

    std::string get_greet(const std::string &thing);

    // Declare the function that will be exported to JavaScript.
    extern "C"
    {
        int add(int a, int b);

        int infer();
    }

}

#endif // LIB_HELLO_H