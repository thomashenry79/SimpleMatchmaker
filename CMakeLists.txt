cmake_minimum_required (VERSION 3.1.0)
set (CMAKE_CXX_STANDARD 17)
project (SimpleMatchmaker)

add_subdirectory (enet)
#add_subdirectory (fltk)

file(GLOB_RECURSE source_files "src/*.cpp")
file(GLOB_RECURSE client_source_files "client/*.cpp")

add_executable(SimpleMatchmakerServer ${source_files})
add_executable(Client ${client_source_files})

target_link_libraries(SimpleMatchmakerServer enet)
target_include_directories(SimpleMatchmakerServer PUBLIC "enet/include")

target_link_libraries(Client enet)
#target_link_libraries(Client fltk)
target_include_directories(Client PUBLIC "enet/include")
#target_include_directories(Client PUBLIC "fltk/")
#target_include_directories(Client PUBLIC "fltk/FL")
#target_include_directories(Client PUBLIC "${PROJECT_BINARY_DIR}/fltk")

if(WIN32)
  target_link_libraries(SimpleMatchmakerServer Winmm ws2_32)
  target_link_libraries(Client Winmm ws2_32)
endif()