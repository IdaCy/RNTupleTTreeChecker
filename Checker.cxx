/// \file Checker.cxx
/// \ingroup NTuple ROOT7
/// \author Ida Caspary <ida.caspary@gmail.com>
/// \date 2024-10-14
/// \warning This is part of the ROOT 7 prototype! It will change without notice. It might trigger earthquakes. Feedback
/// is welcome!

/*************************************************************************
 * Copyright (C) 1995-2023, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CChecker::REDITS.             *
 *************************************************************************/

#include "Checker.hxx"
#include <ROOT/RNTuple.hxx>
#include <ROOT/RNTupleModel.hxx>
#include <ROOT/RNTupleReader.hxx>
#include <ROOT/RNTupleInspector.hxx>
#include <ROOT/RPageStorageFile.hxx>
#include <ROOT/RNTupleDescriptor.hxx>
#include <ROOT/RError.hxx>
#include <ROOT/RField.hxx>
#include <ROOT/RNTupleUtil.hxx>

#include <iomanip>
#include <TTree.h>
#include <TFile.h>
#include <TLeaf.h>
#include <TBranch.h>
#include <TKey.h>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <fstream>
#include <regex>

namespace Checker {

    Checker::Checker(const std::string& ttreeFile, const std::string& rntupleFile, const std::string& ttreeName, const std::string& rntupleName)
        : fTTreeFile(ttreeFile), fRNTupleFile(rntupleFile), fTTreeName(ttreeName), fRNTupleName(rntupleName) {

        // Open the TTree file
        tfile = std::unique_ptr<TFile>(TFile::Open(ttreeFile.c_str()));
        if (!tfile || tfile->IsZombie()) {
            throw std::runtime_error("Cannot open TTree file: " + ttreeFile);
        }
        // Retrieve the TTree object
        tfile = std::unique_ptr<TFile>(TFile::Open(ttreeFile.c_str()));
        ttree = dynamic_cast<TTree*>(tfile->Get(ttreeName.c_str()));
        if (!TTreeExists()) {
            throw std::runtime_error("Cannot find TTree: " + ttreeName + " in file: " + ttreeFile);
        }

        // Open the RNTuple file
        rfile = std::unique_ptr<TFile>(TFile::Open(rntupleFile.c_str()));
        if (!rfile || rfile->IsZombie()) {
            throw std::runtime_error("Cannot open RNTuple file: " + rntupleFile);
        }
        if (!RNTupleExists()) {
            throw std::runtime_error("Cannot find RNTuple: " + rntupleName + " in file: " + rntupleFile);
        }

        // Initialize the RNTuple reader
        rntupleReader = ROOT::Experimental::RNTupleReader::Open(rntupleName, rntupleFile);
        if (!rntupleReader) {
            throw std::runtime_error("Failed to open RNTupleReader.");
        }
    }

    Checker::~Checker() {
        if (tfile) {
            tfile->Close();
        }
    }

    bool Checker::TTreeExists() {
        try {
            // Check if the TFile pointer is valid and the file is not in a zombie state
            if (!tfile || tfile->IsZombie()) {
                return false;  // If the file is not valid, the TTree cannot exist
            }
            // Attempt to retrieve the TTree object from the file using its name
            const auto ttree = dynamic_cast<TTree*>(tfile->Get(fTTreeName.c_str()));
            if (!ttree) { // Check if the retrieved object is a valid TTree
                return false;
            }
        }
        catch (const std::exception& e) {
            std::cerr << "Error checking TTree existence: " << e.what() << std::endl;
            return false;
        }
        return true;
    }

    bool Checker::RNTupleExists() {
        try {
            // Retrieve the list of keys (objects) stored in the ROOT file
            auto keys = rfile->GetListOfKeys();

            // Iterate over the keys to check if an RNTuple with the specified name exists
            for (int i = 0; i < keys->GetEntries(); ++i) {

                // Cast each key to a TKey object to access its properties
                auto key = dynamic_cast<TKey*>(keys->At(i));

                // Check if the key corresponds to an RNTuple (by class name) and if the name matches fRNTupleName
                if (std::string(key->GetClassName()) == "ROOT::Experimental::RNTuple" && std::string(key->GetName()) == fRNTupleName) {
                    return true; // Both conditions are met - the RNTuple exists
                }
            }
        }
        catch (const std::exception& e) {
            std::cerr << "Error checking RNTuple existence: " << e.what() << std::endl;
            return false;
        }
        return false;
    }

    std::pair<int, int> Checker::CountEntries() {
        return { static_cast<int>(ttree->GetEntries()), static_cast<int>(rntupleReader->GetNEntries()) };
    }

