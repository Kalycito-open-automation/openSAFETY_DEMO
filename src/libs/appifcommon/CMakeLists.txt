CMAKE_MINIMUM_REQUIRED (VERSION 2.8)

PROJECT (appifcommon)

########################################################################
# Set all source files
########################################################################
SET ( LIB_SRCS
    ${PROJECT_SOURCE_DIR}/ccobject.c
    ${PROJECT_SOURCE_DIR}/timeout.c
)

########################################################################
# Set include paths
########################################################################
INCLUDE_DIRECTORIES ( 
    ${PROJECT_SOURCE_DIR}/include
    ${endian_SOURCE_DIR}/include
    ${TARGET_DIR}/include
    ${DEMO_CONFIG_DIR}/config/tbuf/include
)

########################################################################
# Build library
########################################################################
ADD_LIBRARY ( appifcommon ${LIB_TYPE} ${LIB_SRCS} )

TARGET_LINK_LIBRARIES ( appifcommon "endian" )
ADD_DEPENDENCIES ( appifcommon "endian" )

IF ( ( WIN32 OR CYGWIN ) AND ( ${LIB_TYPE} STREQUAL "SHARED" ) )
    EnsureLibraries(appifcommon "endian" )
ENDIF ( ( WIN32 OR CYGWIN ) AND ( ${LIB_TYPE} STREQUAL "SHARED" ) )
