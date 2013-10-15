CMAKE_MINIMUM_REQUIRED (VERSION 2.8)

PROJECT (tst_appif)

#################################################################
# Drivers implement the tests and provide the testmethods
#################################################################
SET ( TST_DRIVER
    ${PROJECT_SOURCE_DIR}/Driver/ExampleAddTest.c
    ${PROJECT_SOURCE_DIR}/Driver/TST_module_async.c
	${PROJECT_SOURCE_DIR}/Driver/TST_module_status.c
)

# Provide all stubs needed for running the tests
SET ( TST_STUBS
)


#################################################################
# additional compiler flags
#################################################################
ADD_DEFINITIONS(-D_CONSOLE -DHAVE_REMOTE)

# library compile flags
ADD_DEFINITIONS( -DNDEBUG -DDEF_DEBUG_LVL=0x00000000L -DBUILD_APPIF )

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
SET_TARGET_INCLUDE ( tst_appif "${CMAKE_SOURCE_DIR}/appif/include/" )

TARGET_LINK_LIBRARIES(tst_appif "appif")
ADD_DEPENDENCIES ( tst_appif "appif")

IF ( ( WIN32 OR CYGWIN ) AND ( ${LIB_TYPE} STREQUAL "SHARED" ) )
    EnsureLibraries(tst_appif "appif")
ENDIF ( ( WIN32 OR CYGWIN ) AND ( ${LIB_TYPE} STREQUAL "SHARED" ) )

# Enable Coverage on target and test subject
AddCoverage( "PIFA" "tst_appif" )
AddCoverage( "PIFA" "appif" )
