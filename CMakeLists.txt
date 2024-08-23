cmake_minimum_required(VERSION 3.1)
project(Checker)

set(CMAKE_CXX_STANDARD 17)

set(ROOTSYS ~/root/install)
set(ROOT_INCLUDE_DIRS ${ROOTSYS}/include)
set(ROOT_LIBRARY_DIRS ${ROOTSYS}/lib)

find_library(ROOTCore NAMES Core HINTS ${ROOT_LIBRARY_DIRS})
find_library(ROOTHist NAMES Hist HINTS ${ROOT_LIBRARY_DIRS})
find_library(ROOTRIO NAMES RIO HINTS ${ROOT_LIBRARY_DIRS})
find_library(ROOTTree NAMES Tree HINTS ${ROOT_LIBRARY_DIRS})
find_library(ROOTRNTuple NAMES ROOTNTuple HINTS ${ROOT_LIBRARY_DIRS})
find_library(ROOTRNTupleUtil NAMES ROOTNTupleUtil HINTS ${ROOT_LIBRARY_DIRS})
find_library(ROOTGpad NAMES Gpad HINTS ${ROOT_LIBRARY_DIRS})
find_library(ROOTGraf NAMES Graf HINTS ${ROOT_LIBRARY_DIRS})
find_library(ROOTGraf3d NAMES Graf3d HINTS ${ROOT_LIBRARY_DIRS})
find_library(ROOTNet NAMES Net HINTS ${ROOT_LIBRARY_DIRS})

if(NOT ROOTCore OR NOT ROOTHist OR NOT ROOTRIO OR NOT ROOTTree OR NOT ROOTRNTuple OR NOT ROOTRNTupleUtil OR NOT ROOTGpad OR NOT ROOTGraf OR NOT ROOTGraf3d OR NOT ROOTNet)
    message(FATAL_ERROR "Could not find all required ROOT libraries")
endif()

include_directories(${ROOT_INCLUDE_DIRS})

add_library(CheckerLib
        Checker.cxx
        CheckerCLI.cxx
)

include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/release-1.11.0.zip
)

FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(Checker
        Checker.cxx
        CheckerCLI.cxx
        main.cxx
)

target_link_libraries(Checker
        PRIVATE
        CheckerLib
        ${ROOTCore}
        ${ROOTHist}
        ${ROOTRIO}
        ${ROOTTree}
        ${ROOTRNTuple}
        ${ROOTRNTupleUtil}
        ${ROOTGpad}
        ${ROOTGraf}
        ${ROOTGraf3d}
        ${ROOTNet}
)

add_executable(CheckerTests
        CheckerTests.cxx
)

target_link_libraries(CheckerTests
        PRIVATE
        CheckerLib
        gtest_main
        ${ROOTCore}
        ${ROOTHist}
        ${ROOTRIO}
        ${ROOTTree}
        ${ROOTRNTuple}
        ${ROOTRNTupleUtil}
        ${ROOTGpad}
        ${ROOTGraf}
        ${ROOTGraf3d}
        ${ROOTNet}
)

include(GoogleTest)
gtest_discover_tests(CheckerTests)

set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES "Checker;Checker.o")
