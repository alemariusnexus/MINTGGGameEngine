cmake_path(GET CMAKE_CURRENT_LIST_DIR PARENT_PATH MGG_ENGINE_ROOT)
set(MGG_APP_ROOT "${CMAKE_SOURCE_DIR}")

macro(MGG_ProjectPrologue)
    if(ESP_PLATFORM OR DEFINED IDF_TARGET)
        # Exclude components that only work for specific chip families from build of other families
        if(NOT IDF_TARGET STREQUAL "esp32p4")
            #set(EXCLUDE_COMPONENTS esp_lcd_rpi_dsi)
        endif()

        set(MGG_ENGINE_PARTITIONS_FILE_TARGET "${MGG_ENGINE_ROOT}/espidf/partitions_${IDF_TARGET}.csv")
        set(MGG_ENGINE_PARTITIONS_FILE_GENERAL "${MGG_ENGINE_ROOT}/espidf/partitions.csv")
        if(EXISTS "${MGG_ENGINE_PARTITIONS_FILE_TARGET}")
            set(MGG_ENGINE_PARTITIONS_FILE "${MGG_ENGINE_PARTITIONS_FILE_TARGET}")
        else()
            set(MGG_ENGINE_PARTITIONS_FILE "${MGG_ENGINE_PARTITIONS_FILE_GENERAL}")
        endif()

        set(MGG_ENGINE_SDKCONFIG_DEFAULTS "${MGG_ENGINE_ROOT}/espidf/sdkconfig.defaults")
        set(MGG_ENGINE_GEN_SDKCONFIG_DEFAULTS "${CMAKE_BINARY_DIR}/mgg-gen-sdkconfig.defaults")

        # Generate sdkconfig.defaults file for the partition tables. Users can provide their own partition table by
        # overriding these config values in their own sdkconfig.defaults file.
        file(WRITE "${MGG_ENGINE_GEN_SDKCONFIG_DEFAULTS}"
            "CONFIG_PARTITION_TABLE_CUSTOM=y\n"
            "CONFIG_PARTITION_TABLE_CUSTOM_FILENAME=\"${MGG_ENGINE_PARTITIONS_FILE}\"\n"
            )

        # Here's the order in which sdkconfig settings are applied (in each step, sdkconfig.defaults is first applied,
        # then an target-specific files like sdkconfig.defaults.esp32p4):
        #   1. The engine specified defaults in espidf/partitions_*.csv, for general defaults.
        #   2. The generated file containing the partition table file names.
        #   3. The user-defined sdkconfig.defaults in their project directory (if it exists)
        set(SDKCONFIG_DEFAULTS
            "${MGG_ENGINE_SDKCONFIG_DEFAULTS}"
            "${MGG_ENGINE_GEN_SDKCONFIG_DEFAULTS}"
            )
        if(EXISTS "${MGG_APP_ROOT}/sdkconfig.defaults")
            list(APPEND SDKCONFIG_DEFAULTS "${MGG_APP_ROOT}/sdkconfig.defaults")
        endif()

        set(SDKCONFIG "${CMAKE_BINARY_DIR}/sdkconfig")

        include($ENV{IDF_PATH}/tools/cmake/project.cmake)

        set(MGG_PLATFORM "ESPIDF")
    else()
        set(MGG_PLATFORM "DESKTOP")
    endif()
endmacro()


macro(MGG_ComponentPrologue)
    if(NOT DEFINED MGG_PLATFORM)
        if(ESP_PLATFORM OR DEFINED IDF_TARGET)
            set(MGG_PLATFORM "ESPIDF")
        else()
            set(MGG_PLATFORM "DESKTOP")
        endif()
    endif()
endmacro()


macro(MGG_SetupProject)
    if(NOT MGG_PLATFORM STREQUAL ESPIDF)
        add_subdirectory(components/MINTGGGameEngine)
        add_subdirectory(main)
    endif()
endmacro()


macro(MGG_SetupApplication)
    if(MGG_PLATFORM STREQUAL ESPIDF)
        idf_component_register(
            SRCS
                ${SRCS}
            INCLUDE_DIRS
                . # Project main directory. Necessary for the engine to find mgg_config.h
            REQUIRES
                MINTGGGameEngine
        )

        target_compile_options(${COMPONENT_LIB} PRIVATE
                -Wno-missing-field-initializers
        )
        target_compile_features(${COMPONENT_LIB} PUBLIC cxx_std_20)
        set(MGG_APP_MAIN_TARGET ${COMPONENT_LIB})

        spiffs_create_partition_image(spiffs ../data FLASH_IN_PROJECT)
    elseif(MGG_PLATFORM STREQUAL DESKTOP)
        add_executable("${CMAKE_PROJECT_NAME}" ${SRCS})
        target_link_libraries("${CMAKE_PROJECT_NAME}" PRIVATE MINTGGGameEngine)
        target_include_directories("${CMAKE_PROJECT_NAME}" PUBLIC ../components/MINTGGGameEngine/src)
        target_compile_features("${CMAKE_PROJECT_NAME}" PUBLIC cxx_std_20)

        set(MGG_APP_MAIN_TARGET "${CMAKE_PROJECT_NAME}")
    endif()
endmacro()
