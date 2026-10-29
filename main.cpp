#include "getip.hpp"
#include "selfupdate.hpp"

#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>

int main(int argc, char *argv[])
{
    // Subcomando de configuração persistente (estilo git config).
    if (argc > 1 && std::string(argv[1]) == "config")
        return getip::comando_config(argc, argv);

    // Autoatualização via GitHub Releases (SHA256 obrigatório).
    if (argc > 1 && std::string(argv[1]) == "update")
    {
        selfupdate::Contexto ctx;
        ctx.app = "getip";
        ctx.versao = GETIP_VERSION;
        ctx.repo_padrao = "vandinho10/getip";
        ctx.env_repo = "GETIP_UPDATE_REPO";
        ctx.http_get = [](const std::string &url, std::string &erro) {
            getip::Config c;
            c.url = url;
            c.verify_tls = true;
            c.timeout_ms = 120000;
            return getip::fetch_page(c, erro);
        };
        return selfupdate::comando_update(ctx, argc, argv, 2);
    }

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
