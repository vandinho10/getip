#include "../getip.hpp"
#include "test_framework.hpp"

#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>

// --- element_text -----------------------------------------------------------

TEST("element_text: id com espaço dentro das aspas (padrão do roteador)")
{
    const std::string html =
        "<dl><dt>WAN:</dt>\n"
        "<dd id= \"TIPv4\" class=\"success\"><span>IPv4</span> 203.0.113.42/255.255.255.0 </dd>"
        "</dl>";
    const std::string r = getip::element_text(html, "TIPv4");
    CHECK(r == "IPv4 203.0.113.42/255.255.255.0");
}

TEST("element_text: id sem espaços")
{
    const std::string html = "<dd id=\"TIPv4\">10.0.0.1/24</dd>";
    CHECK(getip::element_text(html, "TIPv4") == "10.0.0.1/24");
}

TEST("element_text: aspas simples")
{
    const std::string html = "<dd id='TIPv4' data-x='1'>8.8.8.8</dd>";
    CHECK(getip::element_text(html, "TIPv4") == "8.8.8.8");
}

TEST("element_text: id não encontrado retorna vazio")
{
    const std::string html = "<dd id=\"TLine1\">x</dd>";
    CHECK(getip::element_text(html, "TIPv4").empty());
}

TEST("element_text: ignora data-id de outros elementos")
{
    const std::string html =
        "<div data-id=\"TIPv4\">1.1.1.1</div><dd id=\"TIPv4\">2.2.2.2</dd>";
    CHECK(getip::element_text(html, "TIPv4") == "2.2.2.2");
}

TEST("element_text: seleciona o elemento correto entre vários")
{
    const std::string html =
        "<dd id=\"A\">1.1.1.1</dd><dd id=\"TIPv4\">2.2.2.2</dd><dd id=\"B\">3.3.3.3</dd>";
    CHECK(getip::element_text(html, "TIPv4") == "2.2.2.2");
}

TEST("element_text: conteúdo com quebras de linha e tags aninhadas")
{
    const std::string html =
        "<dd id= \"TIPv4\" class=\"success\">\n <span>IPv4</span>\n 10.1.2.3/24 \n</dd>";
    CHECK(getip::element_text(html, "TIPv4") == "IPv4 10.1.2.3/24");
}

TEST("element_text: valor com hífen e sublinhado")
{
    const std::string html = "<dd id=\"My-Elem_1\">7.7.7.7</dd>";
    CHECK(getip::element_text(html, "My-Elem_1") == "7.7.7.7");
}

// --- first_ipv4 -------------------------------------------------------------

TEST("first_ipv4: IPv4 com máscara")
{
    CHECK(getip::first_ipv4("IPv4 203.0.113.42/255.255.255.0") == "203.0.113.42");
}

TEST("first_ipv4: somente o IP")
{
    CHECK(getip::first_ipv4("10.0.0.1") == "10.0.0.1");
}

TEST("first_ipv4: primeiro de vários")
{
    CHECK(getip::first_ipv4("1.2.3.4 e 5.6.7.8") == "1.2.3.4");
}

TEST("first_ipv4: nenhum IP")
{
    CHECK(getip::first_ipv4("nenhum endereço aqui").empty());
}

TEST("first_ipv4: IPv6 não casa")
{
    CHECK(getip::first_ipv4("fe80::863c:99ff:fe2e:9e00").empty());
}

// --- strip_html --------------------------------------------------------------

TEST("strip_html: remove tags")
{
    CHECK(getip::strip_html("<span>IPv4</span> 203.0.113.42/255.255.255.0") ==
          "IPv4 203.0.113.42/255.255.255.0");
}

TEST("strip_html: decodifica entidades")
{
    CHECK(getip::strip_html("Linha&nbsp;1 &amp; 2") == "Linha 1 & 2");
}

TEST("strip_html: normaliza espaços e faz trim")
{
    CHECK(getip::strip_html("  ab   c  ") == "ab c");
}

TEST("strip_html: vazio")
{
    CHECK(getip::strip_html("").empty());
}

TEST("strip_html: somente tags")
{
    CHECK(getip::strip_html("<b><i></i></b>").empty());
}

// --- write_file --------------------------------------------------------------

TEST("write_file: grava e relê conteúdo")
{
    const std::string path =
        (std::filesystem::temp_directory_path() / "getip_test_out.txt").string();
    std::string error;
    CHECK(getip::write_file(path, "203.0.113.42", error));
    std::ifstream in(path);
    const std::string content((std::istreambuf_iterator<char>(in)),
                              std::istreambuf_iterator<char>());
    CHECK(content == "203.0.113.42");
    std::filesystem::remove(path);
}

TEST("write_file: falha em diretório inexistente")
{
    const std::string path = "/nao/existe/getip.txt";
    std::string error;
    CHECK(!getip::write_file(path, "x", error));
    CHECK(!error.empty());
}

