# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.24

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Disable VCS-based implicit rules.
% : %,v

# Disable VCS-based implicit rules.
% : RCS/%

# Disable VCS-based implicit rules.
% : RCS/%,v

# Disable VCS-based implicit rules.
% : SCCS/s.%

# Disable VCS-based implicit rules.
% : s.%

.SUFFIXES: .hpux_make_needs_suffix_list

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /Applications/CLion.app/Contents/bin/cmake/mac/bin/cmake

# The command to remove a file.
RM = /Applications/CLion.app/Contents/bin/cmake/mac/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = "/Users/noahjoubert/Programming/C++/Chessington/The Engine"

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = "/Users/noahjoubert/Programming/C++/Chessington/The Engine/cmake-build-debug"

# Include any dependencies generated for this target.
include Boost_tests/CMakeFiles/Boost_Tests_run.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include Boost_tests/CMakeFiles/Boost_Tests_run.dir/compiler_depend.make

# Include the progress variables for this target.
include Boost_tests/CMakeFiles/Boost_Tests_run.dir/progress.make

# Include the compile flags for this target's objects.
include Boost_tests/CMakeFiles/Boost_Tests_run.dir/flags.make

Boost_tests/CMakeFiles/Boost_Tests_run.dir/[Move_Generation]_Unit_Tests/genKingBlockers.cpp.o: Boost_tests/CMakeFiles/Boost_Tests_run.dir/flags.make
Boost_tests/CMakeFiles/Boost_Tests_run.dir/[Move_Generation]_Unit_Tests/genKingBlockers.cpp.o: /Users/noahjoubert/Programming/C++/Chessington/The\ Engine/Boost_tests/[Move\ Generation]\ Unit\ Tests/genKingBlockers.cpp
Boost_tests/CMakeFiles/Boost_Tests_run.dir/[Move_Generation]_Unit_Tests/genKingBlockers.cpp.o: Boost_tests/CMakeFiles/Boost_Tests_run.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir="/Users/noahjoubert/Programming/C++/Chessington/The Engine/cmake-build-debug/CMakeFiles" --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object Boost_tests/CMakeFiles/Boost_Tests_run.dir/[Move_Generation]_Unit_Tests/genKingBlockers.cpp.o"
	cd "/Users/noahjoubert/Programming/C++/Chessington/The Engine/cmake-build-debug/Boost_tests" && /Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT Boost_tests/CMakeFiles/Boost_Tests_run.dir/[Move_Generation]_Unit_Tests/genKingBlockers.cpp.o -MF CMakeFiles/Boost_Tests_run.dir/[Move_Generation]_Unit_Tests/genKingBlockers.cpp.o.d -o CMakeFiles/Boost_Tests_run.dir/[Move_Generation]_Unit_Tests/genKingBlockers.cpp.o -c "/Users/noahjoubert/Programming/C++/Chessington/The Engine/Boost_tests/[Move Generation] Unit Tests/genKingBlockers.cpp"

Boost_tests/CMakeFiles/Boost_Tests_run.dir/[Move_Generation]_Unit_Tests/genKingBlockers.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/Boost_Tests_run.dir/[Move_Generation]_Unit_Tests/genKingBlockers.cpp.i"
	cd "/Users/noahjoubert/Programming/C++/Chessington/The Engine/cmake-build-debug/Boost_tests" && /Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E "/Users/noahjoubert/Programming/C++/Chessington/The Engine/Boost_tests/[Move Generation] Unit Tests/genKingBlockers.cpp" > CMakeFiles/Boost_Tests_run.dir/[Move_Generation]_Unit_Tests/genKingBlockers.cpp.i

Boost_tests/CMakeFiles/Boost_Tests_run.dir/[Move_Generation]_Unit_Tests/genKingBlockers.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/Boost_Tests_run.dir/[Move_Generation]_Unit_Tests/genKingBlockers.cpp.s"
	cd "/Users/noahjoubert/Programming/C++/Chessington/The Engine/cmake-build-debug/Boost_tests" && /Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S "/Users/noahjoubert/Programming/C++/Chessington/The Engine/Boost_tests/[Move Generation] Unit Tests/genKingBlockers.cpp" -o CMakeFiles/Boost_Tests_run.dir/[Move_Generation]_Unit_Tests/genKingBlockers.cpp.s

# Object files for target Boost_Tests_run
Boost_Tests_run_OBJECTS = \
"CMakeFiles/Boost_Tests_run.dir/[Move_Generation]_Unit_Tests/genKingBlockers.cpp.o"

# External object files for target Boost_Tests_run
Boost_Tests_run_EXTERNAL_OBJECTS =

Boost_tests/Boost_Tests_run: Boost_tests/CMakeFiles/Boost_Tests_run.dir/[Move_Generation]_Unit_Tests/genKingBlockers.cpp.o
Boost_tests/Boost_Tests_run: Boost_tests/CMakeFiles/Boost_Tests_run.dir/build.make
Boost_tests/Boost_Tests_run: /usr/local/include/boost_1_74_0/lib/libboost_unit_test_framework.dylib
Boost_tests/Boost_Tests_run: Boost_tests/CMakeFiles/Boost_Tests_run.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir="/Users/noahjoubert/Programming/C++/Chessington/The Engine/cmake-build-debug/CMakeFiles" --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable Boost_Tests_run"
	cd "/Users/noahjoubert/Programming/C++/Chessington/The Engine/cmake-build-debug/Boost_tests" && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/Boost_Tests_run.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
Boost_tests/CMakeFiles/Boost_Tests_run.dir/build: Boost_tests/Boost_Tests_run
.PHONY : Boost_tests/CMakeFiles/Boost_Tests_run.dir/build

Boost_tests/CMakeFiles/Boost_Tests_run.dir/clean:
	cd "/Users/noahjoubert/Programming/C++/Chessington/The Engine/cmake-build-debug/Boost_tests" && $(CMAKE_COMMAND) -P CMakeFiles/Boost_Tests_run.dir/cmake_clean.cmake
.PHONY : Boost_tests/CMakeFiles/Boost_Tests_run.dir/clean

Boost_tests/CMakeFiles/Boost_Tests_run.dir/depend:
	cd "/Users/noahjoubert/Programming/C++/Chessington/The Engine/cmake-build-debug" && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" "/Users/noahjoubert/Programming/C++/Chessington/The Engine" "/Users/noahjoubert/Programming/C++/Chessington/The Engine/Boost_tests" "/Users/noahjoubert/Programming/C++/Chessington/The Engine/cmake-build-debug" "/Users/noahjoubert/Programming/C++/Chessington/The Engine/cmake-build-debug/Boost_tests" "/Users/noahjoubert/Programming/C++/Chessington/The Engine/cmake-build-debug/Boost_tests/CMakeFiles/Boost_Tests_run.dir/DependInfo.cmake" --color=$(COLOR)
.PHONY : Boost_tests/CMakeFiles/Boost_Tests_run.dir/depend

