# RNTupleTTreeChecker

## Overview

The `RNTupleTTreeChecker` is a tool designed to help the transition from the `TTree` data format to the new `RNTuple` format used in the high energy physics data analysis framework ROOT. This tool allows users to compare the structure and data consistency between them.

## Features

The tool is made up of two parts: The core file checking tool - `Checker` - and the Command Line Interface tool - `CheckerCLI`.
This allows for two different interfaces. The Checker is purely a tool made up of various functions that can draw values from TTrees and/or RNTuples. The CheckerCLI contains a complete set of checks accessible via the command line that call Checker functions and use returned values to create a comprehensive output stating results.

There are two possible output settings: _default_ and _verbose_.
With default settings, only found inconsistencies in the TTree and RNTuple are outputted. With the _-v_ verbose flag, all comparisons done are printed.

*Checks done:*
- **Structure Comparison**: Compares the number of entries and field count between `TTree` and `RNTuple`.
- **Schema Comparison**: Compares field names and types between `TTree` and `RNTuple`.
- **Data Consistency Checks**: Verifies data consistency across both formats.

## Directory Structure

These are the files of the project:
```
RNTupleTTreeChecker/
│
├── build/                 # Directory for build artifacts
├── testfiles/             # Files for Checker tests
│   ├── createMT           # Macro to create file with test TTrees
│   ├── createMR           # Macro to create file with test RNTuples
│   ├── mtt.root           # ROOT file with TTrees
│   └── mrn.root           # ROOT file with RNTuples
├── Checker.cxx	           # Implementation of the Checker class
├── Checker.hxx	           # Header file for the Checker class
├── CheckerCLI.cxx         # Implementation of the CheckerCLI command-line tool
├── CheckerCLI.hxx         # Header file for the CheckerCLI command-line tool
├── CheckerTests.cxx       # Unit Tests for Checker.cxx
└── CMakeLists.txt         # CMake build configuration file
```


## Using The CheckerCLI

### Prerequisites

- **ROOT Framework**: Ensure ROOT is installed on your system. You can download it from [ROOT's official website](https://root.cern/install/).
- **Git**: Required for version control and managing the repository.

### Installation

1. **Clone the Repository**

   ```
   git clone https://github.com/IdaCy/RNTupleTTreeChecker
   ```

2. **Navigate to the Repository Directory**

   ```
   cd RNTupleTTreeChecker
   ```

3. **Build the Project**

   ```
   mkdir build
   cd build
   cmake ..
   make
   ```

### Command Line Interface Run

The `CheckerCLI` is the CLI tool for comparing `TTree` and `RNTuple` formats. 

1. **Run the CheckerCLI**

   ```
   ./CheckerCLI -t <ttree_file> -r <rntuple_file> -tn <ttree_name> -rn <rntuple_name>
   ```

   - `-t`: Path to the TTree file.
   - `-r`: Path to the RNTuple file.
   - `-tn`: Name of the TTree within the file.
   - `-rn`: Name of the RNTuple within the file.

   Example:

   ```
   ./CheckerCLI -t ttreefile.root -r rntuplefile.root -tn tree_0 -rn rntuple_0
   ```

2. **Verbose Mode**

   To get detailed comparison results, use the `-v` flag:

   ```
   ./CheckerCLI -t ttreefile.root -r rntuplefile.root -tn tree_0 -rn rntuple_0 -v


## Tests

The Checker can be tested with its suit of unit tests by running:

```./CheckerTests```

The test files from /testfiles can be used to test the CheckerCLI.
They are generated with the /testfiles/createMT and /testfiles/createMR macros. Those macros can be run from the /testfiles directory with the commands:

```root -l -b -q 'createMT("mtt.root")'```
```root -l -b -q 'createMR("mrn.root")'```

## Contributions

We welcome contributions to improve the tool!
