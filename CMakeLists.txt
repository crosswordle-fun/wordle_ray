cmake_minimum_required(VERSION 3.11) # FetchContent is available in 3.11+
project(crosswordle)

# Generate compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Dependencies
set(RAYLIB_VERSION 5.5)
find_package(raylib ${RAYLIB_VERSION} QUIET) # QUIET or REQUIRED
if (NOT raylib_FOUND) # If there's none, fetch and build raylib
  include(FetchContent)
  FetchContent_Declare(
    raylib
    DOWNLOAD_EXTRACT_TIMESTAMP OFF
    URL https://github.com/raysan5/raylib/archive/refs/tags/${RAYLIB_VERSION}.tar.gz
  )
  FetchContent_GetProperties(raylib)
  if (NOT raylib_POPULATED) # Have we downloaded raylib yet?
    set(FETCHCONTENT_QUIET NO)
    FetchContent_MakeAvailable(raylib)
  endif()
endif()

# Our Project

add_executable(${PROJECT_NAME} 
    src/main.c
    src/render.c
    src/words.c
    src/crossword_generator.c
    # Core modules
    src/core/state.c
    src/core/validation.c
    # Input modules
    src/input/input_manager.c
    # UI modules
    src/ui/camera.c
    src/ui/view_manager.c
    # Wordle modules
    src/wordle/wordle_game.c
    # Crossword modules
    src/crossword/crossword_game.c
    src/crossword/crossword_navigation.c
    # Animation modules
    src/animation/animation_system.c
    src/animation/animation_triggers.c
    src/animation/particle_system.c
)
#set(raylib_VERBOSE 1)
target_link_libraries(${PROJECT_NAME} raylib)

# Link math library on Unix systems
if(UNIX AND NOT APPLE)
    target_link_libraries(${PROJECT_NAME} m)
endif()

# Web Configurations
if (${PLATFORM} STREQUAL "Web")
    set_target_properties(${PROJECT_NAME} PROPERTIES SUFFIX ".html") # Tell Emscripten to build an example.html file.
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s USE_GLFW=3 -s ASSERTIONS=1 -s WASM=1 -s ASYNCIFY -s GL_ENABLE_GET_PROC_ADDRESS=1")
endif()

# Checks if OSX and links appropriate frameworks (Only required on MacOS)
if (APPLE)
    target_link_libraries(${PROJECT_NAME} "-framework IOKit")
    target_link_libraries(${PROJECT_NAME} "-framework Cocoa")
    target_link_libraries(${PROJECT_NAME} "-framework OpenGL")
endif()
