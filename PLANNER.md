# PLANNER — getip

## Objetivo

Substituir o script `get_ip.py` (Python + Selenium + Chromium) do serviço
`chromium` por um utilitário C++17 leve (`getip`) que coleta o IPv4 do roteador
via HTTPS (libcurl) e grava o endereço em arquivo texto.

## Justificativa

O navegador Chromium é desnecessariamente pesado para uma única extração de
texto via HTTP. O `curl | grep` já entrega praticamente o mesmo resultado; o
`getip` consolida isso em um binário dedicado, com a mesma saída do legado.

## Escopo

- [x] Verificar as saídas: `get_ip.py` (IPv4) vs `curl | grep` (IPv4/máscara).
- [x] Criar a pasta `compiled/` na raiz do monorepo e mover o submódulo `codectx`.
- [x] Implementar `getip` em C++17 (`getip.hpp`, `getip.cpp`, `main.cpp`, `Makefile`).
- [x] Testes table-driven + sanitizers.
- [x] Compilar o binário aarch64 e validar contra o roteador real.
- [ ] Migração futura: substituir o serviço `chromium` por imagem com o binário.

## Fora de escopo

- Alteração do `docker-compose.yaml` (decisão tomada em 2026-08-16: somente
  entregar o tool; a migração fica documentada no `TODO.md`).
