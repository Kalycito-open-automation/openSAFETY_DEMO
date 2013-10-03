CMAKE_MINIMUM_REQUIRED (VERSION 2.8)

PROJECT (libAppIflite C)

########################################################################
# Set paths
########################################################################
SET ( COMMON_LIB_DIR ${PROJECT_SOURCE_DIR}/../common/appif-lite )
SET ( COMMON_GEN_DIR ${PROJECT_SOURCE_DIR}/../common/generic )

########################################################################
# Set all source files
########################################################################
SET ( LIB_SRCS
    ${PROJECT_SOURCE_DIR}/src/appif-async.c
    ${PROJECT_SOURCE_DIR}/src/appif-cc.c
    ${PROJECT_SOURCE_DIR}/src/appif-error.c
    ${PROJECT_SOURCE_DIR}/src/appif-internal.c
    ${PROJECT_SOURCE_DIR}/src/appif-pdo.c
    ${PROJECT_SOURCE_DIR}/src/appif-status.c
    ${PROJECT_SOURCE_DIR}/src/appif-stream.c
    ${COMMON_LIB_DIR}/src/appif-ccobject.c
    ${COMMON_LIB_DIR}/src/appif-timeout.c
    ${COMMON_GEN_DIR}/src/amiLittle.c
)

########################################################################
# Set include paths
########################################################################
INCLUDE_DIRECTORIES ( ${PROJECT_SOURCE_DIR}/inc/ 
    ${COMMON_LIB_DIR}/inc/
    ${COMMON_GEN_DIR}/inc/
)


########################################################################
# Build library
########################################################################
ADD_LIBRARY ( libAppIflite ${LIB_SRCS} )