    std::pair<int, int> Checker::CountFields() {
        // Get TTree field count
        const auto ttreeBranches = ttree->GetListOfBranches();
        const int ttreeFieldCount = ttreeBranches ? ttreeBranches->GetEntries() : 0;

        // To store counted fields
        int rntupleFieldCount = 0;
        try {
            // Using RNTupleInspector to count fields based on type patterns - for field matching capabilities
            const auto inspector = ROOT::Experimental::RNTupleInspector::Create(fRNTupleName, fRNTupleFile);
            std::regex typePattern(".*");
            rntupleFieldCount = inspector->GetFieldCountByType(typePattern, true);
        }
        catch (const std::exception& e) {
            std::cerr << "Error creating RNTupleInspector: " << e.what() << std::endl;
            return { ttreeFieldCount, 0 }; // Return 0 for RNTuple field count if there was an error
        }

        // Count and exclude fields named "_0" in the RNTuple
        int extraFieldCount = 0;
        try {
            for (int i = 0; i < rntupleFieldCount - 1; ++i) {
                try {
                    const auto& fieldDescriptor = rntupleReader->GetDescriptor().GetFieldDescriptor(i);
                    if (fieldDescriptor.GetFieldName() == "_0") {
                        ++extraFieldCount;
                    }
                }
                catch (const std::out_of_range& e) {
                    std::cerr << "Index out of range accessing RNTuple field descriptor: " << e.what() << "\n" << std::endl;
                    // Skip this index and continue
                }
                catch (const std::exception& e) {
                    std::cerr << "Error accessing RNTuple field descriptor: " << e.what() << std::endl;
                    // Skip this index and continue
                }
            }
        }
        catch (const std::exception& e) {
            std::cerr << "Error accessing RNTuple field descriptors: " << e.what() << std::endl;
            return { ttreeFieldCount, 0 }; // Return 0 for RNTuple field count if there was an error
        }

        return { ttreeFieldCount, rntupleFieldCount - extraFieldCount - 1 };
    }

    std::vector<std::pair<std::string, std::string>> Checker::CompareFieldNames() {
        std::vector<std::pair<std::string, std::string>> fieldNames;

        const auto ttreeBranches = ttree->GetListOfBranches();
        const int ttreeFieldCount = ttreeBranches->GetEntries();

        // Store RNTuple fields in a map for easy lookup
        std::unordered_map<std::string, std::string> rntupleFields;
        const auto& descriptor = rntupleReader->GetDescriptor();
        const int rntupleFieldCount = descriptor.GetNFields();

        // Collect RNTuple field names
        for (int i = 0; i < rntupleFieldCount - 1; ++i) {
            try {
                const auto& fieldDescriptor = descriptor.GetFieldDescriptor(i);
                const std::string& fieldName = fieldDescriptor.GetFieldName();
                if (fieldName != "_0") { // Skip fields named "_0"
                    rntupleFields[fieldName] = fieldName;
                }
            }
            catch (const std::exception& e) {
                std::cerr << "Error accessing field descriptor at index " << i << ": " << e.what() << std::endl;
            }
        }

        // Check for matching fields RNTuple-TTree
        for (int i = 0; i < ttreeFieldCount; ++i) {
            const auto branch = dynamic_cast<TBranch*>(ttreeBranches->At(i));
            if (!branch) {
                std::cerr << "Error: Branch at index " << i << " is not a TBranch." << std::endl;
                fieldNames.emplace_back("Invalid branch", "No matching field");
                continue;
            }

            const std::string branchName = branch->GetName();
            auto it = rntupleFields.find(branchName);
            if (it != rntupleFields.end()) {
                fieldNames.emplace_back(branchName, it->second);
                rntupleFields.erase(it); // Remove matched field
            }
            else {
                fieldNames.emplace_back(branchName, "No match");
            }
        }

        // Add remaining unmatched RNTuple fields
        for (const auto& field : rntupleFields) {
            fieldNames.emplace_back("No match", field.first);
        }

        return fieldNames;
    }

    std::vector<std::tuple<std::string, std::string, std::string>> Checker::CompareFieldTypes() {
        std::vector<std::tuple<std::string, std::string, std::string>> fieldTypes;

        const auto ttreeBranches = ttree->GetListOfBranches();
        const int ttreeFieldCount = ttreeBranches->GetEntries();

        // Store RNTuple fields and their types in a map for easy lookup
        std::unordered_map<std::string, std::string> rntupleFieldTypes;
        const auto& descriptor = rntupleReader->GetDescriptor();
        const int rntupleFieldCount = descriptor.GetNFields();

        for (int i = 0; i < rntupleFieldCount - 1; ++i) {
            try {
                const auto& fieldDescriptor = descriptor.GetFieldDescriptor(i);
                const std::string& fieldName = fieldDescriptor.GetFieldName();
                if (fieldName != "_0") { // Skip fields named "_0"
                    rntupleFieldTypes[fieldName] = fieldDescriptor.GetTypeName();
                }
            }
            catch (const std::exception& e) {
                std::cerr << "Error accessing field descriptor at index " << i << ": " << e.what() << std::endl;
            }
        }

        // Compare TTree - RNTuple field names
        for (int i = 0; i < ttreeFieldCount; ++i) {
            const auto branch = dynamic_cast<TBranch*>(ttreeBranches->At(i));
            if (!branch) {
                std::cerr << "Error: Branch at index " << i << " is not a TBranch." << std::endl;
                fieldTypes.emplace_back("Invalid branch", "No matching field", "No matching field");
                continue;
            }

            const std::string branchName = branch->GetName();
            const std::string ttreeType = branch->GetLeaf(branchName.c_str())->GetTypeName();

            auto it = rntupleFieldTypes.find(branchName);
            if (it != rntupleFieldTypes.end()) {
                fieldTypes.emplace_back(branchName, ttreeType, it->second);
                rntupleFieldTypes.erase(it); // Remove matched field
            }
            else {
                fieldTypes.emplace_back(branchName, ttreeType, "No match");
            }
        }

        // Add remaining unmatched RNTuple fields
        for (const auto& field : rntupleFieldTypes) {
            fieldTypes.emplace_back(field.first, "No match", field.second);
        }
        return fieldTypes;
    }

