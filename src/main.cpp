#include <iostream>
#include "club_manager.hpp"

int main(int argc, char **argv) {
    if (argc < 2) {
        std::cerr << "No input file provided" << std::endl;
        return 1;
    }
    ClubManager manager(argv[1]);
    manager.process();
}