// --- parse_args --------------------------------------------------------------

TEST("parse_args: valores padrão")
{
    char *argv[] = {const_cast<char *>("getip")};
    getip::Config cfg;
    CHECK(getip::parse_args(1, argv, cfg));
    CHECK(cfg.url == "https://192.168.10.1");
    CHECK(cfg.element_id == "TIPv4");
    CHECK(cfg.output_file == "TIPv4_element.txt");
    CHECK(cfg.log_level == "INFO");
    CHECK(cfg.timeout_ms == 15000);
    CHECK(!cfg.verify_tls);
}

TEST("parse_args: flags completas")
{
    char *argv[] = {
        const_cast<char *>("getip"),
        const_cast<char *>("--url"), const_cast<char *>("http://10.0.0.1"),
        const_cast<char *>("--element-id"), const_cast<char *>("Tv6LLA"),
        const_cast<char *>("--output-dir"), const_cast<char *>("/tmp/x"),
        const_cast<char *>("--output-file"), const_cast<char *>("ip.txt"),
        const_cast<char *>("--log-level"), const_cast<char *>("DEBUG"),
        const_cast<char *>("--timeout"), const_cast<char *>("5"),
        const_cast<char *>("--verify-tls"),
    };
    getip::Config cfg;
    CHECK(getip::parse_args(14, argv, cfg));
    CHECK(cfg.url == "http://10.0.0.1");
    CHECK(cfg.element_id == "Tv6LLA");
    CHECK(cfg.output_dir == "/tmp/x");
    CHECK(cfg.output_file == "ip.txt");
    CHECK(cfg.log_level == "DEBUG");
    CHECK(cfg.timeout_ms == 5000);
    CHECK(cfg.verify_tls);
}

TEST("parse_args: --help e --version")
{
    char *argv[] = {const_cast<char *>("getip"), const_cast<char *>("--help")};
    getip::Config cfg;
    CHECK(getip::parse_args(2, argv, cfg));
    CHECK(cfg.help);
}

TEST("parse_args: opção desconhecida")
{
    char *argv[] = {const_cast<char *>("getip"), const_cast<char *>("--nao-existe")};
    getip::Config cfg;
    CHECK(!getip::parse_args(2, argv, cfg));
}

TEST("parse_args: valor ausente")
{
    char *argv[] = {const_cast<char *>("getip"), const_cast<char *>("--url")};
    getip::Config cfg;
    CHECK(!getip::parse_args(2, argv, cfg));
}

TEST("parse_args: timeout inválido")
{
    char *argv[] = {
        const_cast<char *>("getip"),
        const_cast<char *>("--timeout"), const_cast<char *>("abc"),
    };
    getip::Config cfg;
    CHECK(!getip::parse_args(3, argv, cfg));
}

// --- configuração persistente ------------------------------------------------

#include <chrono>
#include <cstdio>
#include <sstream>
#include <vector>

namespace fs = std::filesystem;

struct TempDirCfg
{
    fs::path dir;
    TempDirCfg()
    {
        const auto tick =
            std::chrono::steady_clock::now().time_since_epoch().count();
        dir = fs::temp_directory_path() / ("getip_test_tmp_" + std::to_string(tick));
        fs::create_directories(dir);
    }
    ~TempDirCfg() { std::error_code ec; fs::remove_all(dir, ec); }
};

static void escrever_cfg(const fs::path &p, const std::string &conteudo)
{
    std::ofstream f(p);
    f << conteudo;
}

static int rodar_config(std::vector<std::string> args)
{
    std::vector<char *> argv;
    argv.push_back(const_cast<char *>("getip"));
    for (auto &s : args)
        argv.push_back(const_cast<char *>(s.c_str()));
    return getip::comando_config(static_cast<int>(argv.size()), argv.data());
}

TEST("NivelConfig: define/busca/acrescenta/remove")
{
    getip::NivelConfig n;
    CHECK(n.vazia());
    CHECK(n.busca("url") == nullptr);

    n.define("url", {"https://10.0.0.1"});
    CHECK(!n.vazia());
    const auto *u = n.busca("url");
    CHECK(u != nullptr);
    if (u == nullptr)
        return;
    CHECK(u->size() == 1 && (*u)[0] == "https://10.0.0.1");

    n.acrescenta("log-level", {"DEBUG"});
    n.define("url", {"http://x"});
    u = n.busca("url");
    if (u != nullptr)
        CHECK(u->size() == 1 && (*u)[0] == "http://x");

    n.remove("url");
    n.remove("inexistente"); // sem efeito
    CHECK(n.busca("url") == nullptr);
}

