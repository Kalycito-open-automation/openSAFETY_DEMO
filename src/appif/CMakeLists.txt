CMAKE_MINIMUM_REQUIRED (VERSION 2.8)

PROJECT (appif)

########################################################################
# Set all source files
########################################################################
SET ( LIB_SRCS
    ${PROJECT_SOURCE_DIR}/appif-async.c
    ${PROJECT_SOURCE_DIR}/appif-cc.c
    ${PROJECT_SOURCE_DIR}/appif-error.c
    ${PROJECT_SOURCE_DIR}/appif-internal.c
    ${PROJECT_SOURCE_DIR}/appif-pdo.c
    ${PROJECT_SOURCE_DIR}/appif-status.c
    ${PROJECT_SOURCE_DIR}/appif-stream.c
    ${PROJECT_SOURCE_DIR}/appif-ccobject.c
    ${PROJECT_SOURCE_DIR}/appif-timeout.c
    ${PROJECT_SOURCE_DIR}/amiLittle.c
)

########################################################################
# Set include paths
########################################################################
INCLUDE_DIRECTORIES ( 
    ${PROJECT_SOURCE_DIR}/include/ 
    ${PROJECT_SOURCE_DIR}/include/ 
)

########################################################################
# Build library
########################################################################
ADD_LIBRARY ( appif ${LIB_TYPE} ${LIB_SRCS} )