    std::string Checker::Checker::ExtractSubFieldType(const std::string& vectorType) {
        std::string subFieldType;

        // Characters to look for
        size_t start = vectorType.find('<');
        size_t end = vectorType.find('>');

        // Find characters in type string
        if (start != std::string::npos && end != std::string::npos && start < end) {
            subFieldType = vectorType.substr(start + 1, end - start - 1);
        }
        return subFieldType;
    }

    size_t Checker::CountSubFieldsInBranch(TBranch* branch, const std::string& branchTypeName) {
        size_t totalSubfields = 0;

        // For integer branches
        if (branchTypeName == "vector<int>") {
            std::vector<int>* vec = nullptr;
            branch->SetAddress(&vec);

            // Iterate over each entry in the branch to accumulate the total number of subfields
            for (Long64_t j = 0; j < branch->GetEntries(); ++j) {
                branch->GetEntry(j);
                if (vec) {
                    totalSubfields += vec->size();
                }
            }
        } //     + float bramches
        else if (branchTypeName == "vector<float>") {
            std::vector<float>* vec = nullptr;
            branch->SetAddress(&vec);
            for (Long64_t j = 0; j < branch->GetEntries(); ++j) {
                branch->GetEntry(j);
                if (vec) {
                    totalSubfields += vec->size();
                }
            }
        } //     + double branches
        else if (branchTypeName == "vector<double>") {
            std::vector<double>* vec = nullptr;
            branch->SetAddress(&vec);
            for (Long64_t j = 0; j < branch->GetEntries(); ++j) {
                branch->GetEntry(j);
                if (vec) {
                    totalSubfields += vec->size();
                }
            }
        } //     + bool branches
        else if (branchTypeName == "vector<bool>") {
            std::vector<bool>* vec = nullptr;
            branch->SetAddress(&vec);
            for (Long64_t j = 0; j < branch->GetEntries(); ++j) {
                branch->GetEntry(j);
                if (vec) {
                    totalSubfields += vec->size();
                }
            }
        }
        return totalSubfields;
    }

    size_t Checker::CountSubFieldsInRNTuple(const std::string& branchName, const std::string& rntupleSubFieldType) {
        size_t numSubfields = 0;

        // Ensure the RNTupleReader is initialised
        if (!rntupleReader) {
            throw std::runtime_error("RNTupleReader pointer is null");
        }
        try {
            // Retrieve the descriptor of the RNTuple, for its information about the fields
            const auto& descriptor = rntupleReader->GetDescriptor();
            const int rntupleFieldCount = descriptor.GetNFields();

            // Create a regex pattern to match the vector type corresponding to the given subfield type
            std::string regexPattern = "std::vector<.*" + rntupleSubFieldType + ".*>";
            std::regex typeVectorRegex(regexPattern);

            // Iterate over the fields in the RNTuple, excluding the last field (a "_0" it would fail on)
            for (int i = 0; i < rntupleFieldCount - 1; ++i) {
                const auto& fieldDescriptor = descriptor.GetFieldDescriptor(i);

                // Skip the field if the branch name contains "_0"
                if (branchName.find("_0") != std::string::npos) {
                    std::cout << "Skipping field '" << branchName << "' as it contains '_0'" << std::endl;
                    break;
                }

                // Get the type name of the current field
                const std::string& fieldTypeName = fieldDescriptor.GetTypeName();

                // Check if the field type matches the vector type regex pattern
                if (std::regex_match(fieldTypeName, typeVectorRegex)) {

                    // Depending on the subfield type, get the corresponding vector view and count its elements
                    if (rntupleSubFieldType.find("int") != std::string::npos) {
                        auto fieldView = rntupleReader->GetView<std::vector<int>>(branchName);
                        for (auto entryId : *rntupleReader) {

                            // Ensure the entry ID is within valid range
                            if (entryId >= rntupleReader->GetNEntries()) {
                                throw std::out_of_range("Entry ID is out of range");
                            }

                            // Count the number of elements in the vector
                            const auto& vec = fieldView(entryId);
                            numSubfields += vec.size();
                        }
                    }
                    else if (rntupleSubFieldType.find("float") != std::string::npos) {
                        auto fieldView = rntupleReader->GetView<std::vector<float>>(branchName);
                        for (auto entryId : *rntupleReader) {
                            if (entryId >= rntupleReader->GetNEntries()) {
                                throw std::out_of_range("Entry ID is out of range");
                            }
                            const auto& vec = fieldView(entryId);
                            numSubfields += vec.size();
                        }
                    }
                    else if (rntupleSubFieldType.find("double") != std::string::npos) {
                        auto fieldView = rntupleReader->GetView<std::vector<double>>(branchName);
                        for (auto entryId : *rntupleReader) {
                            if (entryId >= rntupleReader->GetNEntries()) {
                                throw std::out_of_range("Entry ID is out of range");
                            }
                            const auto& vec = fieldView(entryId);
                            numSubfields += vec.size();
                        }
                    }
                    else if (rntupleSubFieldType.find("bool") != std::string::npos) {
                        auto fieldView = rntupleReader->GetView<std::vector<bool>>(branchName);
                        for (auto entryId : *rntupleReader) {
                            if (entryId >= rntupleReader->GetNEntries()) {
                                throw std::out_of_range("Entry ID is out of range");
                            }
                            const auto& vec = fieldView(entryId);
                            numSubfields += vec.size();
                        }
                    }
                }
            }
        }
        catch (const std::exception& e) {
            std::cerr << "Error reading int vector from RNTuple: " << e.what() << std::endl;
            throw;
        }

        // Return the total number of subfields counted
        return numSubfields;
    }

