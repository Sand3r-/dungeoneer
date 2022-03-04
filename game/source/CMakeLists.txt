target_sources(dungeoneer PRIVATE
    win32/win32_main.c
)

list(APPEND INCLUDES .)
list(APPEND INCLUDES ../../common)
list(APPEND INCLUDES ../../data_desk)

target_include_directories(dungeoneer PRIVATE ${INCLUDES})
target_link_libraries(dungeoneer ${OPENGL_LIBRARIES} winmm.lib)
