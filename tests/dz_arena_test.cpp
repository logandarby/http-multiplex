#include <gtest/gtest.h>

extern "C" {
#include "arena.c"
}

TEST(Arena, Initialization) {
  DZArena a = dz_arena_init(100);
  ASSERT_TRUE(a.data);
  ASSERT_EQ(a.error, DzArenaError_NONE);
  ASSERT_EQ(a.first_empty_byte, 0);
  ASSERT_EQ(a.max_size, 100);
  dz_arena_free(&a);
}

TEST(Arena, Default_Init) {
  DZArena a = dz_arena_init(0);
  ASSERT_EQ(a.max_size, DZ_ARENA_DEFAULT_MAX_SIZE);
  ASSERT_EQ(a.error, DzArenaError_NONE);
  dz_arena_free(&a);
}

TEST(Arena, Allocation) {
  DZArena a = dz_arena_init(100);
  char *string = (char *)dz_arena_alloc(&a, 50);
  char *string2 = (char *)dz_arena_alloc(&a, 50);
  for (size_t i = 0; i < 50; i++) {
    string[i] = 'a';
    string2[i] = 'b';
  }
  ASSERT_EQ(a.error, DzArenaError_NONE);
  ASSERT_NE(string, string2);
  ASSERT_EQ(a.max_size, 100);
  ASSERT_EQ(a.first_empty_byte, 100);
  ASSERT_EQ(string[49], 'a');
  ASSERT_EQ(string2[0], 'b');
  ASSERT_EQ(string2[49], 'b');
  dz_arena_free(&a);
}

TEST(Arena, Allocation_Init) {
  DZArena a = dz_arena_init(100);
  char *string = (char *)dz_arena_alloc(&a, 100);
  for (size_t i = 0; i < 100; i++) {
    ASSERT_EQ(string[i], '\0');
  }
  dz_arena_free(&a);
}

TEST(Arena, Clear) {
  DZArena a = dz_arena_init(100);
  char *string = (char *)dz_arena_alloc(&a, 50);
  char *string2 = (char *)dz_arena_alloc(&a, 50);
  for (size_t i = 0; i < 50; i++) {
    string[i] = 'a';
    string2[i] = 'b';
  }
  dz_arena_clear(&a);
  ASSERT_EQ(a.first_empty_byte, 0);
  ASSERT_EQ(a.error, DzArenaError_NONE);
  dz_arena_free(&a);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
