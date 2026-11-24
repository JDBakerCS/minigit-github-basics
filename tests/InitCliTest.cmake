cmake_minimum_required(VERSION 3.20)

if(NOT DEFINED MINIGIT_EXECUTABLE OR NOT EXISTS "${MINIGIT_EXECUTABLE}")
    message(FATAL_ERROR "The minigit executable was not provided.")
endif()

if(NOT DEFINED TEST_BASE OR NOT IS_DIRECTORY "${TEST_BASE}")
    message(FATAL_ERROR "The test base directory was not provided.")
endif()

string(RANDOM LENGTH 12 ALPHABET 0123456789abcdef suffix)
set(testRoot "${TEST_BASE}/minigit-init-test-${suffix}")
file(MAKE_DIRECTORY "${testRoot}")

function(runMinigit project expectedCode expectedText)
    execute_process(
        COMMAND "${MINIGIT_EXECUTABLE}" ${ARGN}
        WORKING_DIRECTORY "${project}"
        RESULT_VARIABLE result
        OUTPUT_VARIABLE output
        ERROR_VARIABLE error
    )
    if(NOT "${result}" STREQUAL "${expectedCode}")
        message(FATAL_ERROR "minigit ${ARGN} exited ${result}, expected ${expectedCode}: ${output}${error}")
    endif()
    string(FIND "${output}${error}" "${expectedText}" position)
    if(position EQUAL -1)
        message(FATAL_ERROR "minigit ${ARGN} did not print '${expectedText}': ${output}${error}")
    endif()
endfunction()

function(checkText path expected)
    if(NOT EXISTS "${path}")
        message(FATAL_ERROR "Missing file: ${path}")
    endif()
    file(READ "${path}" actual)
    if(NOT "${actual}" STREQUAL "${expected}")
        message(FATAL_ERROR "File changed: ${path}")
    endif()
endfunction()

function(checkMetadataEntries project)
    set(metadata "${project}/.minigit")
    file(GLOB entries RELATIVE "${metadata}" "${metadata}/*")
    list(SORT entries)
    if(NOT "${entries}" STREQUAL "HEAD;commits;index.txt;stage")
        message(FATAL_ERROR "Unexpected metadata entries: ${entries}")
    endif()
endfunction()

function(checkEmptyLayout project)
    set(metadata "${project}/.minigit")
    if(NOT IS_DIRECTORY "${metadata}/stage" OR
       NOT IS_DIRECTORY "${metadata}/commits")
        message(FATAL_ERROR "Missing metadata directory in ${project}")
    endif()
    checkMetadataEntries("${project}")
    checkText("${metadata}/HEAD" "")
    checkText("${metadata}/index.txt" "")
    file(GLOB stageEntries "${metadata}/stage/*")
    file(GLOB commitEntries "${metadata}/commits/*")
    if(stageEntries OR commitEntries)
        message(FATAL_ERROR "New repository contains staged files or commits.")
    endif()
endfunction()

# A new empty project gets exactly the agreed metadata layout.
set(emptyProject "${testRoot}/empty")
file(MAKE_DIRECTORY "${emptyProject}")
runMinigit("${emptyProject}" 0 "Initialized" init)
checkEmptyLayout("${emptyProject}")

# Init must leave ordinary Working files alone.
set(workingProject "${testRoot}/working")
file(MAKE_DIRECTORY "${workingProject}")
file(WRITE "${workingProject}/notes.txt" "Working data\n")
runMinigit("${workingProject}" 0 "Initialized" init)
checkEmptyLayout("${workingProject}")
checkText("${workingProject}/notes.txt" "Working data\n")

# A valid repository with data is a no-op on repeated init.
set(metadata "${workingProject}/.minigit")
file(WRITE "${metadata}/HEAD" "7")
file(WRITE "${metadata}/index.txt" "notes.txt\n")
file(WRITE "${metadata}/stage/notes.txt" "STAGED")
file(MAKE_DIRECTORY "${metadata}/commits/7")
file(WRITE "${metadata}/commits/7/message.txt" "Saved")
runMinigit("${workingProject}" 0 "Already" init)
checkMetadataEntries("${workingProject}")
checkText("${metadata}/HEAD" "7")
checkText("${metadata}/index.txt" "notes.txt\n")
checkText("${metadata}/stage/notes.txt" "STAGED")
checkText("${metadata}/commits/7/message.txt" "Saved")
checkText("${workingProject}/notes.txt" "Working data\n")

# Missing HEAD is an error and must not be repaired.
file(REMOVE "${metadata}/HEAD")
runMinigit("${workingProject}" 1 "Error:" init)
if(EXISTS "${metadata}/HEAD")
    message(FATAL_ERROR "Init repaired a missing HEAD.")
endif()
checkText("${metadata}/index.txt" "notes.txt\n")
checkText("${metadata}/stage/notes.txt" "STAGED")
checkText("${metadata}/commits/7/message.txt" "Saved")
checkText("${workingProject}/notes.txt" "Working data\n")

# Invalid command lines do not create metadata or change Working files.
set(argumentsProject "${testRoot}/arguments")
file(MAKE_DIRECTORY "${argumentsProject}")
file(WRITE "${argumentsProject}/notes.txt" "Keep this file\n")
runMinigit("${argumentsProject}" 1 "Error:")
runMinigit("${argumentsProject}" 1 "Error:" status)
runMinigit("${argumentsProject}" 1 "Error:" init extra)
if(EXISTS "${argumentsProject}/.minigit")
    message(FATAL_ERROR "Invalid command created metadata.")
endif()
checkText("${argumentsProject}/notes.txt" "Keep this file\n")

file(REMOVE_RECURSE "${testRoot}")
