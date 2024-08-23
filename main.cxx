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
    //gErrorIgnoreLevel = kWarning;
    gErrorIgnoreLevel = kError;

    if (argc < 9) {
        std::cerr << "Usage: " << argv[0] << " -t <ttreeFile> -r <rntupleFile> -tn <ttreeName> -rn <rntupleName>\n";
        exit(1);
    }

    CheckerConfig config;
    bool verbose = false;
    for (int i = 1; i < argc; i += 2) {
        std::string arg = argv[i];
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
            verbose = true;  // ENABLE verbosity if '-v' is passed
        }
        else {
            std::cerr << "Unknown option: " << arg << std::endl;
            exit(1);
        }
    }

    config.fShouldRun = true;


    CheckerCLI cli;
    cli.SetVerbosity(verbose);
    cli.RunAll(config);

    return 0;
}
