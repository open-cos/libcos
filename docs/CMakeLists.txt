find_package(Doxygen)
if (DOXYGEN_FOUND)
    doxygen_add_docs(libcos-docs
        ${CMAKE_CURRENT_SOURCE_DIR}/../include/libcos
        COMMENT "Generating API documentation with Doxygen"
    )
endif (DOXYGEN_FOUND)
