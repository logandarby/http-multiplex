# DZHttp

DZHttp is a small HTTP file server written in C99. I built it as a hobby project to learn POSIX networking, I/O multiplexing, caching, memory ownership, and testing in C.

## Features

- TCP socket setup and HTTP file serving from `res/`
- One event loop using `poll`
- An `FdPool` for listening, client, and file descriptors
- An LRU response cache
- Arena allocation for short-lived request data
- Debug logging and AddressSanitizer builds

## Design

### I/O multiplexing

The server uses one event loop instead of one thread per request. `FdPool` tracks the listening socket, clients, and files opened for requests. `poll` waits for a descriptor to become ready, then the loop accepts a connection, reads a request, reads the requested file, or sends a response.

### LRU cache

The cache maps filenames to generated HTTP responses. It uses a hashmap for lookup and a doubly linked list for usage order. Cache hits promote an entry; once the cache reaches its capacity, the least recently used entry is removed. Lookups and list updates are constant time.

### Testable I/O

`FileModule` contains function pointers for `open`, `read`, `write`, `accept`, `recv`, `send`, and `poll`. Passing these operations into the server leaves a boundary for replacing or observing system calls in tests.

The arena, cache, and descriptor pool have separate cleanup paths. Debug builds add assertions and logging around the low-level operations.

## Building

The project uses CMake and Ninja. CMake FetchContent downloads GoogleTest when configuring the test target.

### Debug build

The default preset enables debug logging and AddressSanitizer through the `CFLAGS` environment setting:

```bash
cmake --preset default
cmake --build build
```

### Release build

```bash
cmake --preset release
cmake --build release
```

The resulting executable is `build/DZHttp` for the debug preset or `release/DZHttp` for the release preset.

## Running the Server

The server serves files from the `res/` directory in the current build configuration. For a debug build:

```bash
./build/DZHttp --help
./build/DZHttp
```

The command-line parser supports selecting a port. Once the server is running, a basic request can be made with:

```bash
curl http://localhost:8080/test.html
```

This is a small HTTP file server, not a production web server. It does not implement HTTPS or TLS.

## Testing

The tests use GoogleTest and CTest:

```bash
ctest --test-dir build --output-on-failure
```

The tests cover:

- LRU initialization, lookup, promotion, and capacity eviction
- A server-spam test with 100 client threads and 10 requests per client
- HTTP status and content-length checks

The default debug preset compiles with AddressSanitizer, so the test runs can also catch many memory errors.

## Fuzzing

`scripts/fuzz.sh` uses Radamsa to mutate a few valid-looking requests, then sends them to the server through `nc` in a loop. It is used to exercise malformed input and repeated connections.

To use it, start the server and provide its listening port:

```bash
./build/DZHttp
./scripts/fuzz.sh 8080
```


## Project Structure

```
src/
├── main.c          # Command-line entry point
├── start.c         # Server event loop and HTTP request handling
├── server.c        # Listening socket setup
├── pool.c          # Poll-based file descriptor pool
├── lru_cache.c     # Hashmap-backed LRU response cache
├── file_module.c   # Injectable POSIX I/O operations
├── arguments.c      # Command-line argument parsing
└── interrupt.c     # Signal handling for graceful shutdown
extern/dz_lib/      # Supporting arena, array, hashmap, and debug utilities
res/                # Static files served by the server
tests/              # GoogleTest cache and concurrent server tests
scripts/fuzz.sh     # Radamsa and netcat request mutation loop
```
