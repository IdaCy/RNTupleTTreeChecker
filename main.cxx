/// \file main.cxx
/// \ingroup NTuple ROOT7
/// \author Ida Caspary <ida.friederike.caspary@cern.ch>
/// \date 2024-10-14
/// \warning This is part of the ROOT 7 prototype! It will change without notice. It might trigger earthquakes. Feedback
/// is welcome!

/*************************************************************************
 * Copyright (C) 1995-2023, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#include "Checker.hxx"
#include "CheckerCLI.hxx"
#include <iostream>

using namespace Checker;

int main(int argc, char *argv[]) {
    gErrorIgnoreLevel = kError;

    // Check if the number of arguments is less than 9; if true, print usage instructions and exit
    if (argc < 9) {
        std::cerr << "Usage: " << argv[0] << " -t <ttreeFile> -r <rntupleFile> -tn <ttreeName> -rn <rntupleName>\n";
        exit(1);
    }

    // Create a configuration object to store command-line arguments
    CheckerConfig config;
    bool verbose = false; // Initialize a flag to check if verbose mode should be enabled

    // Loop through the command-line arguments to parse options and their values
    for (int i = 1; i < argc; i += 2) {
        std::string arg = argv[i]; // Store the current argument in a string
        if (arg == "-t") {
            config.fTTreeFile = argv[i + 1];
        }
        else if (arg == "-r") {
            config.fRNTupleFile = argv[i + 1];
        }
        else if (arg == "-tn") {
            config.fTTreeName = argv[i + 1];
        }
        else if (arg == "-rn") {
            config.fRNTupleName = argv[i + 1];
        }
        else if (arg == "-v") {
            verbose = true;  // Enable verbosity if '-v' is passed
        }
        else {
            std::cerr << "Unknown option: " << arg << std::endl;
            exit(1);
        }
    }

    // Set the flag to indicate the comparison should run
    config.fShouldRun = true;

    CheckerCLI cli;            // Create a CLI object to handle user interaction and output
    cli.SetVerbosity(verbose); // Set verbosity in the CLI object based on the command-line option
    cli.RunAll(config);        // Run the main comparison logic using the configured options

    return 0;  // Exit the program successfully
}
