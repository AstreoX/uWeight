cmake_minimum_required(VERSION 3.20)
project(uWidget VERSION 1.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# 设置Qt6路径（如果CMake找不到Qt6）
if(NOT DEFINED CMAKE_PREFIX_PATH OR CMAKE_PREFIX_PATH STREQUAL "")
    if(WIN32)
        set(CMAKE_PREFIX_PATH "C:/Qt/6.9.0/mingw_64" CACHE PATH "Qt installation path")
    endif()
endif()

# 查找Qt6
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Network)

# 查找其他依赖
find_package(Threads REQUIRED)

# 包含目录
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src
)

# 源文件
set(SOURCES
    src/main.cpp
    src/Framework/WidgetFramework.cpp
    src/Framework/WidgetManager.cpp
    src/Core/BaseWidget.cpp
    src/Core/WidgetRenderer.cpp
    src/Core/InteractionSystem.cpp
    src/BackendManagement/ManagementWindow.cpp
    src/BackendManagement/ConfigWindow.cpp
    src/BackendManagement/CreateWidgetDialog.cpp
    src/BackendManagement/ThemeSettingsDialog.cpp
    src/BackendManagement/AIRankingConfigDialog.cpp
    src/BackendManagement/WeatherConfigDialog.cpp
    src/Utils/SystemTray.cpp
    src/Utils/Logger.cpp
    src/Utils/ThemeManager.cpp
    src/Utils/ThemeResourceManager.cpp
    src/Testing/TestInterface.cpp
    src/Widgets/ClockWidget.cpp
    src/Widgets/WeatherWidget.cpp
    src/Widgets/AIRankingWidget.cpp
    src/Widgets/SystemPerformanceWidget.cpp
    src/Widgets/SimpleNotesWidget.cpp
    src/Widgets/CalendarWidget.cpp
    src/Widgets/SystemInfoWidget.cpp
    src/Utils/SystemInfoCollector.cpp
)

# Windows特定资源文件
if(WIN32)
    list(APPEND SOURCES app_icon.rc)
endif()

# 头文件
set(HEADERS
    include/Framework/WidgetFramework.h
    include/Framework/WidgetManager.h
    include/Core/BaseWidget.h
    include/Core/WidgetRenderer.h
    include/Core/InteractionSystem.h
    include/BackendManagement/ManagementWindow.h
    include/BackendManagement/ConfigWindow.h
    include/BackendManagement/CreateWidgetDialog.h
    include/BackendManagement/ThemeSettingsDialog.h
    include/BackendManagement/AIRankingConfigDialog.h
    include/BackendManagement/WeatherConfigDialog.h
    include/Utils/SystemTray.h
    include/Utils/ThemeManager.h
    include/Utils/ThemeResourceManager.h
    include/Testing/TestInterface.h
    include/Widgets/ClockWidget.h
    include/Widgets/WeatherWidget.h
    include/Widgets/AIRankingWidget.h
    include/Widgets/SystemPerformanceWidget.h
    include/Widgets/SimpleNotesWidget.h
    include/Widgets/CalendarWidget.h
    include/Widgets/SystemInfoWidget.h
    include/Utils/SystemInfoCollector.h
)

# Qt MOC处理
qt6_wrap_cpp(MOC_SOURCES ${HEADERS})

# Qt Resource处理
set(RESOURCES
    icons.qrc
)
qt6_add_resources(RESOURCE_FILES ${RESOURCES})

# 创建可执行文件
add_executable(uWidget ${SOURCES} ${MOC_SOURCES} ${RESOURCE_FILES})

# 设置Qt6自动MOC、UIC、RCC
set_target_properties(uWidget PROPERTIES
    AUTOMOC ON
    AUTOUIC ON
    AUTORCC ON
)

# 链接库
target_link_libraries(uWidget PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Network
    Threads::Threads
)

# Windows特定设置
if(WIN32)
    target_link_libraries(uWidget PRIVATE
        user32
        shell32
        ole32
        dwmapi
        pdh
        iphlpapi
        psapi
    )
    
    # 添加Windows特定的编译定义
    target_compile_definitions(uWidget PRIVATE
        WIN32
        _WINDOWS
        UNICODE
        _UNICODE
        WINVER=0x0601
        _WIN32_WINNT=0x0601
    )
    
    set_target_properties(uWidget PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
    
    # 修复MinGW链接问题
    if(MINGW)
        target_link_libraries(uWidget PRIVATE
            -static-libgcc
            -static-libstdc++
        )
        
        # MinGW特定的编译选项
        target_compile_options(uWidget PRIVATE
            -Wno-unknown-pragmas  # 禁用未知pragma警告
            -Wno-parentheses     # 禁用括号警告
        )
    endif()
endif()

# 编译器特定设置
if(MSVC)
    target_compile_options(uWidget PRIVATE /W4)
else()
    target_compile_options(uWidget PRIVATE -Wall -Wextra -pedantic)
endif() 