    std::vector<std::tuple<std::string, std::vector<std::string>, std::vector<std::string>, size_t, size_t>> Checker::CompareSubFields() {

        // Vector to store the results of the comparison for each branch
        std::vector<std::tuple<std::string, std::vector<std::string>, std::vector<std::string>, size_t, size_t>> subFieldComparisons;

        // Get the list of branches from the TTree
        const auto ttreeBranches = ttree->GetListOfBranches();
        const int ttreeFieldCount = ttreeBranches->GetEntries();

        // Get the descriptor of the RNTuple to access its fields
        const auto& descriptor = rntupleReader->GetDescriptor();

        // Iterate over each branch in the TTree
        for (int i = 0; i < ttreeFieldCount; ++i) {
            const auto branch = dynamic_cast<TBranch*>(ttreeBranches->At(i));
            if (!branch) {
                continue;
            }

            // Get the branch name and the type of data stored in the branch
            const std::string branchName = branch->GetName();
            const std::string ttreeType = branch->GetLeaf(branchName.c_str())->GetTypeName();

            // Skip branches that do not store vectors
            if (ttreeType.find("vector") == std::string::npos) {
                continue;
            }

            // Find the corresponding field ID in the RNTuple
            auto fieldId = descriptor.FindFieldId(branchName);
            if (fieldId == static_cast<decltype(fieldId)>(ROOT::Experimental::kInvalid)) {
                continue;
            }

            // Get the field descriptor for the matching field in the RNTuple
            const auto& fieldDescriptor = descriptor.GetFieldDescriptor(fieldId);
            const std::string rntupType = fieldDescriptor.GetTypeName();

            // Extract subfield types from the field type strings
            std::string ttreeSubFieldType = ExtractSubFieldType(ttreeType);
            std::string rntupleSubFieldType = ExtractSubFieldType(rntupType);

            // Store the subfield types in vectors for comparison
            std::vector<std::string> ttreeSubFields = { ttreeSubFieldType };
            std::vector<std::string> rntupleSubFields = { rntupleSubFieldType };

            // Count the number of subfields in both the TTree and RNTuple for the current branch
            size_t ttreeSubFieldCount = CountSubFieldsInBranch(branch, ttreeType);
            size_t rntupleSubFieldCount = CountSubFieldsInRNTuple(branchName, rntupleSubFieldType);

            // Store the results of the comparison in a tuple and add it to the results vector
            subFieldComparisons.emplace_back(branchName, ttreeSubFields, rntupleSubFields, ttreeSubFieldCount, rntupleSubFieldCount);
        }

        return subFieldComparisons;
    }


    std::vector<int> Checker::ReadIntFromTTree() {
        if (!ttree) {
            throw std::runtime_error("TTree pointer is null");
        }

        std::vector<int> intValues;

        TObjArray* branches = ttree->GetListOfBranches();
        if (!branches) {
            throw std::runtime_error("TTree has no branches");
        }

        // Iterate over each branch in the TTree
        for (int i = 0; i < branches->GetEntries(); ++i) {
            TBranch* branch = dynamic_cast<TBranch*>(branches->At(i));
            if (!branch) {
                continue;
            }

            // Get the type of the branch and check if it is of type "Int_t"
            std::string branchTypeName = branch->GetLeaf(branch->GetName())->GetTypeName();
            if (branchTypeName == "Int_t") {
                int value;
                branch->SetAddress(&value);

                // Loop over all entries in the branch and extract the integer values
                for (int j = 0; j < branch->GetEntries(); ++j) {
                    branch->GetEntry(j);
                    intValues.push_back(value);
                }
            }
        }
        return intValues;
    }

