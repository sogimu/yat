set(INCLUDE_DIRS ${INCLUDE_DIRS} ${CMAKE_CURRENT_LIST_DIR}/include)

set(HEADERS
    ${HEADERS}
    ${CMAKE_CURRENT_LIST_DIR}/include/timer.hpp
)

set(SOURCES
    ${SOURCES}
    ${CMAKE_CURRENT_LIST_DIR}/src/timer.cpp
)
