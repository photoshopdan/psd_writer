set(SOURCE_FILE_LIST
    cwrapper.cpp
    psddata.cpp
    psdimage.cpp
    psdocument.cpp
    psdwriter.cpp)

set(HEADER_FILE_LIST
    "${PROJECT_SOURCE_DIR}/include/${PROJECT_NAME}/psddata.hpp"
    "${PROJECT_SOURCE_DIR}/include/${PROJECT_NAME}/psdimage.hpp"
    "${PROJECT_SOURCE_DIR}/include/${PROJECT_NAME}/psdocument.hpp"
    "${PROJECT_SOURCE_DIR}/include/${PROJECT_NAME}/psdtypes.hpp"
    "${PROJECT_SOURCE_DIR}/include/${PROJECT_NAME}/psdwriter.hpp")

add_library(
    ${PROJECT_NAME}
    SHARED
    ${SOURCE_FILE_LIST}
    ${HEADER_FILE_LIST})

target_include_directories(${PROJECT_NAME} PUBLIC "${PROJECT_SOURCE_DIR}/include/${PROJECT_NAME}")
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)
