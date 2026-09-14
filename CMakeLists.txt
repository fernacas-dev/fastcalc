cmake_minimum_required(VERSION 3.22)
project(fastcalc LANGUAGES C)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL3 REQUIRED sdl3)
pkg_check_modules(FREETYPE REQUIRED freetype2)

option(SANITIZE "Enable AddressSanitizer + UndefinedBehaviorSanitizer (development)" OFF)

set(FASTCALC_WARNINGS
    -Wall
    -Wextra
    -Wpedantic
    -Wconversion
    -Wshadow
)

set(FASTCALC_SOURCES
    src/main.c
    src/app.c
    src/calculator.c
    src/renderer.c
    src/input.c
    src/font.c
    src/glyph_cache.c
    src/layout.c
    src/clipboard.c
    src/theme.c
)

add_executable(fastcalc ${FASTCALC_SOURCES})
target_include_directories(fastcalc PRIVATE
    src
    ${SDL3_INCLUDE_DIRS}
    ${FREETYPE_INCLUDE_DIRS}
)
target_compile_options(fastcalc PRIVATE
    ${FASTCALC_WARNINGS}
    ${SDL3_CFLAGS_OTHER}
    ${FREETYPE_CFLAGS_OTHER}
)
target_link_libraries(fastcalc PRIVATE
    ${SDL3_LIBRARIES}
    ${FREETYPE_LIBRARIES}
    m
)

# Calculator engine unit tests (no SDL dependency).
add_executable(calculator_tests
    tests/calculator_tests.c
    src/calculator.c
)
target_include_directories(calculator_tests PRIVATE src)
target_compile_options(calculator_tests PRIVATE ${FASTCALC_WARNINGS})
target_link_libraries(calculator_tests PRIVATE m)

if(SANITIZE)
    foreach(tgt fastcalc calculator_tests)
        target_compile_options(${tgt} PRIVATE -fsanitize=address,undefined -fno-omit-frame-pointer)
        target_link_options(${tgt} PRIVATE -fsanitize=address,undefined)
    endforeach()
endif()

enable_testing()
add_test(NAME calculator_tests COMMAND calculator_tests)

# System installation: binary + desktop entry + icon.
#   cmake --install build --prefix /usr/local
include(GNUInstallDirs)
install(TARGETS fastcalc RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES assets/fastcalc.desktop
        DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)
install(FILES assets/fastcalc.svg
        DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps)
