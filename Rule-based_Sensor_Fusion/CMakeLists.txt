cmake_minimum_required(VERSION 3.10)
set(CMAKE_VERBOSE_MAKEFILE ON)
project(FusionTracker)
#set(MODULE_NAME FusionTracker)

option(ENABLE_linux "Switch Linux" OFF)
option(ENABLE_CSV "Enable CSV write" ON)
option(ENABLE_visualizer "Enable the opencv" ON)
option(ENABLE_wrappers "Enable the wrappers" OFF)
option(ENABLE_offlineReadWrite "Enable offline mode" ON)

add_compile_definitions(Fusion_Bypass=0)
add_compile_definitions(Lidar_Bypass=0)
add_compile_definitions(Fcos_Tracker_Bypass=0)

if(ENABLE_CSV)
  add_compile_definitions(CSV_WRITING_FUSION=1)
  add_compile_definitions(CSV_WRITING_TRACKER=1)
endif()

find_package(Eigen3)
if(Eigen3_FOUND)
  message(STATUS " ---- Eigen3 directories: ${EIGEN3_INCLUDE_DIR}")
else()
  message(STATUS "*********** Eigen3 not found. Hard address have been set ***********")
  if(ENABLE_linux)
    find_package(PkgConfig REQUIRED)
#    set(EIGEN3_INCLUDE_DIR "/home/softwaremotion/Code/JackL4/MOT/")
  else()
    set(EIGEN3_INCLUDE_DIR "C:/C_Libs/eigen-3.4.0")
  endif()
endif()

if(ENABLE_visualizer)
  add_compile_definitions(Opencv_Operation=1)
  set(visualizer_SOURCE_FILE "${CMAKE_CURRENT_SOURCE_DIR}/Visualizer")

  if(ENABLE_linux)
    find_package(OpenCV REQUIRED)
  else()
    find_package(OpenCV)
  endif()
  
  if(OpenCV_FOUND)
    message(STATUS " ---- OpenCV directories: ${OpenCV_INCLUDE_DIRS}")
	message(STATUS " ---- OpenCV libs: ${OpenCV_LIBS}")
  else()
	message(STATUS "*********** OpenCV not found. Hard address have been set ***********")
	set(OpenCV_INCLUDE_DIRS "C:/opencv/build/include") #;C:/opencv/build/x64/vc17/lib
	file(GLOB OpenCV_LIBS "C:/opencv/build/x64/vc17/lib/*.lib")
  endif()
endif()

if(ENABLE_wrappers)
  add_compile_definitions(Wrapper_Operation=1)
  set(wrappers_SOURCE_FILE "${CMAKE_CURRENT_SOURCE_DIR}/wrappers")
endif()

if(ENABLE_offlineReadWrite)
  add_compile_definitions(OfflineReadWrite_Operation=1)
  set(offlineReadWrite_SOURCE_FILE "${CMAKE_CURRENT_SOURCE_DIR}/Read_Save_Data")
  set(libraries_SOURCE_FILE "${CMAKE_CURRENT_SOURCE_DIR}/libraries")
endif()

# add_subdirectory(proto)

# Set the C++ standard to C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


set(TRACKER_SRC_FILES)
set(TRACKER_INC_FILES)

list(APPEND TRACKER_SRC_FILES "${CMAKE_CURRENT_SOURCE_DIR}/source/*.cc" "${CMAKE_CURRENT_SOURCE_DIR}/libraries/Clipper2Lib/src/*.cpp")
list(APPEND TRACKER_SRC_FILES "${CMAKE_CURRENT_SOURCE_DIR}/source/track_updater/*.cc")
list(APPEND TRACKER_SRC_FILES "${CMAKE_CURRENT_SOURCE_DIR}/source/association/*.cc")
list(APPEND TRACKER_SRC_FILES "${CMAKE_CURRENT_SOURCE_DIR}/source/pre_and_post_process/*.cc")
list(APPEND TRACKER_SRC_FILES "${CMAKE_CURRENT_SOURCE_DIR}/source/estimator_filter/*.cc")

if(ENABLE_visualizer)
  list(APPEND TRACKER_SRC_FILES "${visualizer_SOURCE_FILE}/*.cc")
endif()
if(ENABLE_wrappers)
  list(APPEND TRACKER_SRC_FILES "${wrappers_SOURCE_FILE}/*.cpp")
endif()
if(ENABLE_offlineReadWrite)
  list(APPEND TRACKER_SRC_FILES "${offlineReadWrite_SOURCE_FILE}/*.cc")
endif()
file(GLOB TRACKER_SRC_FILES ${TRACKER_SRC_FILES})
if(NOT TRACKER_SRC_FILES)
  message(FATAL_ERROR "No source files found! Please check your ENABLE_* options and source file paths.")
else()
  message(STATUS " -*-*- Source files: ${TRACKER_SRC_FILES}")
endif()
add_library(${PROJECT_NAME} ${TRACKER_SRC_FILES} )

list(APPEND TRACKER_INC_FILES ${EIGEN3_INCLUDE_DIR} "${CMAKE_CURRENT_SOURCE_DIR}/include" "${CMAKE_CURRENT_SOURCE_DIR}/libraries/Clipper2Lib/include")
if(ENABLE_visualizer)
  list(APPEND TRACKER_INC_FILES ${visualizer_SOURCE_FILE} ${OpenCV_INCLUDE_DIRS})
endif()
if(ENABLE_wrappers)
  list(APPEND TRACKER_INC_FILES ${wrappers_SOURCE_FILE})
endif()
if(ENABLE_offlineReadWrite)
  list(APPEND TRACKER_INC_FILES ${offlineReadWrite_SOURCE_FILE} ${libraries_SOURCE_FILE})
endif()

message(STATUS " -*-*- Include files: ${TRACKER_INC_FILES}")
target_include_directories(${PROJECT_NAME} PUBLIC ${TRACKER_INC_FILES})

add_executable(mot_exe "source/main.cpp")

if(ENABLE_visualizer)
  target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBS})
else()
  link_libraries(${PROJECT_NAME})
endif()

target_link_libraries(mot_exe PRIVATE ${PROJECT_NAME})

if(ENABLE_visualizer OR ENABLE_offlineReadWrite)
  # Combined cleaning target
add_custom_target(clean_output ALL
    COMMAND ${CMAKE_COMMAND} -E rm -rf 
        $<TARGET_FILE_DIR:mot_exe>/resources/
        $<TARGET_FILE_DIR:mot_exe>/mot_output/
    VERBATIM)

# Create output directory (depends on cleaning)
add_custom_target(create_mot_output_dir ALL
    DEPENDS clean_output
    COMMAND ${CMAKE_COMMAND} -E
        make_directory "${CMAKE_CURRENT_BINARY_DIR}/mot_output")

# Copy resources (depends on cleaning)
add_custom_target(copy_resources ALL
    DEPENDS clean_output
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${PROJECT_SOURCE_DIR}/Read_Save_Data/resources
        $<TARGET_FILE_DIR:mot_exe>/resources)
endif()

# enable_testing()
# add_subdirectory(test)