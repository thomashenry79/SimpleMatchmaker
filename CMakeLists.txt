cmake_minimum_required (VERSION 3.1.0)
set (CMAKE_CXX_STANDARD 17)
project (SimpleMatchmaker)

add_subdirectory (enet)
#add_subdirectory (fltk)

file(GLOB_RECURSE server_source_files "src/*.cpp")
file(GLOB_RECURSE client_source_files "client/*.cpp")
file(GLOB_RECURSE common_source_files "common/*.cpp")

add_executable(SimpleMatchmakerServer ${server_source_files} ${common_source_files})
add_executable(Client ${client_source_files} ${common_source_files})

target_link_libraries(SimpleMatchmakerServer enet)
target_include_directories(SimpleMatchmakerServer PUBLIC "enet/include")
target_include_directories(SimpleMatchmakerServer PUBLIC "src/include")
target_include_directories(SimpleMatchmakerServer PUBLIC "common/include")

target_link_libraries(Client enet)
target_include_directories(Client PUBLIC "enet/include")
target_include_directories(Client PUBLIC "common/include")
if(WIN32)
  target_link_libraries(SimpleMatchmakerServer Winmm ws2_32)
  target_link_libraries(Client Winmm ws2_32)
endif()