    std::vector<float> Checker::ReadFloatFromTTree() {
        if (!ttree) {
            throw std::runtime_error("TTree pointer is null");
        }

        std::vector<float> floatValues;

        TObjArray* branches = ttree->GetListOfBranches();
        if (!branches) {
            throw std::runtime_error("TTree has no branches");
        }

        // Iterate over each branch in the TTree
        for (int i = 0; i < branches->GetEntries(); ++i) {
            TBranch* branch = dynamic_cast<TBranch*>(branches->At(i));
            if (!branch) {
                continue;
            }

            // Get the type of the branch and check if it is of type "Float_t"
            std::string branchTypeName = branch->GetLeaf(branch->GetName())->GetTypeName();
            if (branchTypeName == "Float_t") {
                float value;
                branch->SetAddress(&value);

                // Loop over all entries in the branch and extract the floating-point values
                for (int j = 0; j < branch->GetEntries(); ++j) {
                    branch->GetEntry(j);
                    floatValues.push_back(value);
                }
            }
        }
        return floatValues;
    }

    std::vector<double> Checker::ReadDoubleFromTTree() {
        if (!ttree) {
            throw std::runtime_error("TTree pointer is null");
        }

        std::vector<double> doubleValues;

        TObjArray* branches = ttree->GetListOfBranches();
        if (!branches) {
            throw std::runtime_error("TTree has no branches");
        }

        // Iterate over each branch in the TTree
        for (int i = 0; i < branches->GetEntries(); ++i) {
            TBranch* branch = dynamic_cast<TBranch*>(branches->At(i));
            if (!branch) {
                continue;
            }

            // Get the type of the branch and check if it is of type "Double_t"
            std::string branchTypeName = branch->GetLeaf(branch->GetName())->GetTypeName();
            if (branchTypeName == "Double_t") {
                double value;
                branch->SetAddress(&value);

                // Loop over all entries in the branch and extract the double precision values
                for (int j = 0; j < branch->GetEntries(); ++j) {
                    branch->GetEntry(j);
                    doubleValues.push_back(value);
                }
            }
        }
        return doubleValues;
    }

    std::vector<bool> Checker::ReadBoolFromTTree() {
        if (!ttree) {
            throw std::runtime_error("TTree pointer is null");
        }

        std::vector<bool> boolValues;

        TObjArray* branches = ttree->GetListOfBranches();
        if (!branches) {
            throw std::runtime_error("TTree has no branches");
        }

        // Iterate over each branch in the TTree
        for (int i = 0; i < branches->GetEntries(); ++i) {
            TBranch* branch = dynamic_cast<TBranch*>(branches->At(i));
            if (!branch) {
                continue;
            }

            // Get the type of the branch and check if it is of type "Bool_t"
            std::string branchTypeName = branch->GetLeaf(branch->GetName())->GetTypeName();
            if (branchTypeName == "Bool_t") {
                bool value;
                branch->SetAddress(&value);

                // Loop over all entries in the branch and extract the boolean values
                for (int j = 0; j < branch->GetEntries(); ++j) {
                    branch->GetEntry(j);
                    boolValues.push_back(value);
                }
            }
        }
        return boolValues;
    }

    std::vector<int> Checker::ReadIntFromRNTuple() {
        std::vector<int> intVector;

        if (!rntupleReader) {
            throw std::runtime_error("RNTupleReader pointer is null");
        }

        try {
            const auto& descriptor = rntupleReader->GetDescriptor();
            const int rntupleFieldCount = descriptor.GetNFields();

            // Iterate over all fields in the RNTuple to find int fields
            for (int i = 0; i < rntupleFieldCount - 1; ++i) {
                const auto& fieldDescriptor = descriptor.GetFieldDescriptor(i);
                const std::string& fieldName = fieldDescriptor.GetFieldName();
                const std::string& fieldTypeName = fieldDescriptor.GetTypeName();

                // Check if the field is of type "int"
                if (fieldTypeName.find("int") != std::string::npos) {
                    auto fieldView = rntupleReader->GetView<int>(fieldName);

                    // Iterate through all entries and store the values
                    for (auto entryId : *rntupleReader) {
                        intVector.push_back(fieldView(entryId));
                    }
                }
            }
        }
        catch (const std::exception& e) {
            std::cerr << "Error reading int from RNTuple: " << e.what() << std::endl;
            throw;
        }

        return intVector;
    }

