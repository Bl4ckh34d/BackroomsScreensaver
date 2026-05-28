cmake_minimum_required(VERSION 3.20)

project(BackroomsMazeScreensaver LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

set(BACKROOMS_SOURCES
    src/main.cpp
    src/app.rc
)

add_executable(BackroomsMaze WIN32 ${BACKROOMS_SOURCES})

target_compile_definitions(BackroomsMaze PRIVATE
    UNICODE
    _UNICODE
    WIN32_LEAN_AND_MEAN
    NOMINMAX
)

target_link_libraries(BackroomsMaze PRIVATE
    d3d11
    dxgi
    d3dcompiler
    xaudio2
    windowscodecs
    ole32
    comctl32
    user32
    gdi32
    shell32
)

set_target_properties(BackroomsMaze PROPERTIES
    OUTPUT_NAME "BackroomsMaze"
    SUFFIX ".scr"
)

add_executable(BackroomsMazeGame WIN32 ${BACKROOMS_SOURCES})
add_dependencies(BackroomsMazeGame BackroomsMaze)

target_compile_definitions(BackroomsMazeGame PRIVATE
    UNICODE
    _UNICODE
    WIN32_LEAN_AND_MEAN
    NOMINMAX
    BACKROOMS_GAME_EXE
)

target_link_libraries(BackroomsMazeGame PRIVATE
    d3d11
    dxgi
    d3dcompiler
    xaudio2
    windowscodecs
    ole32
    comctl32
    user32
    gdi32
    shell32
)

set_target_properties(BackroomsMazeGame PROPERTIES
    OUTPUT_NAME "BackroomsMazeGame"
    SUFFIX ".exe"
)

foreach(target BackroomsMaze BackroomsMazeGame)
    add_custom_command(TARGET ${target} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:${target}>/assets/branding"
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_SOURCE_DIR}/assets/branding"
            "$<TARGET_FILE_DIR:${target}>/assets/branding"
        COMMAND ${CMAKE_COMMAND} -E rm -rf "$<TARGET_FILE_DIR:${target}>/assets/sounds"
        COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:${target}>/assets/sounds"
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_SOURCE_DIR}/assets/sounds"
            "$<TARGET_FILE_DIR:${target}>/assets/sounds"
        COMMENT "Copying branding assets for ${target}"
    )
endforeach()

foreach(helper Config PreviewTest Profile BloodDebug Test)
    add_custom_command(TARGET BackroomsMaze POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
            "$<TARGET_FILE:BackroomsMaze>"
            "$<TARGET_FILE_DIR:BackroomsMaze>/BackroomsMaze${helper}.exe"
        COMMENT "Refreshing BackroomsMaze${helper}.exe helper"
    )
endforeach()

set(BACKROOMS_DEFAULT_INI "")
foreach(candidate
    "${CMAKE_SOURCE_DIR}/BackroomsMaze_backup.ini"
    "${CMAKE_BINARY_DIR}/BackroomsMaze_backup.ini"
    "${CMAKE_SOURCE_DIR}/build/BackroomsMaze_backup.ini"
)
    if(EXISTS "${candidate}")
        set(BACKROOMS_DEFAULT_INI "${candidate}")
        break()
    endif()
endforeach()

find_program(BACKROOMS_POWERSHELL NAMES powershell pwsh)

if(BACKROOMS_DEFAULT_INI AND BACKROOMS_POWERSHELL)
    add_custom_command(TARGET BackroomsMaze POST_BUILD
        COMMAND "${BACKROOMS_POWERSHELL}" -NoProfile -ExecutionPolicy Bypass
            -File "${CMAKE_SOURCE_DIR}/tools/merge-ini-defaults.ps1"
            -ScrPath "$<TARGET_FILE:BackroomsMaze>"
            -PreferredIni "${BACKROOMS_DEFAULT_INI}"
            -OutputIni "$<TARGET_FILE_DIR:BackroomsMaze>/BackroomsMaze.ini"
        COMMENT "Merging preferred BackroomsMaze.ini defaults"
    )
elseif(BACKROOMS_DEFAULT_INI)
    add_custom_command(TARGET BackroomsMaze POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${BACKROOMS_DEFAULT_INI}"
            "$<TARGET_FILE_DIR:BackroomsMaze>/BackroomsMaze.ini"
        COMMENT "Copying preferred BackroomsMaze.ini defaults"
    )
endif()

if(BACKROOMS_DEFAULT_INI AND BACKROOMS_POWERSHELL)
    add_custom_command(TARGET BackroomsMazeGame POST_BUILD
        COMMAND "${BACKROOMS_POWERSHELL}" -NoProfile -ExecutionPolicy Bypass
            -File "${CMAKE_SOURCE_DIR}/tools/merge-ini-defaults.ps1"
            -ScrPath "$<TARGET_FILE:BackroomsMaze>"
            -PreferredIni "${BACKROOMS_DEFAULT_INI}"
            -OutputIni "$<TARGET_FILE_DIR:BackroomsMazeGame>/BackroomsMaze.ini"
        COMMENT "Merging preferred BackroomsMaze.ini defaults for game"
    )
elseif(BACKROOMS_DEFAULT_INI)
    add_custom_command(TARGET BackroomsMazeGame POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${BACKROOMS_DEFAULT_INI}"
            "$<TARGET_FILE_DIR:BackroomsMazeGame>/BackroomsMaze.ini"
        COMMENT "Copying preferred BackroomsMaze.ini defaults for game"
    )
endif()
