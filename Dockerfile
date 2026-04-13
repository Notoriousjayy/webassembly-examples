# =============================================================================
# WebAssembly-only Dockerfile
# Target: Browser delivery through Emscripten + SDL3 + WebGL2
# =============================================================================

# -----------------------------------------------------------------------------
# Stage: wasm-builder - build the ES-module WebAssembly bundle
# -----------------------------------------------------------------------------
FROM emscripten/emsdk:latest AS wasm-builder

RUN apt-get update && apt-get install -y \
    ninja-build \
    python3 \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY . .

RUN rm -rf build-* CMakeCache.txt

RUN emcmake cmake --preset wasm-release
RUN cmake --build --preset wasm-release --parallel

RUN ls -lh build-wasm/ && \
    test -f build-wasm/index.html && \
    test -f build-wasm/bootstrap.js && \
    test -f build-wasm/testProject.js && \
    test -f build-wasm/testProject.wasm

# -----------------------------------------------------------------------------
# Stage: wasm-server - serve the browser bundle
# -----------------------------------------------------------------------------
FROM nginx:alpine AS wasm-server

COPY --from=wasm-builder /app/build-wasm/ /usr/share/nginx/html/

RUN cat > /etc/nginx/conf.d/default.conf << 'EOF'
server {
    listen 80;
    server_name localhost;
    root /usr/share/nginx/html;
    index index.html index.htm;

    add_header 'Access-Control-Allow-Origin' '*' always;
    add_header 'Cross-Origin-Opener-Policy' 'same-origin' always;
    add_header 'Cross-Origin-Embedder-Policy' 'require-corp' always;

    types {
        text/html              html htm shtml;
        text/css               css;
        application/javascript js mjs;
        application/wasm       wasm;
    }

    location / {
        try_files $uri $uri/ /index.html;
    }

    location ~* \.wasm$ {
        add_header 'Content-Type' 'application/wasm';
        add_header 'Access-Control-Allow-Origin' '*';
    }
}
EOF

RUN nginx -t

EXPOSE 80
STOPSIGNAL SIGTERM
CMD ["nginx", "-g", "daemon off;"]

# -----------------------------------------------------------------------------
# Stage: dev-environment - interactive Emscripten development container
# -----------------------------------------------------------------------------
FROM emscripten/emsdk:latest AS dev-environment

RUN apt-get update && apt-get install -y \
    ninja-build \
    python3 \
    git \
    vim \
    nano \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /workspace
CMD ["/bin/bash"]