    std::vector<float> Checker::ReadFloatFromRNTuple() {
        std::vector<float> floatVector;

        if (!rntupleReader) {
            throw std::runtime_error("RNTupleReader pointer is null");
        }

        try {
            const auto& descriptor = rntupleReader->GetDescriptor();
            const int rntupleFieldCount = descriptor.GetNFields();

            // Iterate over all fields in the RNTuple to find float fields
            for (int i = 0; i < rntupleFieldCount - 1; ++i) {
                const auto& fieldDescriptor = descriptor.GetFieldDescriptor(i);
                const std::string& fieldName = fieldDescriptor.GetFieldName();
                const std::string& fieldTypeName = fieldDescriptor.GetTypeName();

                // Check if the field is of type "float"
                if (fieldTypeName == "float") {
                    auto fieldView = rntupleReader->GetView<float>(fieldName);

                    // Iterate through all entries and store the values
                    for (auto entryId : *rntupleReader) {
                        floatVector.push_back(fieldView(entryId));
                    }
                }
            }
        }
        catch (const std::exception& e) {
            std::cerr << "Error reading float from RNTuple: " << e.what() << std::endl;
            throw;
        }

        return floatVector;
    }

    std::vector<double> Checker::ReadDoubleFromRNTuple() {
        std::vector<double> doubleVector;

        if (!rntupleReader) {
            throw std::runtime_error("RNTupleReader pointer is null");
        }

        try {
            const auto& descriptor = rntupleReader->GetDescriptor();
            const int rntupleFieldCount = descriptor.GetNFields();

            // Iterate over all fields in the RNTuple to find double fields
            for (int i = 0; i < rntupleFieldCount - 1; ++i) {
                const auto& fieldDescriptor = descriptor.GetFieldDescriptor(i);
                const std::string& fieldName = fieldDescriptor.GetFieldName();
                const std::string& fieldTypeName = fieldDescriptor.GetTypeName();

                // Check if the field is of type "double"
                if (fieldTypeName == "double") {
                    auto fieldView = rntupleReader->GetView<double>(fieldName);

                    // Iterate through all entries and store the values
                    for (auto entryId : *rntupleReader) {
                        doubleVector.push_back(fieldView(entryId));
                    }
                }
            }
        }
        catch (const std::exception& e) {
            std::cerr << "Error reading double from RNTuple: " << e.what() << std::endl;
            throw;
        }

        return doubleVector;
    }

    std::vector<bool> Checker::ReadBoolFromRNTuple() {
        std::vector<bool> boolVector;

        if (!rntupleReader) {
            throw std::runtime_error("RNTupleReader pointer is null");
        }

        try {
            const auto& descriptor = rntupleReader->GetDescriptor();
            const int rntupleFieldCount = descriptor.GetNFields();

            // Iterate over all fields in the RNTuple to find bool fields
            for (int i = 0; i < rntupleFieldCount - 1; ++i) {
                const auto& fieldDescriptor = descriptor.GetFieldDescriptor(i);
                const std::string& fieldName = fieldDescriptor.GetFieldName();
                const std::string& fieldTypeName = fieldDescriptor.GetTypeName();

                // Check if the field is of type "bool"
                if (fieldTypeName == "bool") {
                    auto fieldView = rntupleReader->GetView<bool>(fieldName);

                    // Iterate through all entries and store the values
                    for (auto entryId : *rntupleReader) {
                        boolVector.push_back(fieldView(entryId));
                    }
                }
            }
        }
        catch (const std::exception& e) {
            std::cerr << "Error reading bool from RNTuple: " << e.what() << std::endl;
            throw;
        }

        return boolVector;
    }

    std::vector<int> Checker::ReadIntVectorFromTTree() {
        // Ensure the TTree pointer is not null
        if (!ttree) {
            throw std::runtime_error("TTree pointer is null");
        }

        std::vector<int> intVector;

        // Get the list of branches from the TTree
        TObjArray* branches = ttree->GetListOfBranches();
        if (!branches) {
            throw std::runtime_error("TTree has no branches");
        }

        // Iterate over each branch in the TTree
        for (int i = 0; i < branches->GetEntries(); ++i) {
            TBranch* branch = dynamic_cast<TBranch*>(branches->At(i));
            if (!branch) {
                continue;
            }

            // Check if the branch is of type vector<int>
            std::string branchTypeName = branch->GetLeaf(branch->GetName())->GetTypeName();
            if (branchTypeName == "vector<int>") {
                std::vector<int>* vec = nullptr;
                branch->SetAddress(&vec);

                int totalSubfields = 0;  // to count elements in vector

                // Iterate over all entries for the current branch
                for (Long64_t j = 0; j < branch->GetEntries(); ++j) {
                    branch->GetEntry(j);
                    if (vec) {
                        totalSubfields += vec->size(); // count number of elements in vector for this entry

                        // Append the contents of vec to the combined vector
                        intVector.insert(intVector.end(), vec->begin(), vec->end());
                    }
                }
            }
        }

        return intVector;
    }

