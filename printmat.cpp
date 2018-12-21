#include "distmat.h"
#include <cstdio>
#include <getopt.h>
#include <string>

int print_binary_main(int argc, char *argv[]) {
    int c;
    //bool use_float = false; Automatically detected, not needed.
    bool use_scientific = false;
    std::string outpath;
    for(char **p(argv); *p; ++p) if(std::strcmp(*p, "-h") && std::strcmp(*p, "--help") == 0) goto usage;
    if(argc == 1) {
        usage:
        std::fprintf(stderr, "%s <path to binary file> [- to read from stdin]\n-s: Emit in scientific notation.\n", argv ? static_cast<const char *>(*argv): "dashing");
    }
    while((c = getopt(argc, argv, ":o:sh?")) >= 0) {
        switch(c) {
            case 'o': outpath = optarg; break;
            case 's': use_scientific = true; break;
            case 'h': case '?': goto usage;
        }
    }
    std::FILE *fp;
    if(outpath.empty()) outpath = "/dev/stdout";
#define INNER(type) \
        dm::DistanceMatrix<type> mat(argv[optind]);\
        std::fprintf(stderr, "Name of found: %s\n", dm::DistanceMatrix<type>::magic_string());\
        if((fp = std::fopen(outpath.data(), "wb")) == nullptr) throw std::runtime_error(std::string("Could not open file at" + outpath));\
        mat.printf(fp, use_scientific);
    try {
        INNER(float);
    } catch(const std::runtime_error &re) {
        INNER(double);
    }
    std::fclose(fp);
    return EXIT_SUCCESS;
}
