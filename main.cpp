#include "metagraph.h"

int main(int argc, char *argv[]) {
    if(argc == 3) {
        metagraph::Metagraph g((std::string(argv[1])));
        g.computeAttributes(argv[2]);
    } else {
        std::cout << "usage: Rhythm_annotated_metagraph <input file path> <output file path>";
    }
    return 0;
}
