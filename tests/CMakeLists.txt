include(FetchContent)

# Set up Google Test

FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        release-1.11.0
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)
add_library(GTest::GTest INTERFACE IMPORTED)
target_link_libraries(GTest::GTest INTERFACE gtest_main)

include(GoogleTest)

# Add Tests

add_executable(dz_server_spam dz_server_spam.cpp)
# gtest_discover_tests(tests)
target_include_directories(dz_server_spam PRIVATE "../src")
target_link_libraries(dz_server_spam PRIVATE GTest::GTest DZHttp_Lib)

add_test(dz_server_spam_gtest dz_server_spam)
