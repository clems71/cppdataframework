# tests
add_executable(main_test tests.cpp)
target_link_libraries(main_test gtest cppdataframework)
add_test(NAME main_test COMMAND main_test)
