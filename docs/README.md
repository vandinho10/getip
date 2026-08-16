# Documentação Técnica — getip

## Visão geral

`getip` é uma CLI em C++17 que substitui o `get_ip.py` (Selenium/Chromium) do
serviço `chromium`. Ela consulta o painel administrativo do roteador
(`https://192.168.10.1`), localiza o elemento HTML com `id="TIPv4"`, extrai o
primeiro IPv4 do texto e grava em um arquivo de saída.

## Entradas e saídas

| Direção | Item | Formato | Exemplo |
|---|---|---|---|
| Entrada | URL alvo | `--url` ou `GETIP_URL` | `https://192.168.10.1` |
| Entrada | Id do elemento | `--element-id` | `TIPv4` |
| Entrada | Ambiente | `LOG_FILE_PATH`, `LOG_LEVEL` | `/app/logs/get_ip.log` |
| Saída | Arquivo de resultado | `TIPv4_element.txt` | `203.0.113.42` |
| Saída | Log | `get_ip.log` | `2026-08-16 12:00:00 - INFO - IP Encontrado: 203.0.113.42` |

## Fluxo de execução

1. Leitura de argumentos de linha de comando e variáveis de ambiente.
2. Requisição `GET` via libcurl (segue redirecionamentos, timeout de 15 s,
   certificado TLS não verificado por padrão).
3. Localização do elemento pelo atributo `id` (tolerante a aspas simples,
   duplas e espaços internos, ex.: `id= "TIPv4"`).
4. Remoção de tags HTML, decodificação de entidades e normalização de espaços.
5. Extração do primeiro IPv4 por expressão regular.
6. Gravação do IPv4 em `TIPv4_element.txt` (sem quebra de linha, compatível
   com o legado) e registro em log.

## Dependências

- **Compilação:** `g++` (C++17), libcurl headers (`libcurl4-openssl-dev`).
- **Runtime:** libcurl4 e OpenSSL fornecidos pelo Debian (linkagem dinâmica).

## Compatibilidade com o legado

O `get_ip.py` escrevia apenas o IPv4 em `TIPv4_element.txt` dentro do diretório
definido por `dirname(LOG_FILE_PATH)` (padrão `/app/logs`). O `getip` preserva
este contrato, incluindo o uso das variáveis `LOG_FILE_PATH` e `LOG_LEVEL`.

### Resolução de caminhos

- `--output-file` **sem** diretório é gravado dentro de `--output-dir`
  (padrão: `TIPv4_element.txt` no diretório de log).
- `--output-file` **com** diretório (`./ip`, `/tmp/x.txt`, `sub/ip.txt`) é
  usado como caminho completo.
- `--log-file` sem valor usa `--output-dir`/`get_ip.log` (ou `LOG_FILE_PATH`).
- Falhas ao criar diretórios são tratadas como erro sem abortar o processo.

Validação real contra o roteador (2026-08-16):

```
$ curl -sk https://192.168.10.1 | grep -E '([0-9]{1,3}\.){3}[0-9]{1,3}/...'
<dd id= "TIPv4" class="success"><span>IPv4</span> 203.0.113.42/255.255.255.0 </dd>

$ ./getip
2026-08-16 12:00:00 - INFO - IP Encontrado: 203.0.113.42
$ cat /app/logs/TIPv4_element.txt
203.0.113.42
```

## Teste e deploy

```bash
make check        # análise estática com -Werror
make test         # suíte table-driven
make sanitize     # ASan + UBSan
make install      # instala em /usr/local/bin/getip (requer sudo)
```

## Próximos passos

- Migrar o serviço `chromium` para uma imagem enxuta que executa apenas o
  binário (multi-stage, sem navegador).
- Publicar o binário em release do GitHub para download versionado.
