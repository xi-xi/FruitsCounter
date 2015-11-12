cmake_minimum_required(VERSION 3.3)
project(FruitsCounter)

set(OpenCV_DIR "Where" CACHE PATH "Location of opencv_install_dir")
set(BOOST_ROOT "Where" CACHE PATH "Location of boost install dir")
set(BOOST_LIBRARYDIR "Where" CACHE PATH "Location of boost lib dir")
set(DLIB_ROOT "where" CACHE PATH "Location of dlib root dir")
set(Boost_USE_STATIC_LIBS ON)

find_package(OpenCV REQUIRED)
if(OpenCV_FOUND)
    message(STATUS "OpenCV_INCLUDE_DIRS:${OpenCV_INCLUDE_DIRS}")
    message(STATUS "OpenCV_LIB_DIR:${OpenCV_LIB_DIR}")
    message(STATUS "OpenCV_LIBS:${OpenCV_LIBS}")
    include_directories(${OpenCV_INCLUDE_DIRS})
    link_directories(${OpenCV_LIB_DIR})
endif()

find_package(Boost REQUIRED COMPONENTS filesystem system date_time regex thread chrono program_options)
if(Boost_FOUND)
    message(STATUS "Boost_INCLUDE_DIRS:${Boost_INCLUDE_DIRS}")
    message(STATUS "Boost_LIBRARY_DIRS:${Boost_LIBRARY_DIRS}")
    message(STATUS "Boost_LIBRARIES:${Boost_LIBRARIES}")
    include_directories(${Boost_INCLUDE_DIRS})
    link_directories(${Boost_LIBRARY_DIRS})
endif()

include(${DLIB_ROOT}/dlib/cmake)

# panorama
add_executable(panorama panorama.cpp)
target_link_libraries(panorama ${OpenCV_LIBS})
target_link_libraries(panorama ${Boost_LIBRARIES})

# selective_search
add_executable(selective_search selective_search.cpp)
target_link_libraries(selective_search ${OpenCV_LIBS})
target_link_libraries(selective_search ${Boost_LIBRARIES})
target_link_libraries(selective_search dlib)

# training
add_executable(train train.cpp HogUtil.hpp)
target_link_libraries(train ${OpenCV_LIBS})
target_link_libraries(train ${Boost_LIBRARIES})

# main interface...?
add_executable(main main.cpp TimeLapse.cpp TimeLapse.hpp)
target_link_libraries(main ${OpenCV_LIBS})
target_link_libraries(main ${Boost_LIBRARIES})
target_link_libraries(main dlib)
