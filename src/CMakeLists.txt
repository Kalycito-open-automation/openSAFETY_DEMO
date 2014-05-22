################################################################################
#
# CMake main file for PIFA
#
# Copyright (c) 2013, Bernecker+Rainer Industrie-Elektronik Ges.m.b.H. (B&R)
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#     * Redistributions of source code must retain the above copyright
#       notice, this list of conditions and the following disclaimer.
#     * Redistributions in binary form must reproduce the above copyright
#       notice, this list of conditions and the following disclaimer in the
#       documentation and/or other materials provided with the distribution.
#     * Neither the name of the copyright holders nor the
#       names of its contributors may be used to endorse or promote products
#       derived from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL COPYRIGHT HOLDERS BE LIABLE FOR ANY
# DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
# ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
################################################################################

CMAKE_MINIMUM_REQUIRED (VERSION 2.8.4)

PROJECT (openPOWERLINK-PIFA C)

SET( CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/../cmake" "${CMAKE_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

INCLUDE(AddTest)
INCLUDE(SetTargetInclude)
INCLUDE(EnsureLibraries)
INCLUDE(FindSourceFiles)
INCLUDE(CheckIncludeFile)
INCLUDE(CheckFunctionExists)

INCLUDE(CMakeOptions.txt)

INCLUDE(FindDoxygen)

MESSAGE(STATUS "CMAKE_SYSTEM_NAME is ${CMAKE_SYSTEM_NAME}")
MESSAGE(STATUS "CMAKE_SYSTEM_PROCESSOR is ${CMAKE_SYSTEM_PROCESSOR}")

#######################
## SETUP DEFAULT VARS
#######################

# Set the LIB_TYPE variable to STATIC
SET (LIB_TYPE STATIC)

#######################
## UNIT TESTS
#######################
IF ( UNITTEST_ENABLE )
    CONFIGURE_FILE (
        ${CMAKE_SOURCE_DIR}/CTestCustom.cmake
        ${CMAKE_BINARY_DIR}/CTestCustom.cmake
        COPY_ONLY
    )

    IF ( UNITTEST_XML_REPORTS )
        ADD_DEFINITIONS ( " -DCUNIT_AUTOMATED " )
    ENDIF ( UNITTEST_XML_REPORTS )

    IF ( CMAKE_BUILD_TYPE STREQUAL "coverage" )
        IF ( CMAKE_COMPILER_IS_GNUCC  )

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

        ELSE ( CMAKE_COMPILER_IS_GNUCC  )
            MESSAGE ( FATAL_ERROR "Coverage is currently only possible using gcov and gnu compiler suite" )
        ENDIF ( CMAKE_COMPILER_IS_GNUCC  )

    ENDIF ( CMAKE_BUILD_TYPE STREQUAL "coverage" )
ENDIF ( UNITTEST_ENABLE )

#####################################################################
# Setup paths
SET( MISC_DIR ${CMAKE_SOURCE_DIR}/misc )
SET( FPGA_DIR ${CMAKE_SOURCE_DIR}/fpga )
SET( DEMO_CONFIG_DIR ${CMAKE_SOURCE_DIR}/app/demo-${CFG_DEMO_TYPE}/config )

#####################################################################
# Setup architecture specific configuration
IF(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    INCLUDE(configure-linux)
ELSEIF(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    INCLUDE(configure-windows)
ELSEIF((CMAKE_SYSTEM_NAME STREQUAL "Generic") AND (CMAKE_SYSTEM_PROCESSOR STREQUAL "Nios2"))
    INCLUDE(configure-nios2)
ENDIF()

#####################################################################
# Set compiler specific definitions
IF("${CMAKE_C_COMPILER_ID}" STREQUAL "GNU")
    SET(CFLAGS "-Wall -Wextra -pedantic -std=c99")
ENDIF()

#####################################################################
# Add the subprojects depending on the target platform
IF((CMAKE_SYSTEM_NAME STREQUAL "Generic"))
    # We are cross compiling for an embedded platform
    IF(CFG_INCLUDE_SUBPROJECTS MATCHES "application" OR CFG_INCLUDE_SUBPROJECTS MATCHES "all")
        SET(CURR_APPLICATION "app")
        ADD_SUBDIRECTORY("${PROJECT_SOURCE_DIR}/libs/endian" "${PROJECT_BINARY_DIR}/libs/${CURR_APPLICATION}/endian")
        ADD_SUBDIRECTORY("${PROJECT_SOURCE_DIR}/libs/appifcommon" "${PROJECT_BINARY_DIR}/libs/${CURR_APPLICATION}/appifcommon")
        ADD_SUBDIRECTORY("${PROJECT_SOURCE_DIR}/libs/appif" "${PROJECT_BINARY_DIR}/libs/${CURR_APPLICATION}/appif")

        # Add the current demo to the build
        ADD_SUBDIRECTORY("${PROJECT_SOURCE_DIR}/app/demo-${CFG_DEMO_TYPE}")
    ENDIF()

    IF(CFG_INCLUDE_SUBPROJECTS MATCHES "pcp" OR CFG_INCLUDE_SUBPROJECTS MATCHES "all" )
        SET(CURR_APPLICATION "pcp")
        ADD_SUBDIRECTORY("${PROJECT_SOURCE_DIR}/libs/appifcommon" "${PROJECT_BINARY_DIR}/libs/${CURR_APPLICATION}/appifcommon")

        ADD_SUBDIRECTORY("${PROJECT_SOURCE_DIR}/pcp/appif")
    ENDIF()
ELSE()
    # Target platform is the local machine (Windows or Linux)
    ADD_SUBDIRECTORY ("${PROJECT_SOURCE_DIR}/libs/endian" )
    ADD_SUBDIRECTORY ("${PROJECT_SOURCE_DIR}/libs/appifcommon" )
    ADD_SUBDIRECTORY ("${PROJECT_SOURCE_DIR}/libs/appif" )
ENDIF()

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
## UNIT TESTS
#######################
IF ( UNITTEST_ENABLE )

    ENABLE_TESTING()

    ADD_SUBDIRECTORY ( "${PROJECT_SOURCE_DIR}/contrib/cunit" )
    ADD_SUBDIRECTORY ( "${PROJECT_SOURCE_DIR}/unittest" )

    IF ( GCOVR_FOUND AND PYTHONINTERP_FOUND )
        ENABLE_COVERAGE_REPORT ( TARGETS ${COVERAGE_PIFA} COVERAGE "PIFA" BINARY_DIR "${CMAKE_BINARY_DIR}/unittest/" FILTER "unittest/*" )
    ENDIF ( GCOVR_FOUND AND PYTHONINTERP_FOUND )

ENDIF ( UNITTEST_ENABLE )
