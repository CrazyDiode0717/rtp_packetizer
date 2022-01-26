# CMake Minimum Required Version Information
cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 11)

project(
    # Project Name
    rtp_test
)


# Project Compile Defines
#set(DEFINES
#    ${COMMON_DEFINES}
#)

# Project Compile Options
set(CFLAGS
    -ggdb
)

# Library Paths
#set(LDPATH
#
#)

# Library Files
set(LIBS
    avcodec
    avutil
)

# Compile Include Paths
#set(INCS
#    ${CMAKE_CURRENT_SOURCE_DIR}/Inc
#)

# Compile Source Files
file(GLOB_RECURSE SRC_FILES src/*.cpp src/*.h)

add_executable( ${PROJECT_NAME} ${SRC_FILES})

# Add Target Compile Definitions
#target_compile_definitions( ${PROJECT_NAME} PRIVATE     ${DEFINES})

# Add Target Compile Options
target_compile_options(     ${PROJECT_NAME} PRIVATE     ${CFLAGS})

# Add Target Compile Include Path
#target_include_directories( ${PROJECT_NAME} PRIVATE     ${INCS})

# Add Taget Link Directory Of Library
#target_link_directories(    ${PROJECT_NAME} PRIVATE     ${LDPATH})

# Add Taget Link Directory Of Library
target_link_libraries(      ${PROJECT_NAME} PRIVATE     ${LIBS})
