cmake_minimum_required(VERSION 3.18)
project(NetworkSimulation)

# Find the ns-3 package
find_package(PkgConfig REQUIRED)
pkg_check_modules(NS3 libns3.30.1-core-debug libns3.30.1-network-debug libns3.30.1-mobility-debug libns3.30.1-internet-debug libns3.30.1-stats-debug libns3.30.1-wifi-debug)

# Set the C++ standard to at least C++11
set(CMAKE_CXX_STANDARD 17)

# Add the source files
set(SOURCES
    wifi-example-apps.cc
    wifi-example-sim.cc
)

# Include the headers
set(HEADERS
    wifi-example-apps.h
)

# Create the executable
add_executable(network-simulation ${SOURCES} ${HEADERS})

# Link against ns3 libraries
target_link_libraries(network-simulation ${NS3_LIBRARIES})

# Include ns-3 include directories
target_include_directories(network-simulation PRIVATE ${NS3_INCLUDE_DIRS})

# Export compile_commands.json for use with tools like clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "")

# Set the output directory for the build
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Install the executable
install(TARGETS network-simulation DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)

install(FILES wifi-example-db.sh DESTINATION ${CMAKE_INSTALL_PREFIX}/bin PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ WORLD_EXECUTE WORLD_READ)
install(FILES wifi-example.gnuplot DESTINATION ${CMAKE_INSTALL_PREFIX}/bin PERMISSIONS OWNER_READ GROUP_READ WORLD_READ)
