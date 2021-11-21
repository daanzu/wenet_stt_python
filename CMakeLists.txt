cmake_minimum_required(VERSION 3.15 FATAL_ERROR)

cmake_policy(SET CMP0091 NEW)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

message("CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")

project(wenet_stt)

# https://stackoverflow.com/questions/9742003/platform-detection-in-cmake
if(WIN32)
    macro(get_WIN32_WINNT version)
        if(CMAKE_SYSTEM_VERSION)
            set(ver ${CMAKE_SYSTEM_VERSION})
            string(REGEX MATCH "^([0-9]+).([0-9])" ver ${ver})
            string(REGEX MATCH "^([0-9]+)" verMajor ${ver})
            # Check for Windows 10, b/c we'll need to convert to hex 'A'.
            if("${verMajor}" MATCHES "10")
                set(verMajor "A")
                string(REGEX REPLACE "^([0-9]+)" ${verMajor} ver ${ver})
            endif()
            # Remove all remaining '.' characters.
            string(REPLACE "." "" ver ${ver})
            # Prepend each digit with a zero.
            string(REGEX REPLACE "([0-9A-Z])" "0\\1" ver ${ver})
            set(${version} "0x${ver}")
        endif()
    endmacro()
    get_WIN32_WINNT(ver)
    add_definitions(-D_WIN32_WINNT=${ver})
endif()

add_subdirectory(native/wenet/runtime/server/x86)
if(NOT MSVC)
    link_directories(${openfst_PREFIX_DIR}/lib)
else()
    link_directories(${boost_SOURCE_DIR}/stage/lib)
endif()

find_package(Torch REQUIRED)
message("wenet_stt CMakeLists: ${TORCH_CXX_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${TORCH_CXX_FLAGS}")

add_library(wenet_stt_lib SHARED
    native/wenet_stt_lib.cpp
)
target_include_directories(wenet_stt_lib PRIVATE
    native/wenet/runtime/server/x86
    native/wenet/runtime/server/x86/kaldi
    ${openfst_SOURCE_DIR}/src/include
    ${gflags_BINARY_DIR}/include
    ${glog_SOURCE_DIR}/src ${glog_BINARY_DIR}
)
target_link_libraries(wenet_stt_lib
    frontend
    decoder
)
set_property(TARGET wenet_stt_lib PROPERTY CXX_STANDARD 14)

add_custom_command(
    TARGET wenet_stt_lib POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        $<TARGET_FILE:wenet_stt_lib>
        ${PROJECT_SOURCE_DIR}/src/wenet_stt/
    COMMENT "Installing wenet_stt_lib shared library file to python project directory"
)

if(WIN32)
    add_custom_command(
        TARGET wenet_stt_lib POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
            asmjit.dll c10.dll fbgemm.dll libiomp5md.dll torch_cpu.dll uv.dll
            ${PROJECT_SOURCE_DIR}/src/wenet_stt/
        COMMENT "Installing required shared library dependency files to python project directory"
    )
endif(WIN32)

# set(CMAKE_VERBOSE_MAKEFILE ON CACHE BOOL "ON" FORCE)

# get_cmake_property(_variableNames VARIABLES)
# list (SORT _variableNames)
# foreach (_variableName ${_variableNames})
#     message(STATUS "${_variableName}=${${_variableName}}")
# endforeach()