    std::vector<float> Checker::ReadFloatVectorFromTTree() {
        // Ensure the TTree pointer is not null
        if (!ttree) {
            throw std::runtime_error("TTree pointer is null");
        }

        std::vector<float> floatVector;

        // Retrieve the list of branches from the TTree
        TObjArray* branches = ttree->GetListOfBranches();
        if (!branches) {
            throw std::runtime_error("TTree has no branches");
        }

        // Iterate over each branch in the TTree
        for (int i = 0; i < branches->GetEntries(); ++i) {
            TBranch* branch = dynamic_cast<TBranch*>(branches->At(i));
            if (!branch) {
                continue;
            }

            // Check if the branch is of type vector<float>
            std::string branchTypeName = branch->GetLeaf(branch->GetName())->GetTypeName();
            if (branchTypeName == "vector<float>") {
                std::vector<float>* vec = nullptr;
                branch->SetAddress(&vec);

                // Iterate over all entries for the current branch
                for (int j = 0; j < branch->GetEntries(); ++j) {
                    branch->GetEntry(j);
                    if (vec) {
                        // Append the contents of vec to the combined vector
                        floatVector.insert(floatVector.end(), vec->begin(), vec->end());
                    }
                }
            }
        }
        return floatVector;
    }

    std::vector<double> Checker::ReadDoubleVectorFromTTree() {
        // Ensure the TTree pointer is not null
        if (!ttree) {
            throw std::runtime_error("TTree pointer is null");
        }

        std::vector<double> doubleVector;

        // Retrieve the list of branches from the TTree
        TObjArray* branches = ttree->GetListOfBranches();
        if (!branches) {
            throw std::runtime_error("TTree has no branches");
        }

        // Iterate over each branch in the TTree
        for (int i = 0; i < branches->GetEntries(); ++i) {
            TBranch* branch = dynamic_cast<TBranch*>(branches->At(i));
            if (!branch) {
                continue;
            }

            // Check if the branch is of type vector<double>
            std::string branchTypeName = branch->GetLeaf(branch->GetName())->GetTypeName();
            if (branchTypeName == "vector<double>") {
                std::vector<double>* vec = nullptr;
                branch->SetAddress(&vec);

                // Iterate over all entries for the current branch
                for (int j = 0; j < branch->GetEntries(); ++j) {
                    branch->GetEntry(j);
                    if (vec) {
                        // Append the contents of vec to the combined vector
                        doubleVector.insert(doubleVector.end(), vec->begin(), vec->end());
                    }
                }
            }
        }
        return doubleVector;
    }

    std::vector<bool> Checker::ReadBoolVectorFromTTree() {
        // Ensure the TTree pointer is not null
        if (!ttree) {
            throw std::runtime_error("TTree pointer is null");
        }
        std::vector<bool> boolVector;

        // Retrieve the list of branches from the TTree
        TObjArray* branches = ttree->GetListOfBranches();
        if (!branches) {
            throw std::runtime_error("TTree has no branches");
        }

        // Iterate over each branch in the TTree
        for (int i = 0; i < branches->GetEntries(); ++i) {
            TBranch* branch = dynamic_cast<TBranch*>(branches->At(i));
            if (!branch) {
                continue;
            }

            // Check if the branch is of type vector<bool>
            std::string branchTypeName = branch->GetLeaf(branch->GetName())->GetTypeName();
            if (branchTypeName == "vector<bool>") {
                std::vector<bool>* vec = nullptr;
                branch->SetAddress(&vec);

                // Iterate over all entries for the current branch
                for (int j = 0; j < branch->GetEntries(); ++j) {
                    branch->GetEntry(j);
                    if (vec) {
                        // Append the contents of vec to the combined vector
                        boolVector.insert(boolVector.end(), vec->begin(), vec->end());
                    }
                }
            }
        }
        return boolVector;
    }

    std::vector<int> Checker::ReadIntVectorFromRNTuple() {
        if (!rntupleReader) {
            throw std::runtime_error("RNTupleReader pointer is null");
        }

        std::vector<int> intVector;

        try {
            const auto& descriptor = rntupleReader->GetDescriptor();
            const int rntupleFieldCount = descriptor.GetNFields();
            size_t numSubfields = 0;

            // Regex to match forms of integer vector types
            std::regex intVectorRegex(R"(std::vector<.*int.*>)");

            for (int i = 0; i < rntupleFieldCount - 1; ++i) {
                const auto& fieldDescriptor = descriptor.GetFieldDescriptor(i);
                const std::string& fieldName = fieldDescriptor.GetFieldName();
                const std::string& fieldTypeName = fieldDescriptor.GetTypeName();

                // Check if field type matches a vector of integers
                if (std::regex_match(fieldTypeName, intVectorRegex)) {

                    // Create a view for the vector<int> field
                    auto fieldView = rntupleReader->GetView<std::vector<int>>(fieldName);

                    // Iterate over all entries
                    for (auto entryId : *rntupleReader) {
                        if (entryId >= rntupleReader->GetNEntries()) {
                            throw std::out_of_range("Entry ID is out of range");
                        }

                        // vector<int> for the current entry
                        const auto& vec = fieldView(entryId);

                        intVector.insert(intVector.end(), vec.begin(), vec.end());

                        numSubfields += vec.size();
                    }
                }
                else {
                }
            }
        }
        catch (const std::exception& e) {
            std::cerr << "Error reading int vector from RNTuple: " << e.what() << std::endl;
            throw;
        }

        return intVector;
    }

