#include "getip.hpp"

#include <curl/curl.h>

#include <algorithm>
#include <chrono>
#include <cctype>
#include <cstdlib>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <mutex>
#include <regex>
#include <string>
#include <utility>
#include <vector>

namespace getip
{

namespace
{

// ---------------------------------------------------------------------------
// Logger mínimo (console + arquivo) com níveis DEBUG/INFO/WARNING/ERROR.
// ---------------------------------------------------------------------------

enum class Level
{
    DEBUG = 0,
    INFO = 1,
    WARNING = 2,
    ERROR = 3
};

Level level_from_string(const std::string &name)
{
    std::string upper = name;
    std::transform(upper.begin(), upper.end(), upper.begin(),
                   [](unsigned char c) { return static_cast<char>(std::toupper(c)); });
    if (upper == "DEBUG")
        return Level::DEBUG;
    if (upper == "WARNING" || upper == "WARN")
        return Level::WARNING;
    if (upper == "ERROR")
        return Level::ERROR;
    return Level::INFO;
}

std::string level_to_string(Level lvl)
{
    switch (lvl)
    {
    case Level::DEBUG:
        return "DEBUG";
    case Level::INFO:
        return "INFO";
    case Level::WARNING:
        return "WARNING";
    case Level::ERROR:
        return "ERROR";
    }
    return "INFO";
}

std::string timestamp()
{
    const auto now = std::chrono::system_clock::now();
    const std::time_t t = std::chrono::system_clock::to_time_t(now);
    std::tm tm{};
#if defined(_WIN32)
    localtime_s(&tm, &t);
#else
    localtime_r(&t, &tm);
#endif
    char buf[32];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &tm);
    return buf;
}

class Logger
{
public:
    static Logger &instance()
    {
        static Logger logger;
        return logger;
    }

    void configure(const std::string &level_name, const std::string &log_file)
    {
        min_level_ = level_from_string(level_name);
        if (!log_file.empty())
        {
            std::error_code ec;
            std::filesystem::create_directories(std::filesystem::path(log_file).parent_path(), ec);
            file_.open(log_file, std::ios::out | std::ios::app);
        }
    }

    void log(Level lvl, const std::string &msg)
    {
        if (static_cast<int>(lvl) < static_cast<int>(min_level_))
            return;
        std::lock_guard<std::mutex> lock(mtx_);
        const std::string line = timestamp() + " - " + level_to_string(lvl) + " - " + msg;
        std::cout << line << "\n";
        std::cout.flush();
        if (file_.is_open())
            file_ << line << "\n" << std::flush;
    }

    void debug(const std::string &msg) { log(Level::DEBUG, msg); }
    void info(const std::string &msg) { log(Level::INFO, msg); }
    void warning(const std::string &msg) { log(Level::WARNING, msg); }
    void error(const std::string &msg) { log(Level::ERROR, msg); }

private:
    std::mutex mtx_;
    std::ofstream file_;
    Level min_level_ = Level::INFO;
};

// ---------------------------------------------------------------------------
// Helpers de parsing de HTML.
// ---------------------------------------------------------------------------

size_t find_tag_start(const std::string &html, size_t from)
{
    size_t i = from;
    while (i > 0 && html[i] != '<')
        --i;
    return html[i] == '<' ? i : std::string::npos;
}

std::string tag_name(const std::string &html, size_t tag_start)
{
    size_t p = tag_start + 1;
    while (p < html.size() && std::isalpha(static_cast<unsigned char>(html[p])))
        ++p;
    return html.substr(tag_start + 1, p - tag_start - 1);
}

size_t find_tag_close(const std::string &html, size_t tag_start)
{
    bool in_dq = false;
    bool in_sq = false;
    for (size_t i = tag_start + 1; i < html.size(); ++i)
    {
        const char c = html[i];
        if (c == '"' && !in_sq)
            in_dq = !in_dq;
        else if (c == '\'' && !in_dq)
            in_sq = !in_sq;
        else if (c == '>' && !in_dq && !in_sq)
            return i;
    }
    return std::string::npos;
}

size_t write_callback(char *ptr, size_t size, size_t nmemb, void *userdata)
{
    const size_t total = size * nmemb;
    static_cast<std::string *>(userdata)->append(ptr, total);
    return total;
}

std::string dirname_of(const std::string &path)
{
    const std::filesystem::path p(path);
    return p.has_parent_path() ? p.parent_path().string() : std::string{};
}

void apply_env(Config &cfg)
{
    if (const char *v = std::getenv("LOG_FILE_PATH"); v != nullptr && *v != '\0')
    {
        cfg.log_file = v;
        if (cfg.output_dir.empty())
            cfg.output_dir = dirname_of(v);
    }
    if (const char *v = std::getenv("LOG_LEVEL"); v != nullptr && *v != '\0')
        cfg.log_level = v;
    if (const char *v = std::getenv("GETIP_URL"); v != nullptr && *v != '\0')
        cfg.url = v;
}

} // namespace

