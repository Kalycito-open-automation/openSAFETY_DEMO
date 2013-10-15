SET(ENV{LANG} en)
EXECUTE_PROCESS(COMMAND "/usr/bin/gcovr" -e unittest* -x -r "/home/knallr/Development/PLK/openPOWERLINK_PIFA/src" OUTPUT_FILE "/home/knallr/Development/PLK/openPOWERLINK_PIFA/build2/PIFA_coverage.xml" WORKING_DIRECTORY "/home/knallr/Development/PLK/openPOWERLINK_PIFA/build2/unittest/")
