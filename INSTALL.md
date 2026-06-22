# Installation & Configuration

## Requirements

| Dependency | Version |
|---|---|
| C++ compiler | C++20 (MSVC, GCC 12+, Clang 14+) |
| CMake | 3.24+ |
| Ninja | any recent |
| OpenSSL | 3.x |
| Boost | 1.80+ |
| vcpkg | for managing OpenSSL and Boost on Windows |

> **toml++** is fetched automatically by CMake via `FetchContent` — no manual install needed.

## Installing (Linux Server)

### 1. Clone repository

Clone repository to the temp directory (ex. ~/Nexo)

```sh
git clone https://github.com/ImHartash/Nexo.git
cd Nexo
```

### 2. Run bash script from root

```sh
sudo bash install.sh
```

This script will automatically create the required directories and services.

### 3. Configure server configuration

```sh
cd /opt/nexo/
```

Configure `config/server_configuration.toml` using any text editor (`nano`, `vim` and etc.).

### 4. Run Nexo server

If it successfully installed, nexo will be automatically enabled in systemctl. Check Nexo's status via `nexo status`.

```sh
nexo start
```

---

## Server configuration

### 1. Obtain a TLS certificate

You need a certificate and a private key. A self-signed pair for testing:

```sh
openssl req -x509 -newkey rsa:4096 -keyout server.key -out server.crt -days 365 -nodes
```

Place the files where `server_configuration.toml` points (default: `certs/server.crt` and `certs/server.key`).

### 2. Edit `config/server_configuration.toml`

```toml
[server]
port      = 443
cert_file = "certs/server.crt"
key_file  = "certs/server.key"

[fallback]
# When enabled, connections with an unknown UUID receive this HTML page
# instead of being dropped. Useful for disguising the server as a regular website.
enabled   = false
html_file = "fallback/index.html"

[limits]
max_connections = 100
timeout_seconds = 60

[log]
log_level = "info"
file      = "logs/nexo.log"

# Add one [[users]] block per authorised user.
[[users]]
name    = "alice"
uuid    = "550e8400-e29b-41d4-a716-446655440000"
enabled = true
```

Generate a UUID for each user with any standard UUIDv4 generator (e.g. https://www.uuidgenerator.net).

### 3. Run

```sh
./nexod
```

---

## Client configuration

### 1. Edit `config/client_configuration.toml`

```toml
[local]
host = "127.0.0.1"
port = 6578          # the local SOCKS5 port your browser will connect to

[server]
server_host = "your-server-ip-or-domain.com"
server_port = 443
uuid        = "550e8400-e29b-41d4-a716-446655440000"  # must match a [[users]] entry on the server

[tls]
verify_cert = false          # set to true in production with a CA-signed certificate
tls_sni     = "example.com"  # the SNI hostname sent during the TLS handshake

[connection]
timeout_seconds = 10
retry_attempts  = 3
retry_delay_ms  = 1000

[log]
level = "info"
file  = "logs/nexo_client.log"
```

### 2. Run

```sh
./nexo_client
```

On first run, if the config file does not exist, the client creates a default one and exits. Edit it and run again.

### 3. Point your browser or tool at the local proxy

**Browser:** set the SOCKS5 proxy to `127.0.0.1:6578` in your network settings.

**curl:**

```sh
curl -v --socks5 127.0.0.1:6578 https://example.com
```