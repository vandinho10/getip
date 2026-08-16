#include "getip.hpp"

#include <cstdlib>
#include <iostream>

int main(int argc, char *argv[])
{
    getip::Config cfg;
    if (!getip::parse_args(argc, argv, cfg))
    {
        getip::print_help(argv[0]);
        return EXIT_FAILURE;
    }

    if (cfg.help)
    {
        getip::print_help(argv[0]);
        return EXIT_SUCCESS;
    }

    if (cfg.version)
    {
        getip::print_version(argv[0]);
        return EXIT_SUCCESS;
    }

    return getip::run(cfg);
}