// ---------------------------------------------------------------------------
// Opções de linha de comando.
// ---------------------------------------------------------------------------

bool parse_args(int argc, char *argv[], Config &cfg)
{
    for (int i = 1; i < argc; ++i)
    {
        const std::string arg = argv[i];
        auto need_value = [&]() { return i + 1 < argc; };

        if (arg == "--help" || arg == "-h")
        {
            cfg.help = true;
        }
        else if (arg == "--version" || arg == "-V")
        {
            cfg.version = true;
        }
        else if (arg == "--url")
        {
            if (!need_value())
                return false;
            cfg.url = argv[++i];
            cfg.cli_url = true;
        }
        else if (arg == "--element-id")
        {
            if (!need_value())
                return false;
            cfg.element_id = argv[++i];
            cfg.cli_element_id = true;
        }
        else if (arg == "--output-dir")
        {
            if (!need_value())
                return false;
            cfg.output_dir = argv[++i];
            cfg.cli_output_dir = true;
        }
        else if (arg == "--output-file")
        {
            if (!need_value())
                return false;
            cfg.output_file = argv[++i];
            cfg.cli_output_file = true;
        }
        else if (arg == "--log-file")
        {
            if (!need_value())
                return false;
            cfg.log_file = argv[++i];
            cfg.cli_log_file = true;
        }
        else if (arg == "--log-level")
        {
            if (!need_value())
                return false;
            cfg.log_level = argv[++i];
            cfg.cli_log_level = true;
        }
        else if (arg == "--timeout")
        {
            if (!need_value())
                return false;
            const std::string v = argv[++i];
            char *end = nullptr;
            const long seconds = std::strtol(v.c_str(), &end, 10);
            if (end == v.c_str() || seconds <= 0)
            {
                std::cerr << "getip: valor inválido para '--timeout': " << v << "\n";
                return false;
            }
            cfg.timeout_ms = seconds * 1000;
            cfg.cli_timeout = true;
        }
        else if (arg == "--verify-tls")
        {
            cfg.verify_tls = true;
            cfg.cli_verify_tls = true;
        }
        else
        {
            std::cerr << "getip: opção desconhecida '" << arg << "'\n";
            return false;
        }
    }
    return true;
}

