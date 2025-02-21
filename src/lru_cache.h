#pragma once

#include <dz_hashmap.h>
#include <stdlib.h>

// A node of a doubly linked list used for LRUCache lookup
typedef struct LRUNode {
  struct LRUNode *prev;
  char *key;
  char *value;
  struct LRUNode *next;
} LRUNode;

// Stored a doubly linked list for the cache, and uses a hashmap to
// make the doubly linked list lookup O(1)
typedef struct LRUCache {
  LRUNode *head;
  LRUNode *tail;
  const size_t capacity;
  size_t node_count;
  DzHashmap node_map;  // Maps keys to the node entry of the internal
                       // doubly linked list
} LRUCache;

// Initializes an LRU Cache with a capacity
LRUCache lru_init(size_t capacity);

void lru_free(LRUCache *self);

// Pushes a new item onto the list. If the list is at capacity,
// removes the lowest priority item
LRUNode *lru_push(LRUCache *self, const char *key, const char *value);

// Gets the item from the priority list and updates the piority of the
// item to #1 Returns a const pointer to the internal reference of the
// string. The pointer is invalid once the LRU is freed If nothing is
// found, returns NULL
const char *lru_get(LRUCache *self, const char *key);
