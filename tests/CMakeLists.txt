add_executable(psd_writer_tests psdtests.cpp)

target_include_directories(psd_writer_tests PUBLIC "${PROJECT_SOURCE_DIR}/include/${PROJECT_NAME}")
target_link_libraries(psd_writer_tests PUBLIC ${PROJECT_NAME})

# Library needs to be accessible at runtime.
add_custom_command(
    TARGET psd_writer_tests POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    $<TARGET_FILE:${PROJECT_NAME}>
    $<TARGET_FILE_DIR:psd_writer_tests>)

# Make profile available.
add_custom_command(
    TARGET psd_writer_tests POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${PROJECT_SOURCE_DIR}/tests/data/sRGB_v4_ICC_preference.icc"
    ${CMAKE_CURRENT_BINARY_DIR})

add_test(NAME psd_test COMMAND psd_writer_tests)

#find_package(Python REQUIRED)
#add_test(python_test ${Python_EXECUTABLE} "${PROJECT_SOURCE_DIR}/python/psdtests.py")