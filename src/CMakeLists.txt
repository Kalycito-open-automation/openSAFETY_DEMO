CMAKE_MINIMUM_REQUIRED (VERSION 2.8.4)

PROJECT (openPOWERLINK-PIFA)

SET( CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/../cmake" ${CMAKE_MODULE_PATH})

INCLUDE(AddTest)
INCLUDE(SetTargetInclude)
INCLUDE(EnsureLibraries)
INCLUDE(FindSourceFiles)
INCLUDE(CheckIncludeFile)
INCLUDE(CheckFunctionExists)

INCLUDE(CMakeOptions.txt)

INCLUDE(FindDoxygen)

IF ( UNITTEST_ENABLE )

	CONFIGURE_FILE (
		${CMAKE_SOURCE_DIR}/CTestCustom.cmake
		${CMAKE_BINARY_DIR}/CTestCustom.cmake
		COPY_ONLY
	)

	IF ( UNITTEST_XML_REPORTS )
		ADD_DEFINITIONS ( " -DCUNIT_AUTOMATED " )
	ENDIF ( UNITTEST_XML_REPORTS )
ENDIF ( UNITTEST_ENABLE )

IF ( CMAKE_BUILD_TYPE STREQUAL "coverage" AND UNITTEST_ENABLE )
    IF ( CMAKE_COMPILER_IS_GNUCXX )

        INCLUDE(FindPythonInterp)
        FIND_PACKAGE(lcov)
        FIND_PACKAGE(gcov)
        FIND_PACKAGE(gcovr)

        SET ( COVERAGE_PIFA CACHE INTERNAL "" FORCE )

        IF ( PYTHONINTERP_FOUND )
            INCLUDE(EnableCoverageReport)
        ELSE ( PYTHONINTERP_FOUND )
            MESSAGE ( FATAL_ERROR "Can not perform coverage tests, if no python interpreter is present" )
        ENDIF ( PYTHONINTERP_FOUND )

        IF ( NOT GCOVR_FOUND AND NOT GCOV_FOUND )
            MESSAGE ( STATUS "Disabling coverage tests, gcovr not found in path" )
        ENDIF ( NOT GCOVR_FOUND AND NOT GCOV_FOUND )

    ELSE ( CMAKE_COMPILER_IS_GNUCXX )
        MESSAGE ( FATAL_ERROR "Coverage is currently only possible using gcov and gnu compiler suite" )
    ENDIF ( CMAKE_COMPILER_IS_GNUCXX )

ENDIF ( CMAKE_BUILD_TYPE STREQUAL "coverage" AND UNITTEST_ENABLE )

IF ( NOT WIN32 )
	SET ( CMAKE_REQUIRED_INCLUDES "/usr/include" )
ENDIF ( NOT WIN32 )

CHECK_INCLUDE_FILE ( "sys/socket.h" HAVE_SYS_SOCKET_H )
IF ( WIN32 )
    CHECK_INCLUDE_FILE ( "winsock2.h" HAVE_WINSOCK2_H )
    CHECK_INCLUDE_FILE ( "bits/predefs.h" HAVE_BITS_PREDEFS_H )
ENDIF ( WIN32 )
CHECK_INCLUDE_FILE ( "netinet/in.h" HAVE_NETINET_IN_H )
CHECK_INCLUDE_FILE ( "arpa/inet.h" HAVE_ARPA_INET_H )
CHECK_INCLUDE_FILE ( "features.h" HAVE_FEATURES_H )
CHECK_INCLUDE_FILE ( "arpa/nameser.h" HAVE_ARPA_NAMESER_H )
CHECK_INCLUDE_FILE ( "sys/param.h" HAVE_SYS_PARAM_H )
CHECK_INCLUDE_FILE ( "sys/types.h" HAVE_SYS_TYPES_H )
CHECK_INCLUDE_FILE ( "sys/libgen.h" HAVE_LIBGEN_H )
CHECK_INCLUDE_FILE ( "stdarg.h" HAVE_STDARG_H )

CHECK_FUNCTION_EXISTS( "bzero" HAVE_BZERO )
CHECK_FUNCTION_EXISTS( "inet_pton" HAVE_INET_PTON )

CHECK_FUNCTION_EXISTS( "snprintf" HAVE_SNPRINTF )
CHECK_FUNCTION_EXISTS( "strdup" HAVE_STRDUP )
CHECK_FUNCTION_EXISTS( "vsnprintf" HAVE_VSNPRINTF )

SET ( WIN32_EXECUTABLE "" )

# Set the LIB_TYPE variable to STATIC
SET (LIB_TYPE STATIC)
IF (BUILD_SHARED_LIBS)
  # User wants to build Dynamic Libraries, so change the LIB_TYPE variable to CMake keyword 'SHARED'
  SET (LIB_TYPE SHARED)
ENDIF (BUILD_SHARED_LIBS)

IF ( WIN32 )
    SET ( WIN32_EXECUTABLE "WIN32" )
    IF (CMAKE_COMPILER_IS_GNUCC)
        MESSAGE ( STATUS "Cross-compiling for Windows" )
        ADD_DEFINITIONS ( "-DWIN32 -D_WIN32_WINNT=0x0501" )
    ELSE (CMAKE_COMPILER_IS_GNUCC)
        MESSAGE ( STATUS "Building on Windows" )
        ADD_DEFINITIONS ( "/D_WIN32_WINNT=0x0501" )
    ENDIF (CMAKE_COMPILER_IS_GNUCC)
    ADD_SUBDIRECTORY ( "${PROJECT_SOURCE_DIR}/contrib/" )

ELSE ( WIN32 )

    IF ( CYGWIN )
        MESSAGE ( STATUS "Building on Cygwin/Windows" )
    ELSE ( CYGWIN )
        MESSAGE ( STATUS "Building on Linux" )

    # Workaround for 64-Bit
    # The current stack implementation is not able to handle real 64-bit mode.
    # Problems occur, because the code tries to assign pointers (which have 8 byte in 64-bit)
    # to UINT32 which are by nature only 32bit. Solution for now is to allow only 32bit target
    # being built on Linux, which is handled by adding -m32 to gcc
    IF(UNIX)
        IF (CMAKE_COMPILER_IS_GNUCC)
            # We have a 64 bit compiler, let's add a switch for 32 bit
            IF ( CMAKE_SIZEOF_VOID_P EQUAL 8 )
                SET( CMAKE_C_FLAGS ${CMAKE_C_FLAGS} "-m32 -Wformat=0")
            ENDIF ( CMAKE_SIZEOF_VOID_P EQUAL 8 )
        ENDIF(CMAKE_COMPILER_IS_GNUCC)
    ENDIF(UNIX)
    ENDIF ( CYGWIN )

ENDIF ( WIN32 )


#######################
## DOXYGEN
#######################

IF ( DOXYGEN_FOUND )

    SET ( DOXYFILE_OUTPUT_DIR "${CMAKE_BINARY_DIR}/doc" )
    SET ( DOXYFILE_IMAGE_DIR "${CMAKE_SOURCE_DIR}/../doc" )
    SET ( DOXYFILE_SOURCE_DIR "${CMAKE_SOURCE_DIR}" )

    IF ( DOCUMENTATION_GENERATE_LATEX )
        SET ( DOXYFILE_LATEX "YES" )
    ELSE ( DOCUMENTATION_GENERATE_LATEX )
        SET ( DOXYFILE_LATEX "NO" )
    ENDIF ( DOCUMENTATION_GENERATE_LATEX )

    INCLUDE(UseDoxygen OPTIONAL)
ENDIF ( DOXYGEN_FOUND  )


#######################
## Documentation
#######################

ADD_SUBDIRECTORY ("${PROJECT_SOURCE_DIR}/appif" )

IF ( UNITTEST_ENABLE )

    ENABLE_TESTING()

    ADD_SUBDIRECTORY ( "${PROJECT_SOURCE_DIR}/contrib/cunit" )
    INCLUDE_DIRECTORIES ( "${PROJECT_SOURCE_DIR}/contrib/cunit/include/" )
    INCLUDE_DIRECTORIES ( "${PROJECT_SOURCE_DIR}/unittest/common/" )
    ADD_SUBDIRECTORY ( "${PROJECT_SOURCE_DIR}/unittest" )

    IF ( GCOVR_FOUND AND PYTHONINTERP_FOUND )
        ENABLE_COVERAGE_REPORT ( TARGETS ${COVERAGE_PIFA} COVERAGE "PIFA" BINARY_DIR "${CMAKE_BINARY_DIR}/unittest/" FILTER "unittest/*" )
    ENDIF ( GCOVR_FOUND AND PYTHONINTERP_FOUND )

ENDIF ( UNITTEST_ENABLE )
