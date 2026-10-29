# Changelog — getip

Todas as mudanças notáveis deste projeto serão documentadas neste arquivo.

## [Unreleased]

## [1.3.0] - 2026-10-29

### Adicionado

- **`getip update`**: autoatualização pela última release do GitHub
  (`update check` retorna 0/2/1). O SHA256 é conferido contra o `SHA256SUMS`
  da release antes da troca e é obrigatório; o binário anterior fica em
  `.old` e volta sozinho se o novo não responder a `--version`.
- `scripts/install.sh`: instala em `~/.local/bin` conferindo o `SHA256SUMS` e
  cria o timer systemd do usuário `getip-update.timer` (diário).
- `selfupdate.cpp/.hpp`: módulo compartilhado com ddns_manager e invpush
  (SHA-256 embutido, sem depender de `sha256sum`/`certutil`).
- CI: a release publica `SHA256SUMS`.

### Alterado

- `make install` instala em `~/.local/bin` por padrão (`BINDIR` muda o destino).
- Builds Windows com `-Wl,--no-insert-timestamp`, para hashes reprodutíveis.
- Versão do binário: `1.3.0`.

## [1.2.0] - 2026-08-22

### Adicionado

- Suporte a **Windows**: binários estáticos `getip-windows-amd64.exe` e
  `getip-windows-x86.exe` via cross-compile mingw-w64, zero dependências em
  runtime. O transporte HTTP passa a ser duplo: **WinHTTP nativo** no Windows
  (com os mesmos comportamentos do curl: redirecionamentos seguidos,
  `--verify-tls` equivalente ao `curl -k`, timeouts por requisição) e
  **libcurl** nos demais sistemas — sem mudanças de contrato na saída.
- CI: matriz de release estendida com os dois alvos Windows (5 artefatos).

### Alterado

- `Makefile`: variável `TARGET_OS` com linkagem estática e `-lwinhttp` no
  Windows; `clean` remove também o `.exe` gerado pelo ld PE.
- Versão do binário: `1.2.0`.

## [1.1.0] - 2026-08-22

### Adicionado

- **Configuração persistente** (estilo `git config`): subcomando `getip config`
  com verbos `list` (padrão), `get`, `set`, `add`, `unset` e `path`, escopos
  `--global` (padrão) e `--local`.
- Dois níveis de arquivo: global em
  `$XDG_CONFIG_HOME/getip/config` (`%APPDATA%\getip\config.txt` no Windows)
  e local `.getip.conf` descoberto subindo a hierarquia de diretórios a partir
  do cwd (commitável por design; oculto também no Windows via atributo hidden).
- Precedência: **CLI > local > global > defaults**. Opções não fornecidas na
  CLI são preenchidas pela config.
- Chaves suportadas: `url`, `element-id`, `output-dir`, `output-file`,
  `log-file`, `log-level`, `timeout`, `verify-tls`.
- Gravação atômica (tmp + rename), formato `chave=valor` (uma linha por valor,
  comentários com `#`), avisos para linhas/chaves inválidas.
- Variável `GETIP_CONFIG_HOME` para redirecionar o nível global (sandbox/testes).
- 12 novos testes (roundtrip, precedência, walk-up, ciclo set→get→unset,
  `--local`, timeout inválido etc.).

### Alterado

- Versão do binário: `1.1.0`.

## [1.0.2] - 2026-08-17

### Adicionado

- CI: pipeline de release **multi-plataforma** via GitHub Actions —
  cross-compile de `x86_64`, `aarch64` e `arm32` (com jobs Docker+QEMU para
  builds multiplataforma), viabilizando artefatos Linux para os servidores
  ARM64 e desktops x86 sem host Docker dedicado.

## [1.0.1] - 2026-08-16

### Corrigido

- Abort (exceção `filesystem_error`) quando o diretório de log padrão não é
  gravável no host: `--output-dir`/`--output-file` agora definem também o log
  padrão (`--output-dir`/`get_ip.log`), e falhas ao criar diretórios são
  tratadas como erro controlado, sem abortar.
- `--output-file` com diretório (`./ip`, `/tmp/x.txt`) passa a ser usado como
  caminho completo em vez de ser unido ao `--output-dir`.

### Alterado

- Diretório padrão de saída e log passa a ser `./data` (antes `/app/logs`).
- Versão do binário: `1.0.1`.

## [1.0.0] - 2026-08-16

### Added

- Utilitário C++17 `getip` que substitui o legado `get_ip.py` (Python +
  Selenium + Chromium) do serviço `chromium`.
- Coleta do IPv4 do elemento HTML `#TIPv4` do roteador via HTTPS
  (verificação de certificado desabilitada por padrão, compatível com `curl -k`).
- Gravação do IPv4 em `TIPv4_element.txt` (contrato byte a byte com o legado).
- Logging em console e arquivo (`get_ip.log`) com níveis
  `DEBUG`/`INFO`/`WARNING`/`ERROR`.
- Compatibilidade com as variáveis de ambiente `LOG_FILE_PATH`, `LOG_LEVEL` e
  `GETIP_URL`.
- CLI com `--url`, `--element-id`, `--output-dir`, `--output-file`,
  `--log-file`, `--log-level`, `--timeout`, `--verify-tls`, `--help` e
  `--version`.
- Makefile com alvos `all`, `check`, `test`, `sanitize`, `version`, `install`
  e `clean`.
- Suíte de testes table-driven (parser HTML, extração de IPv4, escrita de
  arquivo e argumentos de linha de comando).
- Documentação completa: `README.md`, `CHANGELOG.md`, `TODO.md`, `PLANNER.md`
  e `docs/`.
- Binário aarch64 compilado e validado contra a página real do roteador
  (resultado idêntico ao `curl | grep`).
