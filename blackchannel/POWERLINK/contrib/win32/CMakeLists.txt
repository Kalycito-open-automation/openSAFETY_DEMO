CMAKE_MINIMUM_REQUIRED (VERSION 2.8)

PROJECT (win32)

########################################################################
# Set all source files
########################################################################
SET ( GETOPT_SRCS
    ${PROJECT_SOURCE_DIR}/getopt.c
)
SOURCE_GROUP ( GetOpt FILES ${GETOPT_SRCS} )

########################################################################
# Build library
########################################################################
ADD_LIBRARY ( win32 STATIC ${GETOPT_SRCS} )


