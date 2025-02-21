#include "lru_cache.h"

#include "dz_debug.h"
#include "dz_hashmap.h"

LRUCache lru_init(size_t capacity) {
  DzHmError hm_error = DzHmError_None;
  LRUNode *head = (LRUNode *)calloc(1, sizeof(LRUNode));
  LRUNode *tail = (LRUNode *)calloc(1, sizeof(LRUNode));
  head->next = tail;
  tail->prev = head;
  LRUCache return_value = {.head = head,
                           .tail = tail,
                           .capacity = capacity,
                           .node_count = 0,
                           .node_map = hm_init(&hm_error)};
  DZ_ASSERT(!hm_error);
  return return_value;
}

static void lru_free_item(LRUNode *node) {
  free(node->value);
  free(node->key);
  free(node);
}

void lru_free(LRUCache *self) {
  hm_free(self->node_map);
  LRUNode *current = self->head->next;
  LRUNode *next = current->next;
  while (current != self->tail) {
    // WARNING: Const casted away. This is okay since it's being freed
    lru_free_item(current);
    current = next;
    next = current->next;
  }
  free(self->tail);
  free(self->head);
}

static void lru_remove_node_from_dll(LRUNode *node) {
  LRUNode *prev = node->prev;
  LRUNode *next = node->next;
  next->prev = prev;
  prev->next = next;
}

static void lru_remove_node_from_hm(LRUCache *self, LRUNode *node) {
  hm_delete_str(self->node_map, node->key);
}

static LRUNode *lru_create_node(const char *key, const char *value) {
  LRUNode *new_node = (LRUNode *)calloc(1, sizeof(LRUNode));
  DZ_ASSERT(new_node, "Calloc Error");
  // Copies in value
  new_node->key = strdup(key);
  new_node->value = strdup(value);
  new_node->next = NULL;
  new_node->prev = NULL;
  return new_node;
}

// Inserts a node w/ key value pair after LRUNode *prev
static void lru_insert_node_into_dll(LRUNode *node_to_insert,
                                     LRUNode *prev) {
  LRUNode *next = prev->next;
  node_to_insert->prev = prev;
  node_to_insert->next = next;
  prev->next = node_to_insert;
  next->prev = node_to_insert;
}

static void lru_insert_node_into_hm(LRUCache *self,
                                    LRUNode *node_to_insert) {
  DzHmError hm_error = DzHmError_None;
  hm_add(self->node_map, node_to_insert->key,
         strlen(node_to_insert->key) + 1, &node_to_insert,
         sizeof(node_to_insert), &hm_error);
  DZ_ASSERT(!hm_error);
}

static LRUNode *lru_get_node_from_hm(LRUCache *self,
                                     const char *key) {
  LRUNode **node_ptr =
      (LRUNode **)hm_get(self->node_map, key, strlen(key) + 1);
  if (!node_ptr) {
    return NULL;
  }
  return *node_ptr;
}

LRUNode *lru_push(LRUCache *self, const char *key,
                  const char *value) {
  LRUNode *first_node = self->head->next;
  // Look for new node in hashmap. If it does not exist, create a new
  // one, and push it after head
  LRUNode *new_node = lru_get_node_from_hm(self, key);
  if (!new_node) {
    new_node = lru_create_node(key, value);
    lru_insert_node_into_dll(new_node, self->head);
    lru_insert_node_into_hm(self, new_node);
    self->node_count++;
  }
  // Maybe modify if push and already exist
  /*else if (value != new_node->value) {*/
  /*  free(new_node->value);*/
  /*  new_node->value = strdup(value);*/
  /*}*/
  // If above capacity, remove the last node
  if (self->node_count > self->capacity) {
    LRUNode *last_node = self->tail->prev;
    lru_remove_node_from_dll(last_node);
    lru_remove_node_from_hm(self, last_node);
    lru_free_item(last_node);
    self->node_count--;
  }
  return new_node;
}

const char *lru_get(LRUCache *self, const char *key) {
  DZ_ASSERT(self);
  DZ_ASSERT(key);
  LRUNode *node = lru_get_node_from_hm(self, key);
  if (!node) {
    return NULL;
  }
  // If node exists, update priority
  // TODO: This is shit and bad
  lru_remove_node_from_dll(node);
  lru_insert_node_into_dll(node, self->head);
  return node->value;
}
