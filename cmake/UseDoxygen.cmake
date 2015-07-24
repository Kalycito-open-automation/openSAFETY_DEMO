# - Run Doxygen
#
# Adds a doxygen target that runs doxygen to generate the html
# and optionally the LaTeX API documentation.
# The doxygen target is added to the doc target as a dependency.
# i.e.: the API documentation is built with:
#  make doc
#
# USAGE: GLOBAL INSTALL
#
# Install it with:
#  cmake ./ && sudo make install
# Add the following to the CMakeLists.txt of your project:
#  INCLUDE(UseDoxygen OPTIONAL)
# Optionally copy Doxyfile.in in the directory of CMakeLists.txt and edit it.
#
# USAGE: INCLUDE IN PROJECT
#
#  SET(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR})
#  INCLUDE(UseDoxygen)
# Add the Doxyfile.in and UseDoxygen.cmake files to the projects source directory.
#
#
# Variables you may define are:
#  DOXYFILE_SOURCE_DIR - Path where the Doxygen input files are.
#      Defaults to the current source and binary directory.
#  DOXYFILE_OUTPUT_DIR - Path where the Doxygen output is stored. Defaults to "doc".
#
#  DOXYFILE_LATEX - Set to "NO" if you do not want the LaTeX documentation
#      to be built.
#  DOXYFILE_LATEX_DIR - Directory relative to DOXYFILE_OUTPUT_DIR where
#      the Doxygen LaTeX output is stored. Defaults to "latex".
#
#  DOXYFILE_HTML_DIR - Directory relative to DOXYFILE_OUTPUT_DIR where
#      the Doxygen html output is stored. Defaults to "html".
#

#
#  Copyright (c) 2009, 2010 Tobias Rautenkranz <tobias@rautenkranz.ch>
#
#  Redistribution and use is allowed according to the terms of the New
#  BSD license.
#  For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#

MACRO(usedoxygen_SET_default name value)
    IF(NOT DEFINED "${name}")
        SET("${name}" "${value}")
    ENDIF()
ENDMACRO()

FIND_PACKAGE(Doxygen)

IF(DOXYGEN_FOUND)
    FIND_FILE(DOXYFILE_IN "doxyfile.in"
            PATHS "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_SOURCE_DIR}/doc/doxygen" "${CMAKE_ROOT}/Modules/"
            NO_DEFAULT_PATH)
    SET(DOXYFILE "${DOXYFILE_OUTPUT_DIR}/doxyfile")
    INCLUDE(FindPackageHandleStandardArgs)
    FIND_PACKAGE_handle_standard_args(DOXYFILE_IN DEFAULT_MSG "DOXYFILE_IN")
ENDIF()

IF(DOXYGEN_FOUND AND DOXYFILE_IN_FOUND)
    usedoxygen_SET_default(DOXYFILE_OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/doc")
    usedoxygen_SET_default(DOXYFILE_HTML_DIR "html")

    SET_property(DIRECTORY APPEND PROPERTY ADDITIONAL_MAKE_CLEAN_FILES
        "${DOXYFILE_OUTPUT_DIR}/${DOXYFILE_HTML_DIR}"
    )

    ADD_CUSTOM_TARGET(doxygen
        COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYFILE}
        COMMENT "Writing documentation to ${DOXYFILE_OUTPUT_DIR}..."
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )

    ## dot
    FILE ( GLOB_RECURSE TARGET_VARIABLE_SOURCE "${DOXYFILE_IMAGE_DIR}/dot/*.dot" )

    IF( NOT ${TARGET_VARIABLE_SOURCE} STREQUAL "" )
        GET_TARGET_PROPERTY(DOT_TARGET generate_dot TYPE)
        if(NOT DOT_TARGET)
            ADD_CUSTOM_TARGET(generate_dot)
        ENDIF()

        if(NOT DOXYGEN_DOT_FILEFORMAT)
            SET(DOXYGEN_DOT_FILEFORMAT "png" )
        ENDIF()

        ADD_CUSTOM_COMMAND(
            TARGET generate_dot
            COMMAND ${CMAKE_COMMAND} -E make_directory ${DOXYFILE_OUTPUT_DIR}/${DOXYFILE_HTML_DIR}
            COMMENT "Preparing html directory"
        )

        FOREACH ( dotfile IN ITEMS ${TARGET_VARIABLE_SOURCE} )
            STRING(REPLACE "${DOXYFILE_IMAGE_DIR}/dot/" "" FILE_PATH ${dotfile} )
            STRING(REPLACE ".dot" "" FILE_NAME ${FILE_PATH} )
            ADD_CUSTOM_COMMAND(
                TARGET generate_dot
                COMMAND ${DOXYGEN_DOT_EXECUTABLE} -T${DOXYGEN_DOT_FILEFORMAT} ${dotfile} -o ${DOXYFILE_OUTPUT_DIR}/${DOXYFILE_HTML_DIR}/${FILE_NAME}.png
                COMMENT "Generating image for ${FILE_NAME} ..."
                WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/doc/
            )
            SET ( ADD_DEPEND ${ADD_DEPEND} "dot_${FILE_NAME}" )
        ENDFOREACH()

        ADD_DEPENDENCIES(generate_dot ${ADD_DEPEND})
    ENDIF ( NOT ${TARGET_VARIABLE_SOURCE} STREQUAL "" )

    SET ( ADD_DEPEND "doxygen" )

    ## LaTeX
    SET(DOXYFILE_PDFLATEX "NO")
    SET(DOXYFILE_DOT "NO")

    FIND_PACKAGE(LATEX)
    IF ( LATEX_COMPILER AND MAKEINDEX_COMPILER AND
       ( NOT DEFINED DOXYFILE_LATEX OR DOXYFILE_LATEX STREQUAL "YES" ) )
        SET(DOXYFILE_LATEX "YES")
        usedoxygen_SET_default(DOXYFILE_LATEX_DIR "latex")

        SET_property(DIRECTORY APPEND PROPERTY ADDITIONAL_MAKE_CLEAN_FILES
                "${DOXYFILE_OUTPUT_DIR}/${DOXYFILE_LATEX_DIR}"
        )

        IF(PDFLATEX_COMPILER)
            SET(DOXYFILE_PDFLATEX "YES")
        ENDIF()
        IF(DOXYGEN_DOT_EXECUTABLE)
            SET(DOXYFILE_DOT "YES")
        ENDIF()

        FIND_PROGRAM(MAKE_PROGRAM make)
        IF(MAKE_PROGRAM AND NOT WIN32)
            ADD_CUSTOM_COMMAND(TARGET doxygen
                POST_BUILD
                COMMAND ${MAKE_PROGRAM}
                COMMENT    "Running LaTeX for Doxygen documentation in ${DOXYFILE_OUTPUT_DIR}/${DOXYFILE_LATEX_DIR}..."
                WORKING_DIRECTORY "${DOXYFILE_OUTPUT_DIR}/${DOXYFILE_LATEX_DIR}"
            )
        ENDIF()
    ELSE()
        SET(DOXYGEN_LATEX "NO")
    ENDIF()

    CONFIGURE_FILE(${DOXYFILE_IN} ${DOXYFILE} IMMEDIATE @ONLY)

    IF(NOT TARGET doc)
        ADD_CUSTOM_TARGET(doc)
    ENDIF()

    ADD_DEPENDENCIES(doc ${ADD_DEPEND})
ENDIF()
