# Nexo Protocol Specification

This document describes the Nexo proxy protocol. It is intended for developers who want to implement their own client that is compatible with NexoServer, without using the reference C++ implementation.

## Overview

Nexo is a minimal TCP tunnelling protocol that runs over TLS. The client opens a TLS connection to the server, sends a compact binary header identifying itself and naming the target host, and then relays raw TCP data in both directions.

```
Client                          NexoServer                    Target
  │                                  │                           │
  │──── TLS handshake ──────────────▶│                           │
  │◀─── TLS handshake ───────────────│                           │
  │                                  │                           │
  │──── NexoProtocolHeader ─────────▶│                           │
  │──── hostname (variable) ────────▶│                           │
  │                                  │──── TCP connect ─────────▶│
  │                                  │◀─── TCP connected ────────│
  │                                  │                           │
  │◀════════════ bidirectional relay ═══════════════════════════▶│
```

## Transport

The client connects to the server over **TLS** (port 443 by default). The server expects a valid TLS handshake; plain TCP connections are rejected.

The client may set a custom **SNI** hostname during the handshake to make traffic look like ordinary HTTPS — the server does not validate the SNI value.

## Authentication

Authentication is UUID-based. Each authorised user is assigned a UUIDv4. The raw 16-byte representation of the UUID is sent as the first field of the protocol header (see below). The server looks it up in its user list and drops the connection if it is not found or is marked as disabled.

## Protocol header

Immediately after the TLS handshake, the client sends a fixed-size binary header followed by a variable-length hostname.

All multi-byte integer fields are **big-endian**.

### Fixed part (21 bytes total)

| Offset | Size | Field | Description |
|--------|------|-------|-------------|
| 0 | 16 bytes | UUID | Raw bytes of the user's UUIDv4 (not the ASCII string). |
| 16 | 1 byte | Version | Protocol version. Must be `0x01`. |
| 17 | 1 byte | Command | Command. Must be `0x01` (CONNECT). |
| 18 | 2 bytes | Port | Target port in big-endian (network byte order). |
| 20 | 1 byte | AddressSize | Length in bytes of the hostname that follows. |

### Variable part

Immediately after the fixed header, the client sends the target hostname as a plain UTF-8 string of exactly `AddressSize` bytes. No null terminator.

The hostname may be:
- a domain name (e.g. `example.com`)
- an IPv4 address in dotted-decimal notation (e.g. `93.184.216.34`)
- an IPv6 address in standard notation (e.g. `2606:2800:220:1:248:1893:25c8:1946`)

### Example (hex)

Connecting to `example.com:443` as user `550e8400-e29b-41d4-a716-446655440000`:

```
55 0e 84 00  e2 9b 41 d4  a7 16 44 66  55 44 00 00  ← UUID (16 bytes)
01                                                  ← Version = 1
01                                                  ← Command = CONNECT
01 bb                                               ← Port = 443 (big-endian)
0b                                                  ← AddressSize = 11
65 78 61 6d 70 6c 65 2e 63 6f 6d                    ← "example.com"
```

## Server response

The server does **not** send any response back to the client after a successful header. Once the header is validated and the upstream TCP connection is established, the server immediately begins relaying raw bytes in both directions.

If the UUID is invalid, the connection is terminated without any message — or, if the fallback page feature is enabled on the server, an HTTP response is sent instead.

## Relay

After the header exchange the connection becomes a transparent bidirectional byte stream. The client and the target host communicate directly through this relay; no further Nexo framing is applied.

## UUID encoding

A UUIDv4 string like `550e8400-e29b-41d4-a716-446655440000` maps to 16 bytes by removing the dashes and interpreting the remaining 32 hex characters as a byte sequence:

```
55 0e 84 00 e2 9b 41 d4 a7 16 44 66 55 44 00 00
```