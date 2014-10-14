#### GenEclipseFileList

# Macro for generating a file list suiteable for a eclipse .project file
MACRO(GenEclipseFileList IN_SRC_LIST FOLDER_PREFIX RES_SRC_LIST )

    SET(TMP_RESULT "")

    FOREACH ( SRC_FILE IN ITEMS ${IN_SRC_LIST} )
        GET_FILENAME_COMPONENT(SRC_NAME ${SRC_FILE} NAME)
        IF ( "${FOLDER_PREFIX}" STREQUAL "" )
            SET ( TMP_RESULT "${TMP_RESULT}\t\t<link>\r\t\t\t<name>${SRC_NAME}</name>\r\t\t\t<type>1</type>\r\t\t\t<location>${SRC_FILE}</location>\r\t\t</link>\r")
        ELSE ()
            SET ( TMP_RESULT "${TMP_RESULT}\t\t<link>\r\t\t\t<name>${FOLDER_PREFIX}/${SRC_NAME}</name>\r\t\t\t<type>1</type>\r\t\t\t<location>${SRC_FILE}</location>\r\t\t</link>\r")
        ENDIF ( "${FOLDER_PREFIX}" STREQUAL "" )
    ENDFOREACH ( SRC_FILE )

    # Add to result list
    SET( ${RES_SRC_LIST} ${TMP_RESULT} )
ENDMACRO(GenEclipseFileList)