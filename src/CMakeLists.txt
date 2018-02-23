# require pkg-config and threads
find_package( PkgConfig REQUIRED )
find_package( Threads REQUIRED )

# require libAV
pkg_check_modules( AV REQUIRED libswscale libavformat libavcodec libavutil libswresample )

# require openCV
set(OpenCV_STATIC ON)
find_package(OpenCV REQUIRED core nonfree)

# project libraries
add_library (KdTree ${CMAKE_SOURCE_DIR}/src/KdTree.cpp )

# main executable
add_executable(locateFrame2 
    ${CMAKE_SOURCE_DIR}/src/locateFrame2.cpp 
    ${CMAKE_SOURCE_DIR}/src/Arguments.cpp 
    ${CMAKE_SOURCE_DIR}/src/VideoDecoder.cpp 
    ${CMAKE_SOURCE_DIR}/src/VideoFrame.cpp 
    ${CMAKE_SOURCE_DIR}/src/WorkerQueue.cpp 
    ${CMAKE_SOURCE_DIR}/src/Worker.cpp 
    ${CMAKE_SOURCE_DIR}/src/SurfMatcher.cpp 
    ${CMAKE_SOURCE_DIR}/src/InputImage.cpp 
    ${CMAKE_SOURCE_DIR}/src/Match.cpp 
)

# project libraries
target_link_libraries(locateFrame2 KdTree)
# openCV
target_link_libraries(locateFrame2 ${OpenCV_LIBS})
# threads
target_link_libraries(locateFrame2 ${CMAKE_THREAD_LIBS_INIT})
# libAV
target_link_libraries(locateFrame2 ${AV_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
target_include_directories(locateFrame2 PUBLIC ${AV_INCLUDE_DIRS})
target_compile_options(locateFrame2 PUBLIC ${AV_CFLAGS_OTHER})