CMAKE_MINIMUM_REQUIRED (VERSION 2.8)

PROJECT (tst_appif)

#################################################################
# Drivers implement the tests and provide the test methods
#################################################################
SET ( TST_DRIVER
    ${PROJECT_SOURCE_DIR}/Driver/ExampleAddTest.c
    ${PROJECT_SOURCE_DIR}/Driver/TST_module_ssdo.c
    ${PROJECT_SOURCE_DIR}/Driver/TST_module_status.c
)

# Provide all stubs needed for running the tests
SET ( TST_STUBS
)


#################################################################
# additional compiler flags
#################################################################
ADD_DEFINITIONS(-D_CONSOLE -DHAVE_REMOTE)
ADD_DEFINITIONS( -DNDEBUG -DDEF_DEBUG_LVL=0x00000000L )

#################################################################
# assemble source file list
#################################################################
SET (TST_SOURCES 
    ${CMAKE_SOURCE_DIR}/unittest/common/cunit_main.c
    ${TST_DRIVER}
    ${TST_STUBS}
)
SOURCE_GROUP("Test Application" FILES ${TST_SOURCES})

SimpleTest ( "TST_AppIF" "tst_appif" "${TST_SOURCES}" )

SET_TARGET_INCLUDE ( tst_appif "${PROJECT_SOURCE_DIR}" )
SET_TARGET_INCLUDE ( tst_appif "${PROJECT_SOURCE_DIR}/Driver" )
SET_TARGET_INCLUDE ( tst_appif "${CMAKE_SOURCE_DIR}/libs/appifcommon/include" )
SET_TARGET_INCLUDE ( tst_appif "${CMAKE_SOURCE_DIR}/libs/appif/include" )
SET_TARGET_INCLUDE ( tst_appif "${DEMO_CONFIG_DIR}/config/tbuf/include" )
SET_TARGET_INCLUDE ( tst_appif "${TARGET_DIR}/include" )
SET_TARGET_INCLUDE ( tst_appif "${CMAKE_SOURCE_DIR}/libs/endian/include" )
IF (WIN32)
    SET_TARGET_INCLUDE ( tst_appif "${CMAKE_SOURCE_DIR}/contrib/win32" )
    
    TARGET_LINK_LIBRARIES( tst_appif "win32" )
    ADD_DEPENDENCIES ( tst_appif "win32")
endif (WIN32)

TARGET_LINK_LIBRARIES(tst_appif "appif" "appifcommon" "endian")
ADD_DEPENDENCIES ( tst_appif "appif" "appifcommon" "endian")

IF ( ( WIN32 OR CYGWIN ) AND ( ${LIB_TYPE} STREQUAL "SHARED" ) )
    EnsureLibraries(tst_appif "appif" )
    EnsureLibraries(tst_appif "appifcommon" )
    EnsureLibraries(tst_appif "endian" )
ENDIF ( ( WIN32 OR CYGWIN ) AND ( ${LIB_TYPE} STREQUAL "SHARED" ) )

# Enable Coverage on target and test subject
AddCoverage( "PIFA" "tst_appif" )
AddCoverage( "PIFA" "appif" )
AddCoverage( "PIFA" "appifcommon" )
AddCoverage( "PIFA" "endian" )
