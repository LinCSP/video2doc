cmake_minimum_required(VERSION 3.16)
project(Video2Doc)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)

# wxWidgets
# Сначала пробуем CONFIG-режим (кросс-компиляция, vcpkg, статические сборки CMake)
find_package(wxWidgets CONFIG QUIET)
if(wxWidgets_FOUND)
    message(STATUS "wxWidgets found via CONFIG mode")
    set(VIDEO2DOC_WX_LIBS wx::core wx::base)
else()
    # Fallback: стандартный MODULE-режим через wx-config (Linux native, пакеты дистрибутива)
    find_package(wxWidgets REQUIRED COMPONENTS core base)
    include(${wxWidgets_USE_FILE})
    set(VIDEO2DOC_WX_LIBS ${wxWidgets_LIBRARIES})
    set(VIDEO2DOC_WX_INCLUDES ${wxWidgets_INCLUDE_DIRS})
endif()

set(SOURCES
    src/main.cpp
    src/MainFrame.cpp
    src/SettingsPanel.cpp
    src/ProjectListPanel.cpp
    src/PromptEditorPanel.cpp
    src/ControlLogPanel.cpp
    src/ProcessRunner.cpp
    src/TranscriptionEngine.cpp
    src/DocGenerator.cpp
    src/ConfigManager.cpp
    src/PathValidator.cpp
    src/DependencyChecker.cpp
)

set(HEADERS
    src/MainFrame.h
    src/SettingsPanel.h
    src/ProjectListPanel.h
    src/PromptEditorPanel.h
    src/ControlLogPanel.h
    src/ProcessRunner.h
    src/TranscriptionEngine.h
    src/DocGenerator.h
    src/ConfigManager.h
    src/PathValidator.h
    src/DependencyChecker.h
)

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

target_link_libraries(${PROJECT_NAME} ${VIDEO2DOC_WX_LIBS})
if(VIDEO2DOC_WX_INCLUDES)
    target_include_directories(${PROJECT_NAME} PRIVATE ${VIDEO2DOC_WX_INCLUDES})
endif()

# Windows-specific: статическая линковка runtime уменьшает зависимости,
# но оставляем динамическую по умолчанию для совместимости с MinGW DLL
if(MINGW)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        LINK_FLAGS "-static-libgcc -static-libstdc++"
    )
endif()

# Install
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)

# CPack packaging
set(CPACK_PACKAGE_NAME "Video2Doc")
set(CPACK_PACKAGE_VERSION "1.0.0")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Generate documentation from video using AI")
set(CPACK_PACKAGE_VENDOR "LinCSP")
set(CPACK_PACKAGE_HOMEPAGE_URL "https://github.com/LinCSP/video2doc")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")
include(CPack)
