CMAKE_MINIMUM_REQUIRED (VERSION 2.8)

PROJECT (endian)

########################################################################
# Set all source files
########################################################################
SET ( LIB_SRCS
    ${PROJECT_SOURCE_DIR}/amilittle.c
)

########################################################################
# Set include paths
########################################################################
INCLUDE_DIRECTORIES ( 
    ${PROJECT_SOURCE_DIR}/include
    ${TARGET_DIR}/include
)

########################################################################
# Build library
########################################################################
ADD_LIBRARY ( endian ${LIB_TYPE} ${LIB_SRCS} )
