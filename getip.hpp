#ifndef GETIP_HPP
#define GETIP_HPP

#include <filesystem>
#include <string>
#include <utility>
#include <vector>

#ifndef GETIP_VERSION
#define GETIP_VERSION "1.1.0"
#endif

namespace getip
{

struct Config
{
    std::string url = "https://192.168.10.1";
    std::string element_id = "TIPv4";
    std::string output_dir;
    std::string output_file = "TIPv4_element.txt";
    std::string log_file;
    std::string log_level = "INFO";
    long timeout_ms = 15000;
    bool verify_tls = false;
    bool help = false;
    bool version = false;

    // Opções efetivamente fornecidas na linha de comando (precedência
    // CLI > config local > config global > defaults).
    bool cli_url = false;
    bool cli_element_id = false;
    bool cli_output_dir = false;
    bool cli_output_file = false;
    bool cli_log_file = false;
    bool cli_log_level = false;
    bool cli_timeout = false;
    bool cli_verify_tls = false;
};

bool parse_args(int argc, char *argv[], Config &cfg);
void print_help(const char *prog);
void print_version(const char *prog);

std::string fetch_page(const Config &cfg, std::string &error);
std::string element_text(const std::string &html, const std::string &element_id);
std::string strip_html(const std::string &raw);
std::string first_ipv4(const std::string &text);
bool write_file(const std::string &path, const std::string &content, std::string &error);

int run(const Config &cfg);

// ------------------------- configuração persistente ------------------------

// Nível de configuração: conjunto ordenado de chaves com valores múltiplos.
struct NivelConfig
{
    std::vector<std::pair<std::string, std::vector<std::string>>> entradas;

    bool vazia() const;
    const std::vector<std::string> *busca(const std::string &chave) const;
    void define(const std::string &chave, std::vector<std::string> valores);
    void acrescenta(const std::string &chave, const std::vector<std::string> &valores);
    void remove(const std::string &chave);
};

// Chaves aceitas no arquivo de configuração.
bool chave_valida(const std::string &chave);

// Converte "true/1/sim/yes/on" (e negações) para bool; fora disso usa o padrão.
bool valor_bool(const std::string &valor, bool padrao);

// Global: $GETIP_CONFIG_HOME/getip/config (teste/sandbox) ou o diretório de
// config do usuário ($XDG_CONFIG_HOME|~/.config; %APPDATA% no Windows).
std::filesystem::path caminho_config_global();

// Local: primeiro arquivo `.getip.conf` a partir de `inicio` subindo até a
// raiz. Retorna caminho vazio se não houver.
std::filesystem::path descobrir_config_local(const std::filesystem::path &inicio);

NivelConfig carregar_nivel(const std::filesystem::path &arquivo);
bool salvar_nivel(const std::filesystem::path &arquivo, const NivelConfig &nivel);

// Local sobrepõe global por chave inteira (estilo git).
NivelConfig mesclar_niveis(const NivelConfig &global, const NivelConfig &local);

// Preenche em `cfg` as opções NÃO fornecidas na CLI.
void aplicar_config(Config &cfg, const NivelConfig &efetivo);

// Subcomando `getip config [get|set|add|unset|list|path]` — retorna código de
// saída. Escrita via --global (padrão) ou --local.
int comando_config(int argc, char *argv[]);

} // namespace getip

#endif // GETIP_HPP
