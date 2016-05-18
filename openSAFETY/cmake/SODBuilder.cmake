#### SODBuilder

# Generates the necessary eplscfg.h and sod.h based on a given osdd file
MACRO(SODBuilder OUTPUT_PATH OSDD_PATH)

    IF ( NOT EXISTS ${OSDD_PATH} )
        MESSAGE ( FATAL_ERROR "SODBuilder:: The provided OSDD path [${OSDD_PATH}] does not exist" )
        RETURN ()
    ENDIF ( NOT EXISTS ${OSDD_PATH} )

    GET_FILENAME_COMPONENT ( _sodPath ${OSDD_PATH} NAME )
    GET_TARGET_PROPERTY(sbLocation sodbuilder LOCATION)

    ADD_CUSTOM_COMMAND(
        OUTPUT "${OUTPUT_PATH}"
        COMMAND ${sbLocation}
        ARGS "${OUTPUT_PATH}" "${OSDD_PATH}"
        COMMENT "Creating stack configuration  based on ${_sodPath}"
        DEPENDS "sodbuilder" ${OSDD_PATH}
        VERBATIM
    )

ENDMACRO(SODBuilder)
