*This project has been created as part of the 42 curriculum by yzoullik & noel-baz*

---

# Webserv

## Description

**Webserv** is a custom HTTP server developed in C++ as part of the 42 curriculum.
The goal of this project is to understand how web servers work by building one from scratch, following the HTTP/1.0 protocol.

The server is designed to:

* Handle HTTP requests (GET, POST, DELETE)
* Serve static files
* Execute CGI scripts
* Manage multiple clients simultaneously
* Support configuration via a custom configuration file

This project focuses on low-level network programming, event-driven architecture (e.g., `select`, `poll`, or `epoll`), and proper resource management.

---

## Instructions

### Requirements

* C++ compiler (g++ / clang++)
* Make
* Unix-based system (Linux or macOS recommended)

### Compilation

Clone the repository and compile the project using:

```bash
git clone <your-repo-url>
cd webserv
make
```

This will generate the executable:

```bash
./webserv
```

### Usage

Run the server with a configuration file:

```bash
./webserv <config_file>
```

Example:

```bash
./webserv default.conf
```

If no configuration file is provided, a default configuration may be used (depending on implementation).

### Testing

You can test the server using:

* A web browser (http://localhost:<port>)
* `curl`:

```bash
curl http://localhost:8080
```
---

## Resources

### Documentation & References

* RFC 1945 (HTTP/1.0 specification)
  https://datatracker.ietf.org/doc/html/rfc1945

### Tutorials & Articles

* Socket programming in C/C++

### Use of AI

* we don't do that here