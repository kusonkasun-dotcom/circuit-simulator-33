cmake_minimum_required(VERSION 3.21)
project(ArduLab VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Sql Test)

# ---------------------------------------------------------------------------
# Domain library (no Widgets dependency) : core, component, catalog, import,
# project. This is what the headless tests link against.
# ---------------------------------------------------------------------------
add_library(ardulab_domain STATIC
    src/core/Logger.cpp
    src/core/EventBus.cpp
    src/component/Geometry.cpp
    src/component/ComponentDefinition.cpp
    src/component/ComponentInstance.cpp
    src/catalog/Database.cpp
    src/catalog/Migrations.cpp
    src/catalog/CatalogRepository.cpp
    src/import/ComponentImporter.cpp
    src/project/Project.cpp
    src/project/ProjectSerializer.cpp
)
target_include_directories(ardulab_domain PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(ardulab_domain PUBLIC Qt6::Core Qt6::Sql)

# ---------------------------------------------------------------------------
# GUI application
# ---------------------------------------------------------------------------
add_executable(ardulab
    src/main.cpp
    src/canvas/ComponentItem.cpp
    src/canvas/CanvasScene.cpp
    src/canvas/CanvasView.cpp
    src/commands/Commands.cpp
    src/ui/ComponentBrowser.cpp
    src/ui/PropertyPanel.cpp
    src/ui/ImportReportDialog.cpp
    src/ui/MainWindow.cpp
    examples/examples.qrc
)
target_link_libraries(ardulab PRIVATE ardulab_domain Qt6::Widgets Qt6::Gui)

# ---------------------------------------------------------------------------
# Tests
# ---------------------------------------------------------------------------
enable_testing()
add_subdirectory(tests)