void print_help(const char *prog)
{
    std::cout
        << "Uso: " << prog << " [opções]\n"
        << "\n"
        << "Coleta o IPv4 do roteador a partir do elemento HTML `#TIPv4` via HTTPS e grava\n"
        << "em arquivo texto (compatível com o legado get_ip.py do serviço chromium).\n"
        << "\n"
        << "Opções:\n"
        << "  --url <URL>            Página a consultar (padrão: https://192.168.10.1)\n"
        << "  --element-id <ID>      Id do elemento HTML (padrão: TIPv4)\n"
        << "  --output-dir <DIR>     Diretório de saída (padrão: ./data ou dir de LOG_FILE_PATH)\n"
        << "  --output-file <FILE>   Arquivo de saída (padrão: TIPv4_element.txt)\n"
        << "  --log-file <FILE>      Arquivo de log (padrão: ./data/get_ip.log ou LOG_FILE_PATH)\n"
        << "  --log-level <LVL>      DEBUG|INFO|WARNING|ERROR (padrão: INFO)\n"
        << "  --timeout <SEGUNDOS>   Timeout da requisição (padrão: 15)\n"
        << "  --verify-tls           Habilita verificação do certificado TLS (padrão: desabilitada)\n"
        << "  -h, --help             Exibe esta ajuda\n"
        << "  -V, --version          Exibe a versão\n"
        << "\n"
        << "Variáveis de ambiente: LOG_FILE_PATH, LOG_LEVEL, GETIP_URL.\n";
}

void print_version(const char *prog)
{
    std::cout << prog << " v" << GETIP_VERSION << "\n";
}

// ---------------------------------------------------------------------------
// HTTP (libcurl).
// ---------------------------------------------------------------------------

std::string fetch_page(const Config &cfg, std::string &error)
{
    CURL *curl = curl_easy_init();
    if (curl == nullptr)
    {
        error = "falha ao inicializar a libcurl";
        return {};
    }

    std::string body;
    char errbuf[CURL_ERROR_SIZE] = {0};

    curl_easy_setopt(curl, CURLOPT_URL, cfg.url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &body);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 5L);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT_MS, cfg.timeout_ms);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, cfg.timeout_ms);
    curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errbuf);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "getip/" GETIP_VERSION);
    if (!cfg.verify_tls)
    {
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
    }

    const CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK)
    {
        error = errbuf[0] != '\0' ? std::string(errbuf) : curl_easy_strerror(res);
        curl_easy_cleanup(curl);
        return {};
    }

    long status = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status);
    curl_easy_cleanup(curl);

    if (status < 200 || status >= 300)
    {
        error = "HTTP " + std::to_string(status);
        return {};
    }
    return body;
}

// ---------------------------------------------------------------------------
// Parsing do HTML e extração do IPv4.
// ---------------------------------------------------------------------------

std::string element_text(const std::string &html, const std::string &element_id)
{
    // Atributo `id` tolerante: permite aspas simples/duplas opcionais e
    // espaços internos (ex.: id= "TIPv4" presente na página do roteador).
    const std::regex id_re(R"(([^A-Za-z0-9_-])id\s*=\s*["']?\s*([A-Za-z0-9_-]+)\s*["']?)");
    for (std::sregex_iterator it(html.begin(), html.end(), id_re), end; it != end; ++it)
    {
        const std::smatch &m = *it;
        if (m[2].str() != element_id)
            continue;

        const size_t attr_pos = static_cast<size_t>(m.position()) + 1;
        const size_t start = find_tag_start(html, attr_pos);
        if (start == std::string::npos)
            continue;

        const std::string name = tag_name(html, start);
        const size_t tag_close = find_tag_close(html, start);
        if (tag_close == std::string::npos)
            continue;

        const std::string closing = "</" + name;
        const size_t close_start = html.find(closing, tag_close + 1);
        if (close_start == std::string::npos)
            continue;

        return strip_html(html.substr(tag_close + 1, close_start - (tag_close + 1)));
    }
    return {};
}

