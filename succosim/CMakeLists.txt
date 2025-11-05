cmake_minimum_required(VERSION 3.3 FATAL_ERROR)

# vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
# set project name here
# e.g. set(ENV{PROJECTNAME} succosim)
set(ENV{PROJECTNAME} succosim)
# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

project($ENV{PROJECTNAME})

option(WITH_GEANT4_UIVIS "Build with Geant4 UI and Vis drivers" ON)
if(WITH_GEANT4_UIVIS)
  find_package(Geant4 REQUIRED ui_all vis_all)
else()
  find_package(Geant4 REQUIRED)
endif()

include(${Geant4_USE_FILE})
include_directories(${PROJECT_SOURCE_DIR}/include)

file(GLOB_RECURSE headers ${PROJECT_SOURCE_DIR}/include/*.hh)
file(GLOB_RECURSE sources ${PROJECT_SOURCE_DIR}/src/*.cc)
file(GLOB_RECURSE macros RELATIVE ${PROJECT_SOURCE_DIR} macros/*.mac)

foreach(_file ${macros})
  configure_file(
    ${_file}
    ${PROJECT_BINARY_DIR}/${_file}
    COPYONLY
    )
endforeach()

add_executable($ENV{PROJECTNAME} main.cc ${sources} ${headers})
target_link_libraries($ENV{PROJECTNAME} ${Geant4_LIBRARIES})

install(TARGETS $ENV{PROJECTNAME} DESTINATION bin)

file(MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/out_data)
