# HTTP Server (C)

A minimal HTTP server written in C.

## Features

- `GET` and `HEAD` support
- static file serving
- MIME types
- route system
- keep-alive support
- epoll backend
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
