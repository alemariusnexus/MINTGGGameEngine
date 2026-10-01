macro(MINTGGGameEngine_ProjectPrologue)
    if(ESP_PLATFORM OR DEFINED IDF_TARGET)
        include($ENV{IDF_PATH}/tools/cmake/project.cmake)

        set(MINTGGGAMEENGINE_PLATFORM "ESPIDF")
    else()
        set(MINTGGGAMEENGINE_PLATFORM "DESKTOP")
    endif()
endmacro()

macro(MINTGGGameEngine_ComponentPrologue)
    if(NOT DEFINED MINTGGGAMEENGINE_PLATFORM)
        if(ESP_PLATFORM OR DEFINED IDF_TARGET)
            set(MINTGGGAMEENGINE_PLATFORM "ESPIDF")
        else()
            set(MINTGGGAMEENGINE_PLATFORM "DESKTOP")
        endif()
    endif()
endmacro()

macro(MINTGGGameEngine_SetupProject)
    if(NOT MINTGGGAMEENGINE_PLATFORM STREQUAL ESPIDF)
        add_subdirectory(components/MINTGGGameEngine)
        add_subdirectory(main)
    endif()
endmacro()

macro(MINTGGGameEngine_SetupApplication)
    if(MINTGGGAMEENGINE_PLATFORM STREQUAL ESPIDF)
        idf_component_register(
            SRCS ${SRCS}
            REQUIRES
                MINTGGGameEngine
        )

        target_compile_options(${COMPONENT_LIB} PRIVATE
                -Wno-missing-field-initializers
        )

        spiffs_create_partition_image(spiffs ../spiffs FLASH_IN_PROJECT)
    elseif(MINTGGGAMEENGINE_PLATFORM STREQUAL DESKTOP)
        add_executable("${CMAKE_PROJECT_NAME}" ${SRCS})
        target_link_libraries("${CMAKE_PROJECT_NAME}" PRIVATE MINTGGGameEngine)
        target_include_directories("${CMAKE_PROJECT_NAME}" PUBLIC ../components/MINTGGGameEngine/src)
        target_compile_features("${CMAKE_PROJECT_NAME}" PUBLIC cxx_std_20)
    endif()
endmacro()
