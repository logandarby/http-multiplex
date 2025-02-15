#include "arena.h"

#include <stdbool.h>
#include <string.h>

#include "core.h"

const size_t DZ_ARENA_DEFAULT_MAX_SIZE = 1000000;  // 1MB

DZArena dz_arena_init(const size_t max_size) {
  size_t size_to_allocate =
      (max_size == 0) ? DZ_ARENA_DEFAULT_MAX_SIZE : max_size;
  char *data = (char *)malloc(size_to_allocate * sizeof(char));
  DZArena arena = {
      .max_size = size_to_allocate,
      .first_empty_byte = 0,
      .data = data,
      .error = DzArenaError_NONE,
  };
  if (!data) {
    arena.error = DzArenaError_MALLOC;
    arena.max_size = 0;
  }
  return arena;
}

void dz_arena_free(DZArena *arena) {
  if (!arena || !arena->data || arena->error) {
    return;
  }
  free(arena->data);
}

void dz_arena_clear(DZArena *arena) {
  if (!arena || arena->error) {
    return;
  }
  arena->first_empty_byte = 0;
}

void *dz_arena_alloc(DZArena *arena, const size_t n_bytes) {
  if (!arena || arena->error) {
    return NULL;
  }
  void *address = arena->data + arena->first_empty_byte;
  arena->first_empty_byte += n_bytes;
  if (arena->first_empty_byte > arena->max_size) {
    DZ_ASSERT(arena->first_empty_byte > arena->max_size, "Allocation to big for arena");
    arena->error = DzArenaError_ALLOC;
    return NULL;
  }
  memset(address, 0, n_bytes);
  return address;
}
