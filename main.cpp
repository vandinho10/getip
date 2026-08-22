#include "getip.hpp"

#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>

int main(int argc, char *argv[])
{
    // Subcomando de configuração persistente (estilo git config).
    if (argc > 1 && std::string(argv[1]) == "config")
        return getip::comando_config(argc, argv);

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

    // Precedência: CLI > .getip.conf local > config global > defaults.
    const getip::NivelConfig global =
        getip::carregar_nivel(getip::caminho_config_global());
    const std::filesystem::path local_path =
        getip::descobrir_config_local(std::filesystem::current_path());
    const getip::NivelConfig local = getip::carregar_nivel(local_path);
    getip::aplicar_config(cfg, getip::mesclar_niveis(global, local));

    return getip::run(cfg);
}