    std::vector<float> Checker::ReadFloatVectorFromRNTuple() {
        if (!rntupleReader) {
            throw std::runtime_error("RNTupleReader pointer is null");
        }

        std::vector<float> floatVector;

        try {
            const auto& descriptor = rntupleReader->GetDescriptor();
            const float rntupleFieldCount = descriptor.GetNFields();

            // Regex to match forms of float vector types
            std::regex floatVectorRegex(R"(std::vector<.*float.*>)");

            for (int i = 0; i < rntupleFieldCount - 1; ++i) {
                const auto& fieldDescriptor = descriptor.GetFieldDescriptor(i);
                const std::string& fieldName = fieldDescriptor.GetFieldName();
                const std::string& fieldTypeName = fieldDescriptor.GetTypeName();

                // Check if field type matches float vector
                if (std::regex_match(fieldTypeName, floatVectorRegex)) {
                    auto fieldView = rntupleReader->GetView<std::vector<float>>(fieldName);

                    for (auto entryId : *rntupleReader) {
                        if (entryId >= rntupleReader->GetNEntries()) {
                            throw std::out_of_range("Entry ID is out of range");
                        }

                        const auto& vec = fieldView(entryId);
                        floatVector.insert(floatVector.end(), vec.begin(), vec.end());
                    }
                }
            }
        }
        catch (const std::exception& e) {
            std::cerr << "Error reading float vector from RNTuple: " << e.what() << std::endl;
            throw;
        }

        return floatVector;
    }

    std::vector<double> Checker::ReadDoubleVectorFromRNTuple() {
        if (!rntupleReader) {
            throw std::runtime_error("RNTupleReader pointer is null");
        }

        std::vector<double> doubleVector;

        try {
            const auto& descriptor = rntupleReader->GetDescriptor();
            const double rntupleFieldCount = descriptor.GetNFields();

            // Regex to match forms of double vector types
            std::regex doubleVectorRegex(R"(std::vector<.*double.*>)");

            for (int i = 0; i < rntupleFieldCount - 1; ++i) {
                const auto& fieldDescriptor = descriptor.GetFieldDescriptor(i);
                const std::string& fieldName = fieldDescriptor.GetFieldName();
                const std::string& fieldTypeName = fieldDescriptor.GetTypeName();

                // Check if field type matches double vector
                if (std::regex_match(fieldTypeName, doubleVectorRegex)) {
                    auto fieldView = rntupleReader->GetView<std::vector<double>>(fieldName);

                    for (auto entryId : *rntupleReader) {
                        if (entryId >= rntupleReader->GetNEntries()) {
                            throw std::out_of_range("Entry ID is out of range");
                        }

                        const auto& vec = fieldView(entryId);
                        doubleVector.insert(doubleVector.end(), vec.begin(), vec.end());
                    }
                }
            }
        }
        catch (const std::exception& e) {
            std::cerr << "Error reading double vector from RNTuple: " << e.what() << std::endl;
            throw;
        }

        return doubleVector;
    }

    std::vector<bool> Checker::ReadBoolVectorFromRNTuple() {
        if (!rntupleReader) {
            throw std::runtime_error("RNTupleReader pointer is null");
        }

        std::vector<bool> boolVector;

        try {
            const auto& descriptor = rntupleReader->GetDescriptor();
            const int rntupleFieldCount = descriptor.GetNFields();

            // Regex to match this exact form!
            std::regex boolVectorRegex(R"(std::vector<bool>)");

            for (int i = 0; i < rntupleFieldCount - 1; ++i) {
                const auto& fieldDescriptor = descriptor.GetFieldDescriptor(i);
                const std::string& fieldName = fieldDescriptor.GetFieldName();
                const std::string& fieldTypeName = fieldDescriptor.GetTypeName();

                // Exactly type "std::vector<bool>" to be matched
                if (std::regex_match(fieldTypeName, boolVectorRegex)) {
                    auto fieldView = rntupleReader->GetView<std::vector<bool>>(fieldName);

                    for (auto entryId : *rntupleReader) {
                        if (entryId >= rntupleReader->GetNEntries()) {
                            throw std::out_of_range("Entry ID is out of range");
                        }

                        const auto& vec = fieldView(entryId);
                        boolVector.insert(boolVector.end(), vec.begin(), vec.end());
                    }
                }
            }
        }
        catch (const std::exception& e) {
            std::cerr << "Error reading bool vector from RNTuple: " << e.what() << std::endl;
            throw;
        }
        return boolVector;
    }
} // namespace Checker
