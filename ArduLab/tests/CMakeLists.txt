set(CMAKE_AUTOMOC ON)

function(ardulab_test name src)
    add_executable(${name} ${src})
    target_link_libraries(${name} PRIVATE ardulab_domain Qt6::Test)
    target_include_directories(${name} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    add_test(NAME ${name} COMMAND ${name})
    # Headless: force the offscreen platform plugin for any Qt bits.
    set_tests_properties(${name} PROPERTIES
        ENVIRONMENT "QT_QPA_PLATFORM=offscreen")
endfunction()

ardulab_test(test_geometry TestGeometry.cpp)
ardulab_test(test_catalog  TestCatalog.cpp)
ardulab_test(test_import   TestImport.cpp)
ardulab_test(test_project  TestProject.cpp)

# GUI-level test (canvas + commands) needs Widgets; run under offscreen.
add_executable(test_canvas TestCanvas.cpp
    ${CMAKE_SOURCE_DIR}/src/canvas/CanvasScene.cpp
    ${CMAKE_SOURCE_DIR}/src/canvas/CanvasView.cpp
    ${CMAKE_SOURCE_DIR}/src/canvas/ComponentItem.cpp
    ${CMAKE_SOURCE_DIR}/src/commands/Commands.cpp)
target_link_libraries(test_canvas PRIVATE ardulab_domain Qt6::Widgets Qt6::Test)
target_include_directories(test_canvas PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
add_test(NAME test_canvas COMMAND test_canvas)
set_tests_properties(test_canvas PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")