std::string strip_html(const std::string &raw)
{
    std::string out;
    bool in_tag = false;
    for (char c : raw)
    {
        if (c == '<')
        {
            in_tag = true;
            continue;
        }
        if (c == '>')
        {
            in_tag = false;
            continue;
        }
        if (!in_tag)
            out.push_back(c);
    }

    const std::vector<std::pair<std::string, std::string>> entities = {
        {"&nbsp;", " "}, {"&amp;", "&"}, {"&lt;", "<"}, {"&gt;", ">"},
        {"&quot;", "\""}, {"&#39;", "'"}};
    for (const auto &ent : entities)
    {
        std::string::size_type pos = 0;
        while ((pos = out.find(ent.first, pos)) != std::string::npos)
        {
            out.replace(pos, ent.first.size(), ent.second);
            pos += ent.second.size();
        }
    }

    std::string normalized;
    bool prev_space = false;
    for (char c : out)
    {
        if (std::isspace(static_cast<unsigned char>(c)))
        {
            if (!prev_space)
            {
                normalized.push_back(' ');
                prev_space = true;
            }
        }
        else
        {
            normalized.push_back(c);
            prev_space = false;
        }
    }

    const std::string::size_type b = normalized.find_first_not_of(' ');
    const std::string::size_type e = normalized.find_last_not_of(' ');
    if (b == std::string::npos)
        return "";
    return normalized.substr(b, e - b + 1);
}

std::string first_ipv4(const std::string &text)
{
    static const std::regex ipv4_re(R"((?:[0-9]{1,3}\.){3}[0-9]{1,3})");
    std::smatch m;
    if (std::regex_search(text, m, ipv4_re))
        return m[0].str();
    return {};
}

// ---------------------------------------------------------------------------
// Escrita de arquivo.
// ---------------------------------------------------------------------------

bool write_file(const std::string &path, const std::string &content, std::string &error)
{
    std::ofstream out(path, std::ios::out | std::ios::trunc);
    if (!out)
    {
        error = "falha ao abrir o arquivo para escrita: " + path;
        return false;
    }
    out << content;
    out.close();
    if (!out)
    {
        error = "falha ao gravar o arquivo: " + path;
        return false;
    }
    return true;
}

// ---------------------------------------------------------------------------
// Orquestração.
// ---------------------------------------------------------------------------

int run(const Config &cfg)
{
    Config c = cfg;
    apply_env(c);

    if (c.output_dir.empty())
        c.output_dir = c.log_file.empty() ? "./data" : dirname_of(c.log_file);
    if (c.output_dir.empty())
        c.output_dir = "./data";
    if (c.log_file.empty())
        c.log_file = (std::filesystem::path(c.output_dir) / "get_ip.log").string();

    const std::filesystem::path out_rel(c.output_file);
    const std::string out_path = out_rel.has_parent_path()
        ? c.output_file
        : (std::filesystem::path(c.output_dir) / out_rel).string();

    Logger::instance().configure(c.log_level, c.log_file);
    Logger &log = Logger::instance();

    std::error_code ec;
    std::filesystem::create_directories(std::filesystem::path(out_path).parent_path(), ec);
    if (ec)
    {
        log.error("falha ao criar o diretório de saída '" +
                  std::filesystem::path(out_path).parent_path().string() + "': " + ec.message());
        return EXIT_FAILURE;
    }

    log.debug("getip v" GETIP_VERSION);
    log.debug("URL alvo: " + c.url);

    std::string error;
    const std::string html = fetch_page(c, error);
    if (html.empty())
    {
        log.error("falha ao obter a página: " + error);
        return EXIT_FAILURE;
    }
    log.debug("página obtida com " + std::to_string(html.size()) + " bytes");

    const std::string text = element_text(html, c.element_id);
    if (text.empty())
    {
        log.error("elemento '" + c.element_id + "' não encontrado na página");
        return EXIT_FAILURE;
    }
    log.debug("texto bruto do elemento: '" + text + "'");

    const std::string ip = first_ipv4(text);
    if (ip.empty())
    {
        log.error("nenhum IPv4 válido encontrado no texto: '" + text + "'");
        return EXIT_FAILURE;
    }

    if (!write_file(out_path, ip, error))
    {
        log.error(error);
        return EXIT_FAILURE;
    }

    log.info("IP Encontrado: " + ip);
    log.debug("arquivo gravado: " + out_path);
    return EXIT_SUCCESS;
}

} // namespace getip
