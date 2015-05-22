set ( SOURCES 
    "src/interface.cpp"
    "src/image_proxy.cpp"
)

set (HEADERS
    "include/image_proxy.h"
	${HEADERS_SHARED}
)

include_directories("include")

add_library ( image_proxy MODULE ${SOURCES} ${HEADERS})
target_link_libraries(image_proxy PRIVATE lms imaging)
