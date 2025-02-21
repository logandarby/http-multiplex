#include <gtest/gtest.h>

extern "C" {
#include "lru_cache.c"
}

TEST(LRUCache, Init) {
  LRUCache cache = lru_init(100);
  ASSERT_TRUE(cache.head);
  ASSERT_TRUE(cache.tail);
  ASSERT_FALSE(cache.tail->next);
  ASSERT_EQ(cache.tail->prev, cache.head);
  ASSERT_FALSE(cache.head->prev);
  ASSERT_EQ(cache.head->next, cache.tail);
  ASSERT_TRUE(cache.node_map);
  ASSERT_EQ(cache.node_count, 0);
  ASSERT_EQ(cache.capacity, 100);
  lru_free(&cache);
}

TEST(LRUCache, PushAndGet) {
  LRUCache cache = lru_init(100);

  // Pushing values
  const char *first_value = "1";
  LRUNode *first_node = lru_push(&cache, "1key", first_value);
  ASSERT_TRUE(first_node);
  ASSERT_EQ(first_node->prev, cache.head);
  ASSERT_EQ(first_node->next, cache.tail);
  ASSERT_STREQ(first_node->key, "1key");
  ASSERT_STREQ(first_node->value, first_value);
  ASSERT_NE(first_node->value, first_value);

  const char *second_value = "2";
  LRUNode *second_node = lru_push(&cache, "2key", second_value);
  ASSERT_TRUE(second_node);
  ASSERT_EQ(second_node->prev, cache.head);
  ASSERT_EQ(second_node->next, first_node);
  ASSERT_STREQ(second_node->key, "2key");
  ASSERT_STREQ(second_node->value, second_value);
  ASSERT_NE(second_node->value, second_value);
  ASSERT_EQ(first_node->prev, second_node);
  ASSERT_EQ(first_node->next, cache.tail);

  lru_push(&cache, "3key", "3");

  ASSERT_EQ(cache.node_count, 3);

  // Now, test getting the values
  const char *first_get_value = lru_get(&cache, "1key");
  ASSERT_TRUE(first_get_value);
  ASSERT_STREQ(first_get_value, "1");
  ASSERT_STREQ(lru_get(&cache, "2key"), "2");
  ASSERT_STREQ(lru_get(&cache, "3key"), "3");
  ASSERT_FALSE(lru_get(&cache, "4key"));

  lru_free(&cache);
}

TEST(LRUCache, Capacity) {
  LRUCache cache = lru_init(2);

  lru_push(&cache, "1key", "1");
  lru_push(&cache, "2key", "2");
  lru_push(&cache, "3key", "3");
  ASSERT_STREQ(lru_get(&cache, "2key"), "2");
  ASSERT_STREQ(lru_get(&cache, "3key"), "3");
  ASSERT_FALSE(lru_get(&cache, "1key"));

  lru_free(&cache);
}

TEST(LRUCache, PriorityAdjustment) {
  LRUCache cache = lru_init(2);

  lru_push(&cache, "1key", "1");
  lru_push(&cache, "2key", "2");
  lru_get(&cache, "1key");
  lru_push(&cache, "3key", "3");

  ASSERT_STREQ(lru_get(&cache, "1key"), "1");
  ASSERT_STREQ(lru_get(&cache, "3key"), "3");
  ASSERT_FALSE(lru_get(&cache, "2key"));

  lru_get(&cache, "1key");
  lru_push(&cache, "4key", "4");

  ASSERT_STREQ(lru_get(&cache, "1key"), "1");
  ASSERT_STREQ(lru_get(&cache, "4key"), "4");
  ASSERT_FALSE(lru_get(&cache, "3key"));

  lru_free(&cache);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
