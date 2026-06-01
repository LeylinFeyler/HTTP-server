# HTTP Server (C)

A minimal HTTP server written in C.

## Features

- `GET` and `HEAD` support
- static file serving
- MIME type detection
- route system
- keep-alive support
- non-blocking sockets
- epoll backend
- idle connection timeout
- request size limits
- path traversal protection
- custom error pages
- request logging

## Build

```bash
make
````

## Run

```bash
./server
```

Server runs on:

```text
http://localhost:8888
```

## Example routes

```text
/
/hello
/health
```
