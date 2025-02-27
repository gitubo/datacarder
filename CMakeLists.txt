cmake_minimum_required(VERSION 3.0)

project(myproject)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_VERBOSE_MAKEFILE OFF)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g3 -O2")

add_library(proto_service STATIC proto/cpp/service.grpc.pb.cc proto/cpp/service.pb.cc)
add_library(nlohmann_json INTERFACE)

add_library(catalog src/catalog/SchemaCatalog.cpp)
add_library(logger src/logger/Logger.cpp)
add_library(bitstream src/bitstream/BitStream.cpp)

add_executable(anyformat src/main.cpp)

find_package(gRPC CONFIG REQUIRED)
find_package(Lua REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(LUAJIT REQUIRED luajit)
message(STATUS "LuaJIT Include Dir: ${LUAJIT_INCLUDE_DIRS}")
message(STATUS "LuaJIT Libraries: ${LUAJIT_LIBRARIES}")

#target_include_directories(anyformat PRIVATE ${LUA_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}/proto/cpp)
target_include_directories(anyformat PRIVATE  ${LUAJIT_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}/proto/cpp)
target_include_directories(nlohmann_json INTERFACE /app/json-3.11.2/include)

target_link_libraries(catalog PRIVATE nlohmann_json)

#target_link_libraries(anyformat PRIVATE ${LUA_LIBRARIES} logger bitstream catalog proto_service gRPC::grpc++ nlohmann_json)
#set(CMAKE_EXE_LINKER_FLAGS "-v")
target_link_libraries(anyformat PRIVATE ${LUAJIT_LIBRARIES} -ldl -lm logger bitstream catalog proto_service gRPC::grpc++ nlohmann_json)

# add_executable(client test/client.cpp)
# target_include_directories(client PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/proto/cpp)
# target_link_libraries(client PRIVATE proto_service gRPC::grpc++)
