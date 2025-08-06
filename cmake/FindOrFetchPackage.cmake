# 使用 /<library>/<platform>/<architecture>/<config> 的预构建库目录结构
# 例如: 3rdparty/prebuilt/spdlog/windows/x86_64/debug
function(get_library_store_path LIB_NAME RETURN_VAR)
    # 1. 确定平台名称
    if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
        set(platform "windows")
    elseif (CMAKE_SYSTEM_NAME STREQUAL "Linux")
        set(platform "linux")
    elseif (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
        set(platform "macos")
    else ()
        string(TOLOWER "${CMAKE_SYSTEM_NAME}" platform)
    endif ()

    # 2. 确定架构名称
    if (CMAKE_SYSTEM_PROCESSOR MATCHES "AMD64|x86_64")
        set(arch "x86_64")
    elseif (CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64|arm64")
        set(arch "arm64")
    elseif (CMAKE_SYSTEM_PROCESSOR MATCHES "x86|i[3-6]86")
        set(arch "x86")
    else ()
        string(TOLOWER "${CMAKE_SYSTEM_PROCESSOR}" arch)
    endif ()

    # 3. 确定构建配置
    set(config "debug")
    if (CMAKE_BUILD_TYPE)
        string(TOLOWER "${CMAKE_BUILD_TYPE}" config)
    endif ()

    # 4. 组合成最终路径，CMAKE_SOURCE_DIR 是项目的根目录
    # 推荐预编译库目录：3rdparty/prebuilt/spdlog
    # 推荐源码库目录：  3rdparty/sources/googletest
    set(result_path "${CMAKE_SOURCE_DIR}/3rdparty/${LIB_NAME}/${platform}/${arch}/${config}")

    # 5. 通过指定的返回变量名，将结果返回给调用者
    # set(variable_name value PARENT_SCOPE) 是在CMake函数中返回值给上一层的标准方法
    set(${RETURN_VAR} "${result_path}" PARENT_SCOPE)
endfunction()

# 用法示例：
# find_or_fetch_package(
#        NAME spdlog
#        GIT_REPOSITORY https://github.com/gabime/spdlog.git
#        GIT_TAG v1.10.0
#        CONFIGURE_ARGS
#            -DSPDLOG_BUILD_SHARED=OFF
#            -DSPDLOG_INSTALL=ON
#)
function(find_or_fetch_package)
    cmake_parse_arguments(
            LIB  # prefix for output variables
            ""    # list of names of the boolean arguments (options)
            "NAME;GIT_REPOSITORY;GIT_TAG;URL" # list of names of the single-value arguments (one_value_args)
            "CONFIGURE_ARGS" # list of names of the multi-value arguments (multi_value_args)
            ${ARGN} # arguments of the function
    )

    # For debugging purposes, print the parsed arguments
    message(STATUS "find_or_fetch_package called with:")
    message(STATUS "  NAME: ${LIB_NAME}")
    if (LIB_GIT_REPOSITORY)
        message(STATUS "  GIT_REPOSITORY: ${LIB_GIT_REPOSITORY}")
    endif ()
    if (LIB_GIT_TAG)
        message(STATUS "  GIT_TAG: ${LIB_GIT_TAG}")
    endif ()
    if (LIB_URL)
        message(STATUS "  URL: ${LIB_URL}")
    endif ()
    if (LIB_CONFIGURE_ARGS)
        message(STATUS "  CONFIGURE_ARGS: ${LIB_CONFIGURE_ARGS}")
    endif ()

    if (NOT LIB_NAME)
        message(FATAL_ERROR "Error: NAME is required.")
    endif ()

    # Validation: LIB_URL and LIB_GIT_REPOSITORY/LIB_GIT_TAG cannot be simultaneously defined
    if (DEFINED LIB_URL AND (DEFINED LIB_GIT_REPOSITORY OR DEFINED LIB_GIT_TAG))
        message(FATAL_ERROR "Error: Cannot specify both URL and GIT_REPOSITORY/GIT_TAG.")
    endif ()

    # Define FetchContent_ARGS based on provided parameters
    set(FetchContent_ARGS)
    if (DEFINED LIB_URL)
        list(APPEND FetchContent_ARGS URL ${LIB_URL})
        list(APPEND FetchContent_ARGS DOWNLOAD_EXTRACT_TIMESTAMP TRUE)
    elseif (DEFINED LIB_GIT_REPOSITORY)
        list(APPEND FetchContent_ARGS GIT_REPOSITORY ${LIB_GIT_REPOSITORY})
        if (DEFINED LIB_GIT_TAG)
            list(APPEND FetchContent_ARGS GIT_TAG ${LIB_GIT_TAG})
        endif ()
        list(APPEND FetchContent_ARGS GIT_SHALLOW TRUE)
        list(APPEND FetchContent_ARGS GIT_PROGRESS TRUE)
    endif ()

    _find_or_fetch_package_impl(
            "${LIB_NAME}"
            "${FetchContent_ARGS}"
            "${LIB_CONFIGURE_ARGS}"
    )
endfunction()

function(_find_or_fetch_package_impl _NAME _FETCH_CONTENT_ARGS _CONFIGURE_ARGS)
    set(LIB_NAME ${_NAME})
    set(PRJ_NAME "${LIB_NAME}_project")

    # 1. 获取期望的预编译路径，并尝试查找已编译的库
    get_library_store_path(${LIB_NAME} PREBUILT_PATH)
    find_package(${LIB_NAME} CONFIG HINTS "${PREBUILT_PATH}")
    if (${LIB_NAME}_FOUND OR TARGET "${LIB_NAME}::${LIB_NAME}")
        message(STATUS "Found prebuilt ${LIB_NAME} at: ${PREBUILT_PATH}")
        return()
    endif ()

    # 2. 如果没找到，进入自动构建和安装流程
    FetchContent_Populate(
            "${PRJ_NAME}"
            "${_FETCH_CONTENT_ARGS}"
    )
    message(STATUS "${PRJ_NAME}_SOURCE_DIR: ${${PRJ_NAME}_SOURCE_DIR}")
    message(STATUS "${PRJ_NAME}_BINARY_DIR: ${${PRJ_NAME}_BINARY_DIR}")

    set(LIB_CONFIGURE_ARGS
            # 安装和构建类型
            -DCMAKE_INSTALL_PREFIX=${PREBUILT_PATH}
            -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
            # 传递编译器信息，避免不匹配
            # -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
            # -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
            # 传递编译标志
            # -DCMAKE_C_FLAGS=${CMAKE_C_FLAGS}
            # -DCMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS}
            # 传递交叉编译工具链文件 (如果存在)
            # -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
            # 传递生成器信息 (非常重要！)
            -G ${CMAKE_GENERATOR}
            # -A ${CMAKE_GENERATOR_PLATFORM} # For VS
            # -T ${CMAKE_GENERATOR_TOOLSET}  # For VS
            # spdlog 特定的选项
            -DCMAKE_POSITION_INDEPENDENT_CODE=ON
            ${_CONFIGURE_ARGS}
    )

    # 3. 手动编译项目
    MESSAGE(STATUS "Configuring ${LIB_NAME}...")
    EXECUTE_PROCESS(
            COMMAND ${CMAKE_COMMAND} -S ${${PRJ_NAME}_SOURCE_DIR} -B ${${PRJ_NAME}_BINARY_DIR} ${LIB_CONFIGURE_ARGS}
            OUTPUT_QUIET COMMAND_ERROR_IS_FATAL ANY
    )
    MESSAGE(STATUS "Building ${LIB_NAME}...")
    EXECUTE_PROCESS(
            COMMAND ${CMAKE_COMMAND} --build ${${PRJ_NAME}_BINARY_DIR} --config ${CMAKE_BUILD_TYPE}
            OUTPUT_QUIET COMMAND_ERROR_IS_FATAL ANY
    )
    MESSAGE(STATUS "Installing ${LIB_NAME}...")
    EXECUTE_PROCESS(
            COMMAND ${CMAKE_COMMAND} --install ${${PRJ_NAME}_BINARY_DIR} --config ${CMAKE_BUILD_TYPE}
            OUTPUT_QUIET COMMAND_ERROR_IS_FATAL ANY
    )
    MESSAGE(STATUS "Cleaning ${LIB_NAME}...")
    EXECUTE_PROCESS(
            COMMAND ${CMAKE_COMMAND} -E remove_directory ${${PRJ_NAME}_BINARY_DIR}
            OUTPUT_QUIET COMMAND_ERROR_IS_FATAL ANY
    )

    # 4. 重新查找配置库
    find_package(${LIB_NAME} CONFIG HINTS "${PREBUILT_PATH}")
    if (${LIB_NAME}_FOUND OR TARGET "${LIB_NAME}::${LIB_NAME}")
        message(STATUS "Prebuilt ${LIB_NAME} successfully built and installed.")
    else ()
        message(FATAL_ERROR "Failed to build and install ${LIB_NAME}.")
    endif ()
endfunction()