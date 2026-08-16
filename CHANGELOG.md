# Changelog — getip

Todas as mudanças notáveis deste projeto serão documentadas neste arquivo.

## [Unreleased]

### Corrigido

- Abort (exceção `filesystem_error`) quando o diretório de log padrão
  (`/app/logs`) não é gravável no host: `--output-dir`/`--output-file` agora
  definem também o log padrão (`--output-dir`/`get_ip.log`), e falhas ao
  criar diretórios são tratadas como erro controlado, sem abortar.
- `--output-file` com diretório (`./ip`, `/tmp/x.txt`) passa a ser usado como
  caminho completo em vez de ser unido ao `--output-dir`.

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
