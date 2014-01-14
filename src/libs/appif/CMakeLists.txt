CMAKE_MINIMUM_REQUIRED (VERSION 2.8)

PROJECT (appif)

########################################################################
# Set all source files
########################################################################
SET ( LIB_SRCS
    ${PROJECT_SOURCE_DIR}/async.c
    ${PROJECT_SOURCE_DIR}/cc.c
    ${PROJECT_SOURCE_DIR}/error.c
    ${PROJECT_SOURCE_DIR}/internal.c
    ${PROJECT_SOURCE_DIR}/pdo.c
    ${PROJECT_SOURCE_DIR}/status.c
    ${PROJECT_SOURCE_DIR}/stream.c
    ${PROJECT_SOURCE_DIR}/error.c
)

########################################################################
# Set include paths
########################################################################
INCLUDE_DIRECTORIES ( 
    ${PROJECT_SOURCE_DIR}/include
    ${appifcommon_SOURCE_DIR}/include
    ${endian_SOURCE_DIR}/include
    ${DEMO_CONFIG_DIR}/config/tbuf/include
    ${TARGET_DIR}/include
)

########################################################################
# Build library
########################################################################
ADD_LIBRARY ( appif ${LIB_TYPE} ${LIB_SRCS} )

TARGET_LINK_LIBRARIES ( appif "appifcommon" )
ADD_DEPENDENCIES ( appif "appifcommon" )

IF ( ( WIN32 OR CYGWIN ) AND ( ${LIB_TYPE} STREQUAL "SHARED" ) )
    EnsureLibraries(appif "appifcommon" )
ENDIF ( ( WIN32 OR CYGWIN ) AND ( ${LIB_TYPE} STREQUAL "SHARED" ) )
