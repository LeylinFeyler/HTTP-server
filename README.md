# HTTP Server (C)

A minimal HTTP server written in C using an `epoll`-based event loop.

This project started as a simple learning project and gradually evolved into a small HTTP server with routing, static file serving, request parsing, and basic API functionality.

## Features

### HTTP

* `GET`, `HEAD`, and `POST` support
* keep-alive connections
* query string parsing
* request body parsing
* URL decoding
* HTTP request validation
* request size limits
* support for common HTTP headers

### Server

* `epoll` backend
* non-blocking sockets
* idle connection timeout
* route system
* request logging

### Static files

* static file serving
* MIME type detection
* custom error pages (`400`, `403`, `404`, `500`)
* path traversal protection

### API / Responses

* JSON response helpers
* basic API routes

## Project Structure

```text
.
├── src/
├── static/
│   ├── css/
│   ├── js/
│   ├── img/
│   └── *.html
├── Makefile
└── README.md
```

## Build

```bash
make
```

## Run

```bash
./server
```

The server runs on:

```text
http://localhost:8888
```

## Example Routes

```text
/
/hello
/health
/greet?name=Geronimo
/json
```

Example request:

```bash
curl http://localhost:8888/hello
```

POST request example:

```bash
curl -X POST \
     -d "hello world" \
     http://localhost:8888/echo
```

## Goals

This project is primarily focused on learning low-level networking, HTTP internals, and systems programming in C.
