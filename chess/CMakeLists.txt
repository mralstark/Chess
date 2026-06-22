cmake_minimum_required(VERSION 3.16)
project(chess LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# On Windows + MinGW you may need to set SFML_DIR (or add it to CMAKE_PREFIX_PATH).
# Example:
#   cmake -S . -B build -G "MinGW Makefiles" -DSFML_DIR=C:/SFML/lib/cmake/SFML
find_package(SFML 2.5 COMPONENTS graphics window system REQUIRED)

add_executable(chess
    src/main.cpp
    src/chess/Types.h
    src/chess/Board.h src/chess/Board.cpp
    src/chess/MoveGen.h src/chess/MoveGen.cpp
    src/chess/Rules.h src/chess/Rules.cpp
    src/chess/Eval.h src/chess/Eval.cpp
    src/chess/Search.h src/chess/Search.cpp
    src/chess/Engine.h src/chess/Engine.cpp
    src/gui/Gui.h src/gui/Gui.cpp
)

target_include_directories(chess PRIVATE src)
target_link_libraries(chess PRIVATE sfml-graphics sfml-window sfml-system)

if (WIN32)
  target_compile_definitions(chess PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()
