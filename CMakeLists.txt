cmake_minimum_required (VERSION 2.8)
project (hdf5++)
set (mapiSim_VERSION_MAJOR 1)
set (mapiSim_VERSION_MINOR 0)

include(CheckCXXSourceCompiles)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/CMakeModules/")
add_subdirectory (CMakeModules)

## find necessary libraries
find_package(Boost REQUIRED)
set(LDFLAGS ${LDFLAGS} ${Boost_LDFLAGS})
set(CXXFLAGS ${CXXFLAGS} ${Boost_CPPFLAGS})
include_directories (${ROOT_INCLUDE_DIR})

find_package(HDF5 REQUIRED)
message(STATUS "HDF5_LIBRARY_DIRS: ${HDF5_LIBRARY_DIRS}") 
message(STATUS "HDF5_INCLUDE_DIRS: ${HDF5_INCLUDE_DIRS}")
link_directories(${HDF5_LIBRARY_DIRS})
include_directories(${HDF5_INCLUDE_DIRS})

## removing trailing whitespaces
foreach (a ${LDFLAGS})
	set(b "${b} ${a}")
endforeach(a)
set(LDFLAGS ${b})
string(REGEX REPLACE "^[ \t]+|[ \t]+$" "" LDFLAGS "${LDFLAGS}")


#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O3 -march=native -mtune=native --fast-math -Wall -std=c++0x -Wno-deprecated")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ggdb -Wall -std=c++0x -Wno-deprecated")

## define libraries and programs
set(LIBRARIES boost_regex boost_program_options xerces-c png X11 ${PCL_COMMON_LIBRARIES} ${PCL_IO_LIBRARIES} ${PCL_VISUALIZATION_LIBRARIES} hdf5_cpp)

## define subdirectories
set(LDFLAGS "${LDFLAGS}")

# build modules for subdirectories
add_executable(hdfTest hdfTest.cpp)
target_link_libraries(hdfTest ${LIBRARIES} "${LDFLAGS}")
