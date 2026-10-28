cmake_minimum_required(VERSION 4.3)
project(cjournal)

set(CMAKE_CXX_STANDARD 17)

add_executable(cjournal main.cpp
        Headers/command_handler.h
        Sources/command_handler.cpp
        Classes/command.h
        Commands/list_entries.cpp
        Commands/list_entries.h
        Headers/utility.h
        Commands/open.cpp
        Commands/open.h
        Commands/edit.cpp
        Commands/edit.h
        Commands/new_.cpp
        Commands/new_.h
        Commands/remove.cpp
        Commands/remove.h
        Commands/search.cpp
        Commands/search.h
        Commands/tags.cpp
        Commands/tags.h
        Commands/stats.cpp
        Commands/stats.h
        Commands/memory.cpp
        Commands/memory.h
)

target_include_directories(cjournal PRIVATE ccolor/include)
target_link_libraries(cjournal PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/ccolor/lib/libccolor.a)
