cmake_minimum_required(VERSION 3.5)
project(DPsim CXX)

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
	set(TOPLEVEL_PROJECT ON)
else()
		set(TOPLEVEL_PROJECT OFF)
endif()

# Enable C++11 support
set(CMAKE_CXX_STANDARD 17)
add_definitions(-D_SILENCE_ALL_CXX17_DEPRECATION_WARNINGS)

set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/CMake;${PROJECT_SOURCE_DIR}/Dependencies/libcps/CMake")

set(PROJECT_AUTHOR "Institute for Automation of Complex Power Systems, RWTH Aachen University")
set(PROJECT_COPYRIGHT "2017, Institute for Automation of Complex Power Systems, RWTH Aachen University")

include(CheckCXXCompilerFlag)
if(MSVC)	
	check_cxx_compiler_flag("/W4 /WX" CXX_SUPPORTS_WERROR)

	if(CXX_SUPPORTS_WERROR)
	# TODO activate this again after fixing warnings
	#		set(DPSIM_CXX_FLAGS /W4 /WX)
	endif()
else()
	check_cxx_compiler_flag("-Wall -Werror" CXX_SUPPORTS_WERROR)

	if(CXX_SUPPORTS_WERROR)
		set(DPSIM_CXX_FLAGS -Wall -Werror)
	endif()
endif()

include(CheckSymbolExists)
check_symbol_exists(pipe unistd.h HAVE_PIPE)
check_symbol_exists(timerfd_create sys/timerfd.h HAVE_TIMERFD)
check_symbol_exists(getopt_long getopt.h HAVE_GETOPT)

# Get version info and buildid from Git
include(GetVersion)
GetVersion(${PROJECT_SOURCE_DIR} "DPSIM")

add_subdirectory(Dependencies/libcps)

find_package(PythonInterp 3.6)
find_package(PythonLibs 3.6)
find_package(Sundials)
find_package(VILLASnode)

if(PythonInterp_FOUND AND PythonLibs_FOUND)
	set(Python_FOUND ON)
endif()

if("${CMAKE_SYSTEM}" MATCHES "Linux")
	set(Linux_FOUND ON)
endif()

# Options
option(BUILD_SHARED_LIBS "Build shared library" OFF)
option(BUILD_EXAMPLES "Build C++ examples" ON)
option(COMPARE_REFERENCE "Download reference results and compare" OFF)

option(WITH_SUNDIALS "Enable sundials solver suite"         ${Sundials_FOUND})
option(WITH_SHMEM    "Enable shared memory interface"       ${VILLASnode_FOUND})
option(WITH_RT	     "Enable real-time features"            ${Linux_FOUND})
option(WITH_PYTHON   "Enable Python support"                ${Python_FOUND})
option(WITH_CIM      "Enable support for parsing CIM files" ${CIMpp_FOUND})

configure_file(
	${CMAKE_CURRENT_SOURCE_DIR}/Include/dpsim/Config.h.in
	${CMAKE_CURRENT_BINARY_DIR}/Include/dpsim/Config.h
)

set(INCLUDE_DIRS
	${CMAKE_CURRENT_BINARY_DIR}/Include
	${CMAKE_CURRENT_SOURCE_DIR}/Include
)

add_subdirectory(Tests)
add_subdirectory(Source)
add_subdirectory(Documentation)

if(BUILD_EXAMPLES)
	add_subdirectory(Examples)
endif(BUILD_EXAMPLES)

if(COMPARE_REFERENCE)
	include(ExternalProject)
	ExternalProject_Add(reference-results
	  GIT_REPOSITORY    https://git.rwth-aachen.de/acs/public/simulation/reference-results.git
	  #GIT_TAG           origin/master
	  PREFIX			reference-results
	  CONFIGURE_COMMAND	""
	  BUILD_COMMAND		""
	  INSTALL_COMMAND	""
	)
endif()

if(TOPLEVEL_PROJECT)
	# Show feature summary
	include(FeatureSummary)

	add_feature_info(CIM	WITH_CIM	"Loading Common Information Model Files")
	add_feature_info(Python WITH_PYTHON "Use DPsim as a Python module")
	add_feature_info(Shmem  WITH_SHMEM  "Interface DPsim solvers via shared-memory interfaces")
	add_feature_info(RT	    WITH_RT     "Extended real-time features")
	feature_summary(WHAT ALL VAR enabledFeaturesText)

	message(STATUS "Building ${CMAKE_PROJECT_NAME}:")
	message(STATUS "Feature summary for DPsim")
	message(STATUS "${enabledFeaturesText}")
    message(STATUS "  VERSION:         ${DPSIM_VERSION}")
    message(STATUS "  RELEASE:         ${DPSIM_RELEASE}")
    message(STATUS "  GIT_REV:         ${DPSIM_GIT_REV}")
    message(STATUS "  GIT_BRANCH:      ${DPSIM_GIT_BRANCH}")
    message(STATUS "  VARIANT:         ${DPSIM_VARIANT}")
    message(STATUS "  BUILD_ID:        ${DPSIM_BUILD_ID}")
    message(STATUS "  BUILD_DATE:      ${DPSIM_BUILD_DATE}")
endif()

include(DPsimPackaging)
