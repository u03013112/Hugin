# Enhanced version of ADD_TEST
#
# Usage of this module:
#
#     VIGRA_ADD_TEST(target [[SOURCES] source1 source2 ...] [LIBRARIES lib1 lib2 ...])
#     VIGRA_COPY_TEST_DATA(datafile1 datafile2 ...)
#
# The function VIGRA_ADD_TEST
# * creates a new executable for 'target', using the given sources and libraries
# * makes the global target 'test' depend on the new 'target' (target 'test' must already exist)
# * installs a post-build event that runs the test automatically after linking
#
# The function VIGRA_COPY_TEST_DATA copies the given files from the current source directory
# to the corresponding binary directory.
#
MACRO(VIGRA_NATIVE_PATH out in)
    file(TO_CMAKE_PATH "${in}" ${out})
    IF(NOT CMAKE_CFG_INTDIR STREQUAL ".")
        STRING(REGEX REPLACE "\\$\\([^\\)]*\\)" "%CONFIGURATION%" ${out} "${${out}}")
    ENDIF()
    IF(MINGW)
        # turn "c:/" into "/c/"
        STRING(REGEX REPLACE "^([a-zA-Z]):" "/\\1" ${out} "${${out}}")
    ENDIF()
    file(TO_NATIVE_PATH "${${out}}" ${out})
ENDMACRO(VIGRA_NATIVE_PATH)

FUNCTION(VIGRA_ADD_TEST target)
    # parse the args
    set(v SOURCES)
    foreach(i ${ARGN})
        if(${i} MATCHES "^SOURCES$")
            set(v SOURCES)
        elseif(${i} MATCHES "^LIBRARIES$")
            set(v LIBRARIES)
        else()
            set(${v} ${${v}} ${i})
        endif()
    endforeach(i)

    FILE(GLOB TESTSUCCESS_FOUND ${CMAKE_CURRENT_BINARY_DIR}/testsuccess.cxx)
    IF(NOT TESTSUCCESS_FOUND)
        FILE(WRITE ${CMAKE_CURRENT_BINARY_DIR}/testsuccess.cxx
         "// auto-generated dummy file to force re-execution of failed tests
")
    ENDIF()
    SET(SOURCES ${SOURCES} ${CMAKE_CURRENT_BINARY_DIR}/testsuccess.cxx)

    # configure the target
    IF(AUTOBUILD_TESTS)
        ADD_EXECUTABLE(${target} ${SOURCES})
    ELSE()
        ADD_EXECUTABLE(${target} EXCLUDE_FROM_ALL ${SOURCES})
    ENDIF()

    ADD_DEPENDENCIES(check_cpp ${target})
    ADD_DEPENDENCIES(ctest ${target})
    if(DEFINED LIBRARIES)
        TARGET_LINK_LIBRARIES(${target} ${LIBRARIES})
    endif()

    # add dependencies to the PATH
    cmake_policy(PUSH)
    if(POLICY CMP0026)
        # allow 'GET_TARGET_PROPERTY(... LOCATION)'
        cmake_policy(SET CMP0026 OLD)
    endif()

    set(EXTRA_PATH "")
    IF(MSVC)
        SET(PATHSEP ";")
    ELSE()
        SET(PATHSEP ":")
    ENDIF()
    FOREACH(lib ${LIBRARIES})
        if(TARGET ${lib})
            GET_TARGET_PROPERTY(p ${lib} LOCATION)
            if(p)
                GET_FILENAME_COMPONENT(p ${p} PATH)
                VIGRA_NATIVE_PATH(p ${p})
                SET(EXTRA_PATH  "${p}${PATHSEP}${EXTRA_PATH}")
            endif()
        endif()
    ENDFOREACH(lib)
    cmake_policy(POP)

    # set up a script to run the test
    IF(MSVC)
        SET(VIGRA_TEST_EXECUTABLE "%CONFIGURATION%\\${target}.exe")
        SET(VIGRA_TEST_SCRIPT     "run_${target}.bat")
        CONFIGURE_FILE("${CMAKE_SOURCE_DIR}/config/run_test.bat.in"
                       "${CMAKE_CURRENT_BINARY_DIR}/${VIGRA_TEST_SCRIPT}"
                       @ONLY)
    ELSE()
        SET(VIGRA_TEST_EXECUTABLE "./${target}")
        SET(VIGRA_TEST_SCRIPT     "${CMAKE_CURRENT_BINARY_DIR}/run_${target}.sh")
        CONFIGURE_FILE("${CMAKE_SOURCE_DIR}/config/run_test.sh.in"
                       "${VIGRA_TEST_SCRIPT}"
                       @ONLY)
        EXECUTE_PROCESS(COMMAND chmod u+x ${VIGRA_TEST_SCRIPT} OUTPUT_QUIET ERROR_QUIET)
    ENDIF()

    # register the test execution command
    IF(NOT CMAKE_CFG_INTDIR STREQUAL ".")
        SET(VIGRA_CONFIGURATION ${CMAKE_CFG_INTDIR})
    ELSE()
        SET(VIGRA_CONFIGURATION)
    ENDIF()

    IF(AUTOEXEC_TESTS)
        add_custom_command(
            TARGET ${target}
            POST_BUILD
            COMMAND ${VIGRA_TEST_SCRIPT} ARGS ${VIGRA_CONFIGURATION}
            COMMENT "Running ${target}")
    ENDIF()

    ADD_TEST(${target} ${VIGRA_TEST_SCRIPT} ${VIGRA_CONFIGURATION})

    # if we configure for Visual Studio, setup the debug command
    if(MSVC AND NOT (MSVC_VERSION VERSION_LESS "11"))
        # FIXME: this may not portable between VC versions (works for 11.0 - 14.0)
        set(VIGRA_TEST_EXE "${CMAKE_CURRENT_BINARY_DIR}/\$(Configuration)/${target}")
        set(VIGRA_TEST_ENVIRONMENT "PATH=${EXTRA_PATH}%PATH%")
        configure_file(${CMAKE_SOURCE_DIR}/config/testdebug.vcxproj.user.in
                       ${CMAKE_CURRENT_BINARY_DIR}/${target}.vcxproj.user
                       @ONLY)
    endif()

    IF(WITH_VALGRIND AND VALGRIND_EXECUTABLE)
        IF(VALGRIND_SUPPRESSION_FILE)
            SET(VALGRIND_SUPPRESSION "--suppressions=${VALGRIND_SUPPRESSION_FILE}")
        ELSE()
            SET(VALGRIND_SUPPRESSION)
        ENDIF()
        ADD_TEST(${target}_valgrind
                ${VALGRIND_EXECUTABLE}
                ${VALGRIND_SUPPRESSION}
                --error-exitcode=1
                ${${target}_executable})
    ENDIF()

ENDFUNCTION(VIGRA_ADD_TEST)

MACRO(VIGRA_COPY_TEST_DATA)
    FOREACH(test_data ${ARGN})
        configure_file(${CMAKE_CURRENT_SOURCE_DIR}/${test_data}
                       ${CMAKE_CURRENT_BINARY_DIR}/${test_data}
                       COPYONLY)
    ENDFOREACH(test_data)
ENDMACRO(VIGRA_COPY_TEST_DATA)
