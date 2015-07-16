add_library(gtest src/gtest-all.cc src/gtest_main.cc)
target_include_directories(gtest SYSTEM PUBLIC include)
target_link_libraries(gtest pthread)
