set(SEMANTIC_CPP_TEST_TARGET semantic-cpp-tests)

add_executable(
    ${SEMANTIC_CPP_TEST_TARGET}
    test_main.cpp
    test_bunch.cpp
    test_localisation.cpp
    test_unicode.cpp
    test_semantic.cpp
    test_containers.cpp
    test_tree.cpp
    test_ast.cpp
    test_temporal.cpp
    test_mathematics.cpp
)

target_link_libraries(${SEMANTIC_CPP_TEST_TARGET} PRIVATE semantic-cpp::semantic-cpp)
target_compile_features(${SEMANTIC_CPP_TEST_TARGET} PRIVATE cxx_std_17)
set_target_properties(
    ${SEMANTIC_CPP_TEST_TARGET}
    PROPERTIES
        CXX_EXTENSIONS OFF
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
)

if(MSVC)
    target_compile_options(${SEMANTIC_CPP_TEST_TARGET} PRIVATE /W4 /permissive- /Zc:__cplusplus)
else()
    target_compile_options(${SEMANTIC_CPP_TEST_TARGET} PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(NAME semantic-cpp-tests COMMAND ${SEMANTIC_CPP_TEST_TARGET})
set_tests_properties(semantic-cpp-tests PROPERTIES TIMEOUT 120)

# Compile every public header on its own. This catches headers that rely on
# transitive includes from another header.
foreach(header IN LISTS SEMANTIC_CPP_HEADERS)
    string(REPLACE "/" "-" headerId "${header}")
    set(headerSource "${CMAKE_CURRENT_BINARY_DIR}/header_${headerId}.cpp")
    file(WRITE "${headerSource}" "#include \"${header}\"\n\nint main()\n{\n    return 0;\n}\n")

    set(headerTarget "semantic-cpp-header-${headerId}")
    add_executable(${headerTarget} "${headerSource}")
    target_link_libraries(${headerTarget} PRIVATE semantic-cpp::semantic-cpp)
    target_compile_features(${headerTarget} PRIVATE cxx_std_17)
    set_target_properties(
        ${headerTarget}
        PROPERTIES
            CXX_EXTENSIONS OFF
            CXX_STANDARD 17
            CXX_STANDARD_REQUIRED ON
    )
    if(MSVC)
        target_compile_options(${headerTarget} PRIVATE /W4 /permissive- /Zc:__cplusplus)
    else()
        target_compile_options(${headerTarget} PRIVATE -Wall -Wextra -Wpedantic)
    endif()

    add_test(NAME header-${headerId} COMMAND ${headerTarget})
    set_tests_properties(header-${headerId} PROPERTIES TIMEOUT 60)
endforeach()
