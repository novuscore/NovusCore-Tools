# MIT License

# Copyright (c) 2018-2020 NovusCore

# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:

# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

project(shadercookerstandalone VERSION 1.0.0 DESCRIPTION "Standalone Shader Cooker for NovusCore")

file(GLOB_RECURSE SHADER_COOKER_STANDALONE_FILES "*.cpp" "*.h")

add_executable(${PROJECT_NAME} ${SHADER_COOKER_STANDALONE_FILES})
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER ${ROOT_FOLDER})
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 17)

find_assign_files(${SHADER_COOKER_STANDALONE_FILES})

include_directories(include/)

if(APPLE)
    target_link_libraries(${PROJECT_NAME} PRIVATE c++fs)
elseif (UNIX)
    target_link_libraries(${PROJECT_NAME} PRIVATE stdc++fs)
endif()

add_compile_definitions(NOMINMAX _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS)

target_link_libraries(${PROJECT_NAME} PRIVATE
    common::common
    shadercooker::shadercooker
)

install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

add_custom_command(
    TARGET shadercookerstandalone 
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        "${DXCOMPILER_DYNAMIC_LIB_PATH}"
        "$<TARGET_FILE_DIR:shadercookerstandalone>/dxcompiler.dll"
)