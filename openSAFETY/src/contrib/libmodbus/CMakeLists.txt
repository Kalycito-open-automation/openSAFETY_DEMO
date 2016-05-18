CMAKE_MINIMUM_REQUIRED (VERSION 2.8)

PROJECT (modbus)

SET ( MODBUS_SRCS
    "${PROJECT_SOURCE_DIR}/modbus.c"
    "${PROJECT_SOURCE_DIR}/modbus.h"
    "${PROJECT_SOURCE_DIR}/modbus-data.c"
    "${PROJECT_SOURCE_DIR}/modbus-private.h"
    "${PROJECT_SOURCE_DIR}/modbus-tcp.c"
    "${PROJECT_SOURCE_DIR}/modbus-tcp.h"
    "${PROJECT_SOURCE_DIR}/modbus-tcp-private.h"
    "${PROJECT_SOURCE_DIR}/modbus-version.h"
)

SET ( ADD_ADDITIONAL_LIBRARIES )

# configure a header file to pass some of the CMake settings
# to the source code
CONFIGURE_FILE (
    "${PROJECT_SOURCE_DIR}/config.h.in"
    "${PROJECT_BINARY_DIR}/modbusconfig.h"
)

#IF ( WIN32 )
    #ADD_SUBDIRECTORY ( "${CMAKE_SOURCE_DIR}/contrib/pthread-w32/" )
#    SET ( ADD_ADDITIONAL_LIBRARIES "${ADD_ADDITIONAL_LIBRARIES}" "pthread-w32" )
#    SET ( ADD_ADDITIONAL_LIBRARIES "${ADD_ADDITIONAL_LIBRARIES}" "ws2_32" )
#    INCLUDE_DIRECTORIES("${PROJECT_SOURCE_DIR}/vcnet")
#ELSE ()
#    SET ( mxml_inline "__inline" )
#    CHECK_FUNCTION_EXISTS ( "snprintf" HAVE_SNPRINTF )
#    CHECK_FUNCTION_EXISTS ( "vsnprintf" HAVE_VSNPRINTF )
#    CHECK_FUNCTION_EXISTS ( "strdup" HAVE_STRDUP )
#    CHECK_INCLUDE_FILE ( "pthread.h" HAVE_PTHREAD_H )
#    SET ( ADD_ADDITIONAL_LIBRARIES "${ADD_ADDITIONAL_LIBRARIES}" "pthread" )
    
#    CHECK_TYPE_SIZE("long long" DATASIZE_DLONG)
#    IF(DATASIZE_DLONG GREATER 3 )
#        SET ( HAVE_LONG_LONG "1" )
#    ENDIF()
#ENDIF ( WIN32 )

INCLUDE_DIRECTORIES("${PROJECT_BINARY_DIR}")
INCLUDE_DIRECTORIES( ${PROJECT_SOURCE_DIR} )

ADD_LIBRARY ( modbus ${LIB_TYPE} ${MODBUS_SRCS} )
TARGET_LINK_LIBRARIES ( modbus ${ADD_ADDITIONAL_LIBRARIES} )
ADD_DEPENDENCIES(modbus "${ADD_ADDITIONAL_LIBRARIES}" )

IF ( ( WIN32 ) AND ( ${LIB_TYPE} STREQUAL "SHARED" ) )
    EnsureLibraries(modbus "${ADD_ADDITIONAL_LIBRARIES}" )
ENDIF ( ( WIN32 ) AND ( ${LIB_TYPE} STREQUAL "SHARED" ) )

INSTALL ( TARGETS modbus 
            RUNTIME DESTINATION lib 
            LIBRARY DESTINATION lib 
            ARCHIVE DESTINATION lib/static
         )
INSTALL ( FILES "${PROJECT_SOURCE_DIR}/modbus.h" DESTINATION include/opensafety )
INSTALL ( FILES "${PROJECT_SOURCE_DIR}/modbus-tcp.h" DESTINATION include/opensafety )
INSTALL ( FILES "${PROJECT_SOURCE_DIR}/modbus-version.h" DESTINATION include/opensafety )