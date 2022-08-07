cmake_minimum_required (VERSION 3.1.0)
set (CMAKE_CXX_STANDARD 17)
project (SimpleMatchmaker)

add_subdirectory (enet)
#add_subdirectory (fltk)

file(GLOB_RECURSE server_source_files "Server/*.cpp")
file(GLOB_RECURSE clientDemo_source_files "clientDemo/*.cpp")
file(GLOB_RECURSE common_source_files "common/*.cpp")
file(GLOB_RECURSE serverConnection_source_files "ServerConnection/*.cpp")

add_executable(SimpleMatchmakerServer ${server_source_files} ${common_source_files})
add_executable(Client ${clientDemo_source_files} ${common_source_files} ${serverConnection_source_files})

target_link_libraries(SimpleMatchmakerServer enet)
target_include_directories(SimpleMatchmakerServer PUBLIC "enet/include")
target_include_directories(SimpleMatchmakerServer PUBLIC "Server/include")
target_include_directories(SimpleMatchmakerServer PUBLIC "common/include")

target_link_libraries(Client enet)
target_include_directories(Client PUBLIC "enet/include")
target_include_directories(Client PUBLIC "common/include")
target_include_directories(Client PUBLIC "ServerConnection")
if(WIN32)
  target_link_libraries(SimpleMatchmakerServer Winmm ws2_32)
  target_link_libraries(Client Winmm ws2_32)
endif()