TEST("chave_valida aceita exatamente as 8 chaves")
{
    CHECK(getip::chave_valida("url"));
    CHECK(getip::chave_valida("element-id"));
    CHECK(getip::chave_valida("output-dir"));
    CHECK(getip::chave_valida("output-file"));
    CHECK(getip::chave_valida("log-file"));
    CHECK(getip::chave_valida("log-level"));
    CHECK(getip::chave_valida("timeout"));
    CHECK(getip::chave_valida("verify-tls"));
    CHECK(!getip::chave_valida("outra"));
    CHECK(!getip::chave_valida(""));
}

TEST("valor_bool interpreta variantes e cai no padrao")
{
    CHECK(getip::valor_bool("true", false));
    CHECK(getip::valor_bool("TRUE", false));
    CHECK(getip::valor_bool(" 1 ", true));
    CHECK(getip::valor_bool("sim", false));
    CHECK(!getip::valor_bool("false", true));
    CHECK(!getip::valor_bool("off", true));
    CHECK(!getip::valor_bool("nao", true));
    CHECK(getip::valor_bool("lixo", true));   // padrão
    CHECK(!getip::valor_bool("lixo", false)); // padrão
}

TEST("caminho_config_global respeita GETIP_CONFIG_HOME")
{
#ifdef _WIN32
    _putenv_s("GETIP_CONFIG_HOME", "C:\\sandbox");
#else
    setenv("GETIP_CONFIG_HOME", "/sandbox", 1);
#endif
    const fs::path p = getip::caminho_config_global();
#ifdef _WIN32
    CHECK(p == fs::path("C:\\sandbox") / "getip" / "config.txt");
#else
    CHECK(p == fs::path("/sandbox") / "getip" / "config");
#endif
#ifdef _WIN32
    _putenv_s("GETIP_CONFIG_HOME", "");
#else
    unsetenv("GETIP_CONFIG_HOME");
#endif
}

TEST("descobrir_config_local sobe a hierarquia ate encontrar .getip.conf")
{
    TempDirCfg raiz;
    fs::create_directories(raiz.dir / "a" / "b");
    escrever_cfg(raiz.dir / ".getip.conf", "url=http://roteador\n");

    CHECK(!getip::descobrir_config_local(raiz.dir / "a" / "b").empty());
    CHECK(getip::descobrir_config_local(raiz.dir / "a" / "b").filename() == ".getip.conf");
    CHECK(getip::descobrir_config_local(raiz.dir).filename() == ".getip.conf");

    TempDirCfg sem_config;
    CHECK(getip::descobrir_config_local(sem_config.dir).empty());
}

TEST("carregar_nivel: formato chave=valor, comentarios e lixo ignorado")
{
    TempDirCfg td;
    escrever_cfg(td.dir / "cfg",
                 "# comentario\n"
                 "\n"
                 "timeout=30\n"
                 " log-level = DEBUG \n"
                 "sem-igual\n"
                 "desconhecida=x\n"
                 "verify-tls=true\n");

    std::ostringstream vazio;
    auto *antigo = std::cerr.rdbuf(vazio.rdbuf());
    const getip::NivelConfig n = getip::carregar_nivel(td.dir / "cfg");
    std::cerr.rdbuf(antigo);

    const auto *t = n.busca("timeout");
    CHECK(t != nullptr);
    if (t == nullptr)
        return;
    CHECK((*t)[0] == "30");

    const auto *l = n.busca("log-level");
    CHECK(l != nullptr);
    if (l == nullptr)
        return;
    CHECK((*l)[0] == "DEBUG");

    CHECK(n.busca("desconhecida") == nullptr);
    const auto *v = n.busca("verify-tls");
    CHECK(v != nullptr);
    if (v != nullptr)
        CHECK(getip::valor_bool((*v)[0], false) == true);
}

TEST("salvar/carregar roundtrip deterministico com valor contendo '='")
{
    TempDirCfg td;
    getip::NivelConfig n;
    n.define("url", {"https://r/?a=b"});
    n.define("timeout", {"45"});
    CHECK(getip::salvar_nivel(td.dir / "cfg", n));

    std::ifstream in(td.dir / "cfg");
    const std::string bruto((std::istreambuf_iterator<char>(in)),
                            std::istreambuf_iterator<char>());
    CHECK(bruto.find("url=https://r/?a=b") != std::string::npos);
    CHECK(bruto.find("# getip") != std::string::npos);

    const getip::NivelConfig lido = getip::carregar_nivel(td.dir / "cfg");
    // chaves gravadas em ordem alfabetica: timeout antes de url
    CHECK(bruto.find("timeout=") < bruto.find("url="));
}

