#!/usr/bin/env bash
# install.sh - nexo protocol installer
set -e

INSTALL_DIR="/opt/nexo"
SERVICE_NAME="nexod"
SERVICE_USER="nexo"
BUILD_DIR="bin"

# Colors for output — optional, just nicer to read
GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m'

log() {
    echo -e "${GREEN}[install]${NC} $1"
}
error() {
    echo -e "${RED}[error]${NC} $1"
    exit 1
}

if [ "$EUID" -ne 0 ]; then
    error "Root privileges required. Run: sudo ./install.sh"
fi

# Installing dependecies
log "Installing dependencies"
apt update -qq
apt install -y build-essential cmake ninja-build git libboost-dev libssl-dev libcap2-bin

# Building the project
log "Building the cmake project..."
cmake -B "$BUILD_DIR" -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build "$BUILD_DIR"

BINARY_PATH=$(find "$BUILD_DIR" -type f -name "nexod" -executable | head -n 1)

if [ -z "$BINARY_PATH" ]; then
    error "Could not find a built server binary in $BUILD_DIR. Check the build manually."
fi
log "Found binary: $BINARY_PATH"

if ! id "$SERVICE_USER" &>/dev/null; then
    log "Creating user $SERVICE_USER..."
    useradd --system --no-create-home --shell /usr/sbin/nologin "$SERVICE_USER"
else
    log "User $SERVICE_USER already exists, skipping."
fi

log "Creating directory structure in $INSTALL_DIR..."
mkdir -p "$INSTALL_DIR"/{certs,fallback,logs,config}

log "Copying binary and related files..."
cp "$BINARY_PATH" "$INSTALL_DIR/nexod"

[ -f "NexoServer/fallback/index.html" ] && cp -n "NexoServer/fallback/index.html" "$INSTALL_DIR/fallback/"
[ -d "certs" ] && cp -n certs/* "$INSTALL_DIR/certs/" 2>/dev/null || true

log "Granting permission to bind privileged ports..."
setcap 'cap_net_bind_service=+ep' "$INSTALL_DIR/nexod"

chown -R "$SERVICE_USER:$SERVICE_USER" "$INSTALL_DIR"

log "Creating systemd service..."
cat > "/etc/systemd/system/${SERVICE_NAME}.service" <<EOF
[Unit]
Description=Nexo Proxy Server
After=network.target

[Service]
Type=simple
User=${SERVICE_USER}
WorkingDirectory=${INSTALL_DIR}
ExecStart=${INSTALL_DIR}/nexod
Restart=on-failure
RestartSec=3
NoNewPrivileges=true
AmbientCapabilities=CAP_NET_BIND_SERVICE

[Install]
WantedBy=multi-user.target
EOF

log "Installing the nexo command into /usr/local/bin..."
cp nexo /usr/local/bin/nexo
chmod +x /usr/local/bin/nexo

systemctl daemon-reload
systemctl enable "$SERVICE_NAME" >/dev/null

log "Installation complete!"
echo ""
echo "  1. Put your server_configuration.toml in ${INSTALL_DIR}/config/"
echo "  2. Put your certificates in ${INSTALL_DIR}/certs/"
echo "  3. Run:    nexo start"
echo "  4. Check:  nexo status"