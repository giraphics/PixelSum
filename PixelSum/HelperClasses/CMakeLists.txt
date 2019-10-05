CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
SET(PROJECTNAME PixelSum)
PROJECT(${PROJECTNAME})


list(APPEND CMAKE_MODULE_PATH)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH})
file(GLOB source *.cpp)
file(GLOB header *.h)

source_group("Source Files" FILES ${source})
source_group("Header Files" FILES ${header})

add_executable(${PROJECTNAME} ${source} ${header})
install(TARGETS ${PROJECTNAME} RUNTIME DESTINATION bin)