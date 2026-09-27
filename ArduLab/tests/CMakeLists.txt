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
