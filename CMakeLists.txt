cmake_minimum_required(VERSION 3.20)
project(KnowledgeNote VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Sql)

add_executable(${PROJECT_NAME} WIN32
    src/main.cpp
    src/MainWindow.cpp
    src/DatabaseManager.cpp
    src/NoteEditWidget.cpp
    src/NoteListWidget.cpp
    src/MainWindow.h
    src/DatabaseManager.h
    src/NoteEditWidget.h
    src/NoteListWidget.h
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::Sql
)

if(WIN32)
    set(RESOURCES_RC "${CMAKE_CURRENT_SOURCE_DIR}/resources/app.rc")
    if(EXISTS ${RESOURCES_RC})
        target_sources(${PROJECT_NAME} PRIVATE ${RESOURCES_RC})
    endif()
endif()

if(MINGW)
    target_link_options(${PROJECT_NAME} PRIVATE -mwindows)
endif()

if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /utf-8)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -finput-charset=UTF-8 -fexec-charset=UTF-8)
endif()