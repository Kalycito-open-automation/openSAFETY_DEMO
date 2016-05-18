#### PySODBuilder

# Generates the necessary eplscfg.h and sod.h based on a given osdd file
MACRO ( PySODBuilder SOURCE_FILE_PATH OUTPUT_PATH
        OSDD_PATH SETTINGS_FILE MODULE_ID )

    IF ( NOT EXISTS ${OSDD_PATH} )
        SET ( MSG_STR "PySODBuilder:: The provided OSDD path")
        SET ( MSG_STR "${MSG_STR} [${OSDD_PATH}] does not exist" )
        MESSAGE ( FATAL_ERROR "${MSG_STR}" )
        RETURN ()
    ENDIF ( NOT EXISTS ${OSDD_PATH} )

    IF ( NOT EXISTS ${SOURCE_FILE_PATH} )
        SET ( MSG_STR "The provided PYSODBUILDER_INPUT_DIR")
        SET ( MSG_STR "${MSG_STR} [${SOURCE_FILE_PATH}]  does not exist" )
        MESSAGE ( FATAL_ERROR "${MSG_STR}" )
        RETURN ()
    ENDIF ( NOT EXISTS ${SOURCE_FILE_PATH} )

    GET_FILENAME_COMPONENT ( _osddPath ${OSDD_PATH} NAME )

    SET ( PYSODBUILDER_SCRIPT "pysodbuilder.py" )

    IF ( NOT "${MODULE_ID}" STREQUAL "" )
        SET ( PYSODBUILDER_MODULE_ARGUMENT "-m ${MODULE_ID}" )
    ENDIF ()

    ADD_CUSTOM_TARGET ( pysodbuilder ALL
                COMMAND ${PYTHON_EXECUTABLE} -m pysodb.pysodbmain
                -i ${SOURCE_FILE_PATH}
                -o ${OUTPUT_PATH}
                -d ${OSDD_PATH} ${PYSODBUILDER_MODULE_ARGUMENT}
                -s ${SETTINGS_FILE}
                --remove-cog-comments
                COMMENT "Generating source files based on ${_osddPath}"
                DEPENDS pysodbuilder_binding
                WORKING_DIRECTORY ${PYSODBUILDER_BINARY_DIR}
                VERBATIM
    )

ENDMACRO( PySODBuilder SOURCE_FILE_PATH OUTPUT_PATH OSDD_PATH MODULE_ID )

# sets cache variables to demo files, if they have not been set and the variable
# DEMO_ENABLE is true
MACRO ( PySODBuilder_Set_Demo_Paths )

    IF ( DEMO_ENABLE )
        IF ( DEFINED PYSODBUILDER_INPUT_DIR AND NOT PYSODBUILDER_INPUT_DIR )
            SET ( PYSODBUILDER_INPUT_DIR
                "${PROJECT_SOURCE_DIR}/demo_files/input"
                CACHE PATH "directory of input source files" FORCE )
        ENDIF()

        IF ( DEFINED PYSODBUILDER_OSDD_FILE AND NOT PYSODBUILDER_OSDD_FILE )
            SET ( PYSODBUILDER_OSDD_FILE
                "${PROJECT_SOURCE_DIR}/demo_files/osdd/demo.xosdd"
                CACHE FILEPATH "osdd file to use for pysodbuilder" FORCE )
        ENDIF()

        IF ( DEFINED PYSODBUILDER_SETTINGS_FILE AND
            NOT PYSODBUILDER_SETTINGS_FILE )

            SET ( PYSODBUILDER_SETTINGS_FILE
                "${PROJECT_BINARY_DIR}/pysodbsettings.ini" CACHE FILEPATH
                "settings file to use for pysodbuilder" FORCE )
        ENDIF()

        IF ( DEFINED PYSODBUILDER_OUTPUT_DIR AND NOT PYSODBUILDER_OUTPUT_DIR )
            SET ( PYSODBUILDER_OUTPUT_DIR
                "${PROJECT_BINARY_DIR}/demo_files/output"
                CACHE PATH "direcotry of the outputfiles" FORCE )
        ENDIF()

        IF ( DEFINED PYSODBUILDER_OSDD_SCHEMA_FILE AND
            NOT PYSODBUILDER_OSDD_SCHEMA_FILE )

            IF ( NOT EXISTS ${CMAKE_BINARY_DIR}/XOSDD_R15.xsd )
                SET ( PYSODBUILDER_OSDD_SCHEMA_FILE
                    "${CMAKE_SOURCE_DIR}/../doc/XOSDD_R15.xsd" CACHE FILEPATH
                    "OSDD schema file" FORCE )
            ELSE ()
                SET ( PYSODBUILDER_OSDD_SCHEMA_FILE
                    "${CMAKE_BINARY_DIR}/XOSDD_R15.xsd" CACHE FILEPATH
                    "OSDD schema file" FORCE )
            ENDIF()
        ENDIF()
    ENDIF( DEMO_ENABLE )

ENDMACRO ( PySODBuilder_Set_Demo_Paths )
