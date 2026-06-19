# Nexo

A lightweight **proxy protocol** written in C++ 20 using the **boost** library. All traffic is protected by **OpenSSL TLS** encryption. Uses the UUID v4 authorization method. It includes both the **server** part and the **client**.

```
Browser / App
    │  SOCKS5
    ▼
NexoClient (127.0.0.1:6578)
    │  TLS + Nexo protocol header
    ▼
NexoServer (:443)
    │  plain TCP
    ▼
Target host
```

## Features

- All traffic ecrypted with TLS (OpenSSL)
- UUID-based user authentication
- Included client with local SOCKS-5 server
- Optional SNI spoofing to blend in with regular HTTPS traffic
- Configurable fallback page for non-proxy connections
- Asynchronous I/O via Boost.Asio coroutines
- TOML configuration files for both server and client

## Quick start

Build and install - see [INSTALL.md](INSTALL.md).

**Run the server:**
```sh
./nexod
```

**Run the client:**
```sh
./nexo_client
```

**Test the connection:**
```sh
curl -v --socks5 127.0.0.1:6578 https://example.com
```

On first run, if a config file is missing, the binary creates a default one and exits — edit it and run again.

## Project structure

```
Nexo/
├── CMakeLists.txt
├── CMakePresets.json
├── NexoClient/
│   ├── config/                      # default client config template
│   └── src/
│       ├── client.cpp               # entry point
│       ├── config/                  # config loader
│       ├── headers/                 # protocol structs (nexo.hpp, socks5.hpp)
│       ├── logger/                  # CLogger
│       ├── servers/socks5server/    # local SOCKS5 listener
│       ├── sessions/socks5session/  # per-connection logic + TLS relay
│       └── utils/
└── NexoServer/
    ├── config/                      # default server config template
    └── src/
        ├── server.cpp               # entry point
        ├── config/                  # config loader
        ├── logger/                  # CLogger
        ├── server/                  # CServer — accepts TLS connections
        ├── session/                 # CSession — auth, resolve, relay
        └── utils/
```

## Roadmap

- [ ] Stabilise the protocol (finalize header format, versioning)
- [ ] Add UDP support
- [ ] TLS traffic flow imitation
- [ ] Multithreading for the server
- [ ] Additional transports: WebSocket, XHTTP, and others

## License

This project is licensed under the GNU General Public License v3.0 — see [LICENSE](LICENSE) for details.

## Other

> Telegram channel (RU): [@project_freedom_channel](https://t.me/project_freedom_channel)