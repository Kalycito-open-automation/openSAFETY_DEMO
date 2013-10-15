MACRO(FindSourceFiles SEARCH_DIRECTORY TARGET_VARIABLE)

    FILE ( GLOB_RECURSE  TARGET_VARIABLE_HEADERS "${SEARCH_DIRECTORY}/*.h" )
    FILE ( GLOB_RECURSE  TARGET_VARIABLE_CINCLUDES "${SEARCH_DIRECTORY}/*.cin" )
    FILE ( GLOB_RECURSE  TARGET_VARIABLE_SOURCE "${SEARCH_DIRECTORY}/*.c" )
    
    SET ( ${TARGET_VARIABLE} ${TARGET_VARIABLE_HEADERS} ${TARGET_VARIABLE_CINCLUDES} ${TARGET_VARIABLE_SOURCE} )

    SET ( TARGET_VARIABLE_HEADERS )
    SET ( TARGET_VARIABLE_SOURCE )

ENDMACRO(FindSourceFiles)