TEST("mesclar_niveis: local sobrepoe global por chave inteira")
{
    getip::NivelConfig g, l;
    g.define("url", {"https://global"});
    g.define("timeout", {"15"});
    l.define("url", {"https://local"});

    const getip::NivelConfig m = getip::mesclar_niveis(g, l);
    const auto *u = m.busca("url");
    CHECK(u != nullptr);
    if (u == nullptr)
        return;
    CHECK(u->size() == 1 && (*u)[0] == "https://local");
    const auto *t = m.busca("timeout");
    CHECK(t != nullptr);
    if (t == nullptr)
        return;
    CHECK((*t)[0] == "15");
}

TEST("aplicar_config: preenche apenas opcoes ausentes na CLI")
{
    char *argv[] = {const_cast<char *>("getip"),
                    const_cast<char *>("--url"), const_cast<char *>("http://cli"),
                    const_cast<char *>("--verify-tls")};
    getip::Config cfg;
    CHECK(getip::parse_args(4, argv, cfg));

    getip::NivelConfig efetivo;
    efetivo.define("url", {"https://da-config"});   // ignorado: CLI vence
    efetivo.define("element-id", {"Tv6LLA"});       // aplicado
    efetivo.define("timeout", {"60"});              // aplicado
    efetivo.define("verify-tls", {"false"});        // ignorado: CLI vence
    getip::aplicar_config(cfg, efetivo);

    CHECK(cfg.url == "http://cli" && cfg.cli_url);
    CHECK(cfg.element_id == "Tv6LLA");
    CHECK(cfg.timeout_ms == 60000);
    CHECK(cfg.verify_tls); // flag da CLI permanece
    CHECK(cfg.log_level == "INFO");
}

TEST("aplicar_config: timeout invalido na config e ignorado com aviso")
{
    getip::Config cfg;
    getip::NivelConfig efetivo;
    efetivo.define("timeout", {"abc"});
    std::ostringstream vazio;
    auto *antigo = std::cerr.rdbuf(vazio.rdbuf());
    getip::aplicar_config(cfg, efetivo);
    std::cerr.rdbuf(antigo);
    CHECK(cfg.timeout_ms == 15000); // mantém o padrão
}

TEST("comando_config: ciclo set -> get -> list -> unset em escopo global sandbox")
{
#ifdef _WIN32
    TempDirCfg td;
    std::string home = td.dir.string();
    _putenv_s("GETIP_CONFIG_HOME", home.c_str());
#else
    TempDirCfg td;
    setenv("GETIP_CONFIG_HOME", td.dir.string().c_str(), 1);
#endif

    CHECK(rodar_config({"config", "set", "url", "https://roteador.local"}) == EXIT_SUCCESS);
    CHECK(fs::exists(getip::caminho_config_global()));

    CHECK(rodar_config({"config", "get", "url"}) == EXIT_SUCCESS);
    CHECK(rodar_config({"config", "list"}) == EXIT_SUCCESS);
    CHECK(rodar_config({"config", "path"}) == EXIT_SUCCESS);
    CHECK(rodar_config({"config", "get", "inexistente"}) == EXIT_FAILURE);
    CHECK(rodar_config({"config", "set", "chave_mala", "x"}) == EXIT_FAILURE);
    CHECK(rodar_config({"config", "verbo_falso"}) == EXIT_FAILURE);

    CHECK(rodar_config({"config", "unset", "url"}) == EXIT_SUCCESS);
    CHECK(rodar_config({"config", "get", "url"}) == EXIT_FAILURE);

#ifdef _WIN32
    _putenv_s("GETIP_CONFIG_HOME", "");
#else
    unsetenv("GETIP_CONFIG_HOME");
#endif
}

TEST("comando_config: --local grava ./.getip.conf no diretorio atual")
{
#ifdef _WIN32
    TempDirCfg td;
    std::string home = td.dir.string();
    _putenv_s("GETIP_CONFIG_HOME", home.c_str());
#else
    TempDirCfg td;
    setenv("GETIP_CONFIG_HOME", td.dir.string().c_str(), 1);
#endif

    const fs::path original = fs::current_path();
    std::error_code ec;
    fs::current_path(td.dir, ec);
    if (ec)
        return;

    CHECK(rodar_config({"config", "--local", "set", "log-level", "DEBUG"}) == EXIT_SUCCESS);
    CHECK(fs::exists(td.dir / ".getip.conf"));

    const getip::NivelConfig lido = getip::carregar_nivel(td.dir / ".getip.conf");
    const auto *v = lido.busca("log-level");
    CHECK(v != nullptr);
    if (v == nullptr)
        return;
    CHECK((*v)[0] == "DEBUG");
    CHECK(getip::descobrir_config_local(td.dir) == td.dir / ".getip.conf");

    fs::current_path(original, ec);

#ifdef _WIN32
    _putenv_s("GETIP_CONFIG_HOME", "");
#else
    unsetenv("GETIP_CONFIG_HOME");
#endif
}

int main()
{
    return tests::run_all();
}
