# getip — Coletor de IP do Roteador

**getip** é um utilitário de linha de comando em **C++17** que coleta o IPv4
do roteador (elemento HTML `#TIPv4`) via HTTPS e grava o endereço em arquivo
texto. Substitui o legado `get_ip.py` (Python + Selenium + Chromium) do
serviço `chromium`, produzindo a mesma saída sem a sobrecarga de um navegador.

## Por que substituir o Chromium?

O `get_ip.py` exigia um container com Chromium + ChromeDriver + Selenium
(~centenas de MB e alto consumo de CPU/RAM). O resultado útil é uma única
string IPv4 obtida por requisição HTTP. `getip` entrega exatamente a mesma
saída (`TIPv4_element.txt`) com um binário estático leve e de inicialização
instantânea.

## Requisitos

- Compilador `g++` com suporte a **C++17**.
- `libcurl` para desenvolvimento (`libcurl4-openssl-dev` no Debian).

## Compilação

```bash
make              # build de release (otimizado, v1.0.0)
```

Compilação manual:

```bash
g++ -std=c++17 -O2 -o getip main.cpp getip.cpp -lcurl
```

## Verificação, Testes e Sanitizers

```bash
make check        # warnings tratados como erro (-Werror)
make test         # suíte table-driven (parser HTML, IPv4, escrita, args)
make sanitize     # AddressSanitizer + UndefinedBehaviorSanitizer
```

## Uso

```bash
./getip                     # coleta de https://192.168.10.1 -> /app/logs/TIPv4_element.txt
./getip --help              # ajuda
./getip --version           # getip v1.0.0
./getip --url http://10.0.0.1 --output-dir ./out --log-level DEBUG
```

Opções principais:

| Opção | Padrão | Descrição |
|---|---|---|
| `--url` | `https://192.168.10.1` | Página a consultar |
| `--element-id` | `TIPv4` | Id do elemento HTML |
| `--output-dir` | dir de `LOG_FILE_PATH` ou `/app/logs` | Diretório de saída |
| `--output-file` | `TIPv4_element.txt` | Arquivo de saída (sem diretório → dentro de `--output-dir`; com diretório → caminho completo) |
| `--log-file` | `--output-dir`/`get_ip.log` (ou `LOG_FILE_PATH`) | Arquivo de log |
| `--log-level` | `INFO` | `DEBUG`, `INFO`, `WARNING`, `ERROR` |
| `--timeout` | `15` (s) | Timeout da requisição |
| `--verify-tls` | desabilitada | Habilita verificação do certificado TLS |

Variáveis de ambiente compatíveis com o legado:

- `LOG_FILE_PATH` — define o arquivo de log e o diretório de saída padrão.
- `LOG_LEVEL` — nível de log.
- `GETIP_URL` — URL alvo alternativa.

## Compatibilidade com o legado

O `get_ip.py` escrevia o IPv4 (sem máscara) em `TIPv4_element.txt`. O `getip`
mantém esse contrato byte a byte (sem quebra de linha). A verificação real foi
validada contra `curl -sk https://192.168.10.1 | grep -E '...'`, que retorna o
mesmo endereço com a máscara `203.0.113.42/255.255.255.0`.

## Instalação (uso global)

```bash
sudo make install   # instala em /usr/local/bin/getip
getip --version
```

## Documentação

- `docs/README.md` — detalhamento técnico de entradas, saídas e deploy.
- `CHANGELOG.md` — histórico de versões.
- `TODO.md` — pendências e próximos passos.
- `PLANNER.md` — planejamento e escopo.

> **Nota de arquitetura:** o binário é compilado para **aarch64** no servidor
> (Debian 12). Para outras arquiteturas, recompile no destino.
