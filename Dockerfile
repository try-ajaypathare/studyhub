# syntax=docker/dockerfile:1.6
#
# studyhub · multi-stage Docker build
#
# Stage 1 (build): Alpine + g++. Compiles the C++14 backend.
# Stage 2 (run):   Alpine + libstdc++. Ships the binary, frontend
#                  and sample data. Final image is ~25 MB.

# ---------- Build stage ----------
FROM alpine:3.19 AS build

RUN apk add --no-cache g++ make musl-dev

WORKDIR /app

# Copy only what we need to compile.
COPY backend/include  ./backend/include
COPY backend/src      ./backend/src
COPY backend/main.cpp ./backend/main.cpp
COPY backend/Makefile ./backend/Makefile

# Build with the cross-platform Makefile (uname -s != Windows -> Linux libs).
WORKDIR /app/backend
RUN make OPT="-O2 -s"

# ---------- Runtime stage ----------
FROM alpine:3.19

RUN apk add --no-cache libstdc++ libgcc

WORKDIR /app

# Pull in the binary, the frontend, and the seed data.
COPY --from=build /app/backend/build/ssaas_server  ./ssaas_server
COPY backend/data                                  ./data
COPY frontend                                      ./frontend

# Render injects PORT; bind to 0.0.0.0 so it's reachable from outside the container.
ENV HOST=0.0.0.0 \
    PORT=8090 \
    DATA_PATH=/app/data/sample_data.json \
    STATIC_DIR=/app/frontend

EXPOSE 8090

# Drop to a non-root user.
RUN adduser -D -u 1000 ssaas && chown -R ssaas /app
USER ssaas

CMD ["./ssaas_server"]
