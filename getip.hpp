#ifndef GETIP_HPP
#define GETIP_HPP

#include <string>

#ifndef GETIP_VERSION
#define GETIP_VERSION "1.0.1"
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

} // namespace getip

#endif // GETIP_HPP
