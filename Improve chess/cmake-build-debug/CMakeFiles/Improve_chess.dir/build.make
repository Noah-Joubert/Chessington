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
CMAKE_SOURCE_DIR = "/Users/noahjoubert/CLionProjects/Improve chess"

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = "/Users/noahjoubert/CLionProjects/Improve chess/cmake-build-debug"

# Include any dependencies generated for this target.
include CMakeFiles/Improve_chess.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include CMakeFiles/Improve_chess.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/Improve_chess.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/Improve_chess.dir/flags.make

CMakeFiles/Improve_chess.dir/src/main.cpp.o: CMakeFiles/Improve_chess.dir/flags.make
CMakeFiles/Improve_chess.dir/src/main.cpp.o: /Users/noahjoubert/CLionProjects/Improve\ chess/src/main.cpp
CMakeFiles/Improve_chess.dir/src/main.cpp.o: CMakeFiles/Improve_chess.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir="/Users/noahjoubert/CLionProjects/Improve chess/cmake-build-debug/CMakeFiles" --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/Improve_chess.dir/src/main.cpp.o"
	/Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/Improve_chess.dir/src/main.cpp.o -MF CMakeFiles/Improve_chess.dir/src/main.cpp.o.d -o CMakeFiles/Improve_chess.dir/src/main.cpp.o -c "/Users/noahjoubert/CLionProjects/Improve chess/src/main.cpp"

CMakeFiles/Improve_chess.dir/src/main.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/Improve_chess.dir/src/main.cpp.i"
	/Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E "/Users/noahjoubert/CLionProjects/Improve chess/src/main.cpp" > CMakeFiles/Improve_chess.dir/src/main.cpp.i

CMakeFiles/Improve_chess.dir/src/main.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/Improve_chess.dir/src/main.cpp.s"
	/Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S "/Users/noahjoubert/CLionProjects/Improve chess/src/main.cpp" -o CMakeFiles/Improve_chess.dir/src/main.cpp.s

# Object files for target Improve_chess
Improve_chess_OBJECTS = \
"CMakeFiles/Improve_chess.dir/src/main.cpp.o"

# External object files for target Improve_chess
Improve_chess_EXTERNAL_OBJECTS =

Improve_chess: CMakeFiles/Improve_chess.dir/src/main.cpp.o
Improve_chess: CMakeFiles/Improve_chess.dir/build.make
Improve_chess: CMakeFiles/Improve_chess.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir="/Users/noahjoubert/CLionProjects/Improve chess/cmake-build-debug/CMakeFiles" --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable Improve_chess"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/Improve_chess.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/Improve_chess.dir/build: Improve_chess
.PHONY : CMakeFiles/Improve_chess.dir/build

CMakeFiles/Improve_chess.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/Improve_chess.dir/cmake_clean.cmake
.PHONY : CMakeFiles/Improve_chess.dir/clean

CMakeFiles/Improve_chess.dir/depend:
	cd "/Users/noahjoubert/CLionProjects/Improve chess/cmake-build-debug" && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" "/Users/noahjoubert/CLionProjects/Improve chess" "/Users/noahjoubert/CLionProjects/Improve chess" "/Users/noahjoubert/CLionProjects/Improve chess/cmake-build-debug" "/Users/noahjoubert/CLionProjects/Improve chess/cmake-build-debug" "/Users/noahjoubert/CLionProjects/Improve chess/cmake-build-debug/CMakeFiles/Improve_chess.dir/DependInfo.cmake" --color=$(COLOR)
.PHONY : CMakeFiles/Improve_chess.dir/depend

