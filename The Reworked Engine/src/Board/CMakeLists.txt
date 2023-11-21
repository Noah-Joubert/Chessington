project(Board)

set(HEADER_FILES
       "../types.h"
        )

set(SOURCE_FILES
        board.cpp
        )

add_library(Board STATIC ${SOURCE_FILES} ${HEADER_FILES})