cmake_minimum_required(VERSION 3.16)
project(Video2Doc)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)

# wxWidgets
find_package(wxWidgets REQUIRED COMPONENTS core base)
include(${wxWidgets_USE_FILE})

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
)

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

target_link_libraries(${PROJECT_NAME} ${wxWidgets_LIBRARIES})

# Install
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)
