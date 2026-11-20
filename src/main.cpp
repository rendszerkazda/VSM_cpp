#include <iostream>
#include <string>
#include <vector>

#include "vsm/App.h"

int main(int argc, char* argv[]) {
    const std::vector<std::string> args(argv + 1, argv + argc);
    return vsm::runMain(args, std::cout, std::cerr);
}
