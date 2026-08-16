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

int main()
{
    return tests::run_all();
}
