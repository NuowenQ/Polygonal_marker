# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.10

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/odroid/projects/nevangeliou_GapterUAV/src/flight_pkg

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/odroid/projects/nevangeliou_GapterUAV/build/flight_pkg

# Include any dependencies generated for this target.
include CMakeFiles/RoombaFollowScan.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/RoombaFollowScan.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/RoombaFollowScan.dir/flags.make

CMakeFiles/RoombaFollowScan.dir/src/followWithScan.cpp.o: CMakeFiles/RoombaFollowScan.dir/flags.make
CMakeFiles/RoombaFollowScan.dir/src/followWithScan.cpp.o: /home/odroid/projects/nevangeliou_GapterUAV/src/flight_pkg/src/followWithScan.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/odroid/projects/nevangeliou_GapterUAV/build/flight_pkg/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/RoombaFollowScan.dir/src/followWithScan.cpp.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/RoombaFollowScan.dir/src/followWithScan.cpp.o -c /home/odroid/projects/nevangeliou_GapterUAV/src/flight_pkg/src/followWithScan.cpp

CMakeFiles/RoombaFollowScan.dir/src/followWithScan.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/RoombaFollowScan.dir/src/followWithScan.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/odroid/projects/nevangeliou_GapterUAV/src/flight_pkg/src/followWithScan.cpp > CMakeFiles/RoombaFollowScan.dir/src/followWithScan.cpp.i

CMakeFiles/RoombaFollowScan.dir/src/followWithScan.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/RoombaFollowScan.dir/src/followWithScan.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/odroid/projects/nevangeliou_GapterUAV/src/flight_pkg/src/followWithScan.cpp -o CMakeFiles/RoombaFollowScan.dir/src/followWithScan.cpp.s

CMakeFiles/RoombaFollowScan.dir/src/followWithScan.cpp.o.requires:

.PHONY : CMakeFiles/RoombaFollowScan.dir/src/followWithScan.cpp.o.requires

CMakeFiles/RoombaFollowScan.dir/src/followWithScan.cpp.o.provides: CMakeFiles/RoombaFollowScan.dir/src/followWithScan.cpp.o.requires
	$(MAKE) -f CMakeFiles/RoombaFollowScan.dir/build.make CMakeFiles/RoombaFollowScan.dir/src/followWithScan.cpp.o.provides.build
.PHONY : CMakeFiles/RoombaFollowScan.dir/src/followWithScan.cpp.o.provides

CMakeFiles/RoombaFollowScan.dir/src/followWithScan.cpp.o.provides.build: CMakeFiles/RoombaFollowScan.dir/src/followWithScan.cpp.o


# Object files for target RoombaFollowScan
RoombaFollowScan_OBJECTS = \
"CMakeFiles/RoombaFollowScan.dir/src/followWithScan.cpp.o"

# External object files for target RoombaFollowScan
RoombaFollowScan_EXTERNAL_OBJECTS =

