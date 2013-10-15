CMAKE_MINIMUM_REQUIRED (VERSION 2.8)

PROJECT (appifcommon)

########################################################################
# Set all source files
########################################################################
SET ( LIB_SRCS
    ${PROJECT_SOURCE_DIR}/appif-ccobject.c
    ${PROJECT_SOURCE_DIR}/appif-timeout.c
    ${PROJECT_SOURCE_DIR}/amiLittle.c
)

########################################################################
# Set include paths
########################################################################
INCLUDE_DIRECTORIES ( 
    ${PROJECT_SOURCE_DIR}/include/ 
)

########################################################################
# Build library
########################################################################
ADD_LIBRARY ( appifcommon ${LIB_TYPE} ${LIB_SRCS} )
