add_executable(psd_writer_tests psdtests.cpp)
target_include_directories(psd_writer_tests PUBLIC "${PROJECT_SOURCE_DIR}/include/${PROJECT_NAME}")
target_link_libraries(psd_writer_tests PUBLIC ${PROJECT_NAME})
add_test(NAME psd_test COMMAND psd_writer_tests)