/home/odroid/projects/nevangeliou_GapterUAV/devel/lib/flight_pkg/RoombaFollowScan: CMakeFiles/RoombaFollowScan.dir/src/followWithScan.cpp.o
/home/odroid/projects/nevangeliou_GapterUAV/devel/lib/flight_pkg/RoombaFollowScan: CMakeFiles/RoombaFollowScan.dir/build.make
/home/odroid/projects/nevangeliou_GapterUAV/devel/lib/flight_pkg/RoombaFollowScan: /opt/ros/melodic/lib/libmavros.so
/home/odroid/projects/nevangeliou_GapterUAV/devel/lib/flight_pkg/RoombaFollowScan: /usr/lib/arm-linux-gnueabihf/libGeographic.so
/home/odroid/projects/nevangeliou_GapterUAV/devel/lib/flight_pkg/RoombaFollowScan: /opt/ros/melodic/lib/libeigen_conversions.so
/home/odroid/projects/nevangeliou_GapterUAV/devel/lib/flight_pkg/RoombaFollowScan: /opt/ros/melodic/lib/liborocos-kdl.so.1.4.0
/home/odroid/projects/nevangeliou_GapterUAV/devel/lib/flight_pkg/RoombaFollowScan: /opt/ros/melodic/lib/libmavconn.so
/home/odroid/projects/nevangeliou_GapterUAV/devel/lib/flight_pkg/RoombaFollowScan: /opt/ros/melodic/lib/libclass_loader.so
/home/odroid/projects/nevangeliou_GapterUAV/devel/lib/flight_pkg/RoombaFollowScan: /usr/lib/libPocoFoundation.so
/home/odroid/projects/nevangeliou_GapterUAV/devel/lib/flight_pkg/RoombaFollowScan: /usr/lib/arm-linux-gnueabihf/libdl.so
/home/odroid/projects/nevangeliou_GapterUAV/devel/lib/flight_pkg/RoombaFollowScan: /opt/ros/melodic/lib/libroslib.so
/home/odroid/projects/nevangeliou_GapterUAV/devel/lib/flight_pkg/RoombaFollowScan: /opt/ros/melodic/lib/librospack.so
/home/odroid/projects/nevangeliou_GapterUAV/devel/lib/flight_pkg/RoombaFollowScan: /usr/lib/arm-linux-gnueabihf/libpython2.7.so
/home/odroid/projects/nevangeliou_GapterUAV/devel/lib/flight_pkg/RoombaFollowScan: /usr/lib/arm-linux-gnueabihf/libboost_program_options.so
/home/odroid/projects/nevangeliou_GapterUAV/devel/lib/flight_pkg/RoombaFollowScan: /usr/lib/arm-linux-gnueabihf/libtinyxml2.so
/home/odroid/projects/nevangeliou_GapterUAV/devel/lib/flight_pkg/RoombaFollowScan: /opt/ros/melodic/lib/libdynamic_reconfigure_config_init_mutex.so
/home/odroid/projects/nevangeliou_GapterUAV/devel/lib/flight_pkg/RoombaFollowScan: /opt/ros/melodic/lib/libtf.so
/home/odroid/projects/nevangeliou_GapterUAV/devel/lib/flight_pkg/RoombaFollowScan: /opt/ros/melodic/lib/libtf2_ros.so
/home/odroid/projects/nevangeliou_GapterUAV/devel/lib/flight_pkg/RoombaFollowScan: /opt/ros/melodic/lib/libactionlib.so
/home/odroid/projects/nevangeliou_GapterUAV/devel/lib/flight_pkg/RoombaFollowScan: /opt/ros/melodic/lib/libmessage_filters.so
/home/odroid/projects/nevangeliou_GapterUAV/devel/lib/flight_pkg/RoombaFollowScan: /opt/ros/melodic/lib/libtf2.so
/home/odroid/projects/nevangeliou_GapterUAV/devel/lib/flight_pkg/RoombaFollowScan: /opt/ros/melodic/lib/libroscpp.so
/home/odroid/projects/nevangeliou_GapterUAV/devel/lib/flight_pkg/RoombaFollowScan: /usr/lib/arm-linux-gnueabihf/libboost_filesystem.so
/home/odroid/projects/nevangeliou_GapterUAV/devel/lib/flight_pkg/RoombaFollowScan: /opt/ros/melodic/lib/librosconsole.so
/home/odroid/projects/nevangeliou_GapterUAV/devel/lib/flight_pkg/RoombaFollowScan: /opt/ros/melodic/lib/librosconsole_log4cxx.so
/home/odroid/projects/nevangeliou_GapterUAV/devel/lib/flight_pkg/RoombaFollowScan: /opt/ros/melodic/lib/librosconsole_backend_interface.so
/home/odroid/projects/nevangeliou_GapterUAV/devel/lib/flight_pkg/RoombaFollowScan: /usr/lib/arm-linux-gnueabihf/liblog4cxx.so
/home/odroid/projects/nevangeliou_GapterUAV/devel/lib/flight_pkg/RoombaFollowScan: /usr/lib/arm-linux-gnueabihf/libboost_regex.so
/home/odroid/projects/nevangeliou_GapterUAV/devel/lib/flight_pkg/RoombaFollowScan: /opt/ros/melodic/lib/libxmlrpcpp.so
/home/odroid/projects/nevangeliou_GapterUAV/devel/lib/flight_pkg/RoombaFollowScan: /opt/ros/melodic/lib/libroscpp_serialization.so
/home/odroid/projects/nevangeliou_GapterUAV/devel/lib/flight_pkg/RoombaFollowScan: /opt/ros/melodic/lib/librostime.so
/home/odroid/projects/nevangeliou_GapterUAV/devel/lib/flight_pkg/RoombaFollowScan: /opt/ros/melodic/lib/libcpp_common.so
/home/odroid/projects/nevangeliou_GapterUAV/devel/lib/flight_pkg/RoombaFollowScan: /usr/lib/arm-linux-gnueabihf/libboost_system.so
/home/odroid/projects/nevangeliou_GapterUAV/devel/lib/flight_pkg/RoombaFollowScan: /usr/lib/arm-linux-gnueabihf/libboost_thread.so
/home/odroid/projects/nevangeliou_GapterUAV/devel/lib/flight_pkg/RoombaFollowScan: /usr/lib/arm-linux-gnueabihf/libboost_chrono.so
/home/odroid/projects/nevangeliou_GapterUAV/devel/lib/flight_pkg/RoombaFollowScan: /usr/lib/arm-linux-gnueabihf/libboost_date_time.so
/home/odroid/projects/nevangeliou_GapterUAV/devel/lib/flight_pkg/RoombaFollowScan: /usr/lib/arm-linux-gnueabihf/libboost_atomic.so
/home/odroid/projects/nevangeliou_GapterUAV/devel/lib/flight_pkg/RoombaFollowScan: /usr/lib/arm-linux-gnueabihf/libpthread.so
/home/odroid/projects/nevangeliou_GapterUAV/devel/lib/flight_pkg/RoombaFollowScan: /usr/lib/arm-linux-gnueabihf/libconsole_bridge.so.0.4
/home/odroid/projects/nevangeliou_GapterUAV/devel/lib/flight_pkg/RoombaFollowScan: CMakeFiles/RoombaFollowScan.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/odroid/projects/nevangeliou_GapterUAV/build/flight_pkg/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable /home/odroid/projects/nevangeliou_GapterUAV/devel/lib/flight_pkg/RoombaFollowScan"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/RoombaFollowScan.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/RoombaFollowScan.dir/build: /home/odroid/projects/nevangeliou_GapterUAV/devel/lib/flight_pkg/RoombaFollowScan

.PHONY : CMakeFiles/RoombaFollowScan.dir/build

CMakeFiles/RoombaFollowScan.dir/requires: CMakeFiles/RoombaFollowScan.dir/src/followWithScan.cpp.o.requires

.PHONY : CMakeFiles/RoombaFollowScan.dir/requires

CMakeFiles/RoombaFollowScan.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/RoombaFollowScan.dir/cmake_clean.cmake
.PHONY : CMakeFiles/RoombaFollowScan.dir/clean

CMakeFiles/RoombaFollowScan.dir/depend:
	cd /home/odroid/projects/nevangeliou_GapterUAV/build/flight_pkg && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/odroid/projects/nevangeliou_GapterUAV/src/flight_pkg /home/odroid/projects/nevangeliou_GapterUAV/src/flight_pkg /home/odroid/projects/nevangeliou_GapterUAV/build/flight_pkg /home/odroid/projects/nevangeliou_GapterUAV/build/flight_pkg /home/odroid/projects/nevangeliou_GapterUAV/build/flight_pkg/CMakeFiles/RoombaFollowScan.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/RoombaFollowScan.dir/depend
