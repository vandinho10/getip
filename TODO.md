# TODO — getip

## Coletor C++ (getip)

- [x] Verificar equivalência de saída entre `get_ip.py` e `curl | grep`.
- [x] Implementar requisição HTTPS via libcurl (certificado ignorado por padrão).
- [x] Implementar parser do elemento HTML por atributo `id`.
- [x] Implementar extração do primeiro IPv4.
- [x] Implementar escrita do arquivo de saída `TIPv4_element.txt`.
- [x] Implementar logging em console e arquivo com níveis.
- [x] Implementar CLI (`--url`, `--output-dir`, `--log-level`, `--timeout`, ...).
- [x] Escrever suíte de testes table-driven e rodar sanitizers.
- [x] Compilar binário aarch64 e validar contra o roteador real.
- [ ] Publicar binário em release do GitHub (semelhante ao `codectx`).
- [ ] Migrar o serviço `chromium` do `docker-compose.yaml` para imagem com o binário.

## Infraestrutura

- [x] Criar a pasta `compiled/` na raiz do monorepo.
- [x] Mover o submódulo `codectx` para `compiled/codectx`.
