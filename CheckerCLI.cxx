/// \file CheckerCLI.cxx
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
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#include "CheckerCLI.hxx"
#include "Checker.hxx"
#include <iostream>
#include <iomanip>
#include <memory>
#include <TKey.h>
#include <string>
#include <TH1.h>
#include <TCanvas.h>

namespace Checker {

    void CheckerCLI::SetVerbosity(bool verbose) {
        fVerbose = verbose;
    }

    void CheckerCLI::Compare(const CheckerConfig& config) {
        // Instantiate Checker object with given configuration
        Checker checker(config.fTTreeFile, config.fRNTupleFile, config.fTTreeName, config.fRNTupleName);

        bool output = false;
        bool methodoutput = false;

        // Compare entry counts
        methodoutput = PrintEntryComparison(checker.CountEntries());
        if (methodoutput) output = true;

        // Compare field counts
        methodoutput = PrintFieldComparison(checker.CountFields());
        if (methodoutput) output = true;

        // Compare field names
        methodoutput = PrintFieldNameComparison(checker.CompareFieldNames());
        if (methodoutput) output = true;
        methodoutput = PrintFieldTypeComparison(checker.CompareFieldTypes());
        if (methodoutput) output = true;

        // Generate histograms and gather statistics
        auto histDataTTree = HistTTree(checker.ReadIntFromTTree(), checker.ReadFloatFromTTree(),
            checker.ReadDoubleFromTTree(), checker.ReadBoolFromTTree());
        auto histDataRNTuple = HistRNTuple(checker.ReadIntFromRNTuple(), checker.ReadFloatFromRNTuple(),
            checker.ReadDoubleFromRNTuple(), checker.ReadBoolFromRNTuple());

        // Draw and print histogram statistics
        HistogramDrawStat(histDataTTree, histDataRNTuple);

        // If no inconsistencies were found, print a success message
        if (!output) {
            PrintStyled("\nCheck ran through successfully! No inconsistency found.", { CheckerCLI::GREEN }, true, true);
        }
    }

    void CheckerCLI::RunAll(const CheckerConfig& config) {
        if (config.fShouldRun) {
            // Run the comparison if the configuration flag is set
            Compare(config);
        }
    }

    void CheckerCLI::PrintStyled(const std::string& text, const std::initializer_list<std::string>& styles, bool firstLineBreak, bool secondLineBreak) {
        // Apply each style from the list to the text
        for (const auto& style : styles) {
            std::cout << style;
        }
        std::cout << text << CheckerCLI::RESET;

        // Add line breaks based on the flags
        if (firstLineBreak) {
            std::cout << std::endl;
        }
        if (secondLineBreak) {
            std::cout << std::endl;
        }
    }

    void CheckerCLI::PrintStyled(const std::string& text, const std::initializer_list<std::string>& styles, int width, bool firstLineBreak, bool secondLineBreak) {
        // Apply each style from the list to the text
        for (const auto& style : styles) {
            std::cout << style;
        }
        std::cout << std::setw(width) << std::left << text << CheckerCLI::RESET;

        // Add line breaks based on the flags
        if (firstLineBreak) {
            std::cout << std::endl;
        }
        if (secondLineBreak) {
            std::cout << std::endl;
        }
    }

    bool CheckerCLI::PrintEntryComparison(const std::pair<int, int>& entries) {
        // Skip printing if not verbose and counts match
        if (!fVerbose && entries.first == entries.second) {
            return false;
        }

        // Print the section header for entry comparison
        PrintStyled("\n*** Entry Count ***", { CheckerCLI::MEDIUM_BLUE });

        const bool compareCount = (entries.first == entries.second);
        if (compareCount) {
            // Print the number of entries if they match
            PrintStyled("Number of entries: ", { CheckerCLI::DEFAULT }, false);
            PrintStyled(std::to_string(entries.first), { CheckerCLI::GREEN });
        }
        else {
            // Print the number of entries for both TTree and RNTuple if they differ
            PrintStyled("Number of entries in TTree: ", { CheckerCLI::DEFAULT }, false);
            PrintStyled(std::to_string(entries.first), { CheckerCLI::RED });
            PrintStyled("Number of entries in RNTuple: ", { CheckerCLI::DEFAULT }, false);
            PrintStyled(std::to_string(entries.second), { CheckerCLI::RED });
        }

        // Print whether the entry counts match
        PrintStyled("TTree and RNTuple have the same entry count: ", { CheckerCLI::DEFAULT }, false);
        if (compareCount) {
            PrintStyled("TRUE", { CheckerCLI::BLACK, CheckerCLI::BG_GREEN }, true, true);
        }
        else {
            PrintStyled("FALSE", { CheckerCLI::BLACK, CheckerCLI::BG_RED }, true, true);
        }
        return true;
    }

    bool CheckerCLI::PrintFieldComparison(const std::pair<int, int>& fields) {
        // Skip printing if not verbose and counts match
        if (!fVerbose && (fields.first == fields.second)) {
            return false;
        }

        // Print the section header
        PrintStyled("*** Field Count ***", { CheckerCLI::MEDIUM_BLUE });

        const bool compareCount = fields.first == fields.second;

        if (compareCount) {
            // Print the number of fields if they match
            PrintStyled("Number of fields:  ", { CheckerCLI::DEFAULT }, false);
            PrintStyled(std::to_string(fields.first), { CheckerCLI::GREEN });
        }
        else {
            // Print the number of fields for both TTree and RNTuple if they differ
            std::cout << "Number of fields in TTree: " << fields.first << std::endl;
            std::cout << "Number of fields in RNTuple: " << fields.second << "\n" << std::endl;
        }

        // Print whether the field counts match
        PrintStyled("TTree and RNTuple have the same field count: ", { CheckerCLI::DEFAULT }, false);
        if (compareCount) {
            PrintStyled("TRUE", { CheckerCLI::BLACK, CheckerCLI::BG_GREEN }, true, true);
        }
        else {
            PrintStyled("FALSE", { CheckerCLI::BLACK, CheckerCLI::BG_RED }, true, true);
        }
        return true;
    }

    bool CheckerCLI::PrintFieldNameComparison(const std::vector<std::pair<std::string, std::string>>& fieldNames) {
        bool compareFields = true;

        // Now initial looping through already -
        // Just to determine whether there will be mismatches! non-verbose + no-mismatch -> return nothing
        for (const auto& pair : fieldNames) {
            bool isTTreeNoMatch = (pair.first == "No match");
            bool isRNTupleNoMatch = (pair.second == "No match");
            if (pair.first != pair.second) {
                compareFields = false;
            }
        }

        // Non-verbose + no mismatch is found here = nothing returned
        if (!fVerbose && compareFields) {
            return false;
        }
        // Either verbose or mismatch found -> go on...

        // Print the section header
        PrintStyled("*** Field Names ***", { CheckerCLI::MEDIUM_BLUE });

        int width = 20;
        PrintStyled(std::string("TTree Field"), { CheckerCLI::DEFAULT }, width, false);
        PrintStyled(std::string("|  "), { CheckerCLI::DEFAULT }, false);
        PrintStyled(std::string("RNTuple Field"), { CheckerCLI::DEFAULT }, width, true);
        PrintStyled(std::string("------------------------------------"), { CheckerCLI::DEFAULT }, true);

        // Print each field name comparison result
        for (const auto& pair : fieldNames) {
            bool isTTreeNoMatch = (pair.first == "No match");
            bool isRNTupleNoMatch = (pair.second == "No match");

            // Only red print in case of mismatch
            PrintStyled(pair.first, { isTTreeNoMatch ? CheckerCLI::RED : CheckerCLI::DEFAULT }, width, false);
            PrintStyled("|  ", { CheckerCLI::DEFAULT }, false);

            PrintStyled(pair.second, { isRNTupleNoMatch ? CheckerCLI::RED : CheckerCLI::DEFAULT }, width, true);

            if (pair.first != pair.second) {
                compareFields = false;
            }
        }

        // Print whether the field names match
        PrintStyled("\nThe fields have the same names: ", { CheckerCLI::DEFAULT }, false);
        if (compareFields) {
            PrintStyled("TRUE", { CheckerCLI::BLACK, CheckerCLI::BG_GREEN }, true, true);
        }
        else {
            PrintStyled("FALSE", { CheckerCLI::BLACK, CheckerCLI::BG_RED }, true, true);
        }
        return true;
    }


    bool CheckerCLI::PrintFieldTypeComparison(const std::vector<std::tuple<std::string, std::string, std::string>>& fieldTypes) {

        // Map of accepted type (left) and what it matches with (right)
        std::unordered_map<std::string, std::string> typeMap = {
            {"Int_t", "int"},
            {"std::int32_t", "int"},
            {"Float_t", "float"},
            {"float", "float"},
            {"Double_t", "double"},
            {"double", "double"},
            {"Bool_t", "bool"},
            {"bool", "bool"},
            {"std::vector<std::int32_t>", "vector<int>"},
            {"std::vector<int>", "vector<int>"},
            {"std::vector<float>", "vector<float>"},
            {"std::vector<double>", "vector<double>"},
            {"std::vector<bool>", "vector<bool>"},
            {"vector<int>", "vector<int>"},
            {"std::vector<int>", "vector<int>"},
            {"vector<float>", "vector<float>"},
            {"vector<double>", "vector<double>"},
            {"vector<bool>", "vector<bool>"}
        };
        // Map of yellow-flagged accepted type (left) and what it may go with (right)
        std::unordered_map<std::string, std::string> yellowMap = {
            {"Float_t", "double"},
            {"Float_t", "double"},
            {"float", "double"},
            {"Double_t", "float"},
            {"double", "float"},
            {"std::vector<float>", "vector<double>"},
            {"std::vector<double>", "vector<float>"},
            {"vector<float>", "vector<double>"},
            {"vector<double>", "vector<float>"}
        };

        int diffLevel = 0;
        bool missingType = false;

        // Initial looping through! - just to determine whether there will be discrepancies
        // - if non-verbose + nothing is found here: nothing returned
        for (const auto& tuple : fieldTypes) {
            std::string ttreeType = std::get<1>(tuple);
            std::string rntupType = std::get<2>(tuple);

            std::string ttreeTypeMapped = typeMap.count(ttreeType) ? typeMap[ttreeType] : "Missing";
            std::string rntupTypeMapped = typeMap.count(rntupType) ? typeMap[rntupType] : "Missing";

            if (ttreeTypeMapped == "Missing" || rntupTypeMapped == "Missing") {
                missingType = true;
            }

            if (!missingType && ttreeTypeMapped != rntupTypeMapped) {
                if ((yellowMap[ttreeTypeMapped] == rntupTypeMapped) || (ttreeTypeMapped == yellowMap[rntupTypeMapped])) {
                    diffLevel = 1;
                }
                else {
                    diffLevel = 2;
                }
            }
        }

        // Non-verbose + nothing is found here = nothing returned
        if (!fVerbose && diffLevel == 0) {
            return false;
        }

        // Something was found so the actual logic happens
        int width = 20;
        diffLevel = 0;
        PrintStyled("*** Field Types ***", { CheckerCLI::MEDIUM_BLUE }); // Print the section header

        PrintStyled(std::string("Type - TTree"), { CheckerCLI::DEFAULT }, width, false);
        PrintStyled(std::string("|  "), { CheckerCLI::DEFAULT }, false);
        PrintStyled(std::string("Type - RNTuple"), { CheckerCLI::DEFAULT }, width, false);
        PrintStyled(std::string("Field"), { CheckerCLI::DEFAULT }, width, true);
        PrintStyled(std::string("-------------------------------------"), { CheckerCLI::DEFAULT }, true);

        // Print each field type comparison result
        for (const auto& tuple : fieldTypes) {
            std::string ttreeType = std::get<1>(tuple);
            std::string rntupType = std::get<2>(tuple);

            // Using the type map, find matching data types - set "Missing" if not found"
            std::string ttreeTypeMapped = typeMap.count(ttreeType) ? typeMap[ttreeType] : "Missing";
            std::string rntupTypeMapped = typeMap.count(rntupType) ? typeMap[rntupType] : "Missing";
            if (ttreeTypeMapped == "Missing" || rntupTypeMapped == "Missing") {
                missingType = true; // Current field has a missing type
            }

            // Only printing the type red if it is "Missing"
            PrintStyled(ttreeTypeMapped, { ttreeTypeMapped == "Missing" ? CheckerCLI::RED : CheckerCLI::DEFAULT }, width, false);
            PrintStyled(std::string("|  "), { CheckerCLI::DEFAULT }, false);
            PrintStyled(rntupTypeMapped, { rntupTypeMapped == "Missing" ? CheckerCLI::RED : CheckerCLI::DEFAULT }, width, false);
            PrintStyled(std::string(std::get<0>(tuple)), { CheckerCLI::DEFAULT }, width, false);

            // Mis-matches found -> either print yellow (near match) or big red flag
            if (!missingType && ttreeTypeMapped != rntupTypeMapped) {
                if ((yellowMap[ttreeTypeMapped] == rntupTypeMapped) || (ttreeTypeMapped == yellowMap[ttreeTypeMapped])) {
                    diffLevel = 1;
                    PrintStyled("   no exact match   ", { CheckerCLI::WHITE, CheckerCLI::BG_YELLOW }, false);
                }
                else {
                    diffLevel = 2;
                    PrintStyled("   type mismatch   ", { CheckerCLI::WHITE, CheckerCLI::BG_RED }, false);
                }
            }
            std::cout << std::endl;
        }

        // Final output line - TRUE/FALSE
        if (!missingType) {
            PrintStyled("\nThe fields have the same types: ", { CheckerCLI::DEFAULT }, false);

            if (diffLevel == 0) {
                PrintStyled("TRUE", { CheckerCLI::BLACK, CheckerCLI::BG_GREEN }, true, true);
            }
            else if (diffLevel == 1) {
                PrintStyled("NOT EXACTLY", { CheckerCLI::BLACK, CheckerCLI::BG_YELLOW }, true, false);
            }
            else {
                PrintStyled("FALSE", { CheckerCLI::BLACK, CheckerCLI::BG_RED }, true, false);
            }
        }
        else {
            PrintStyled("\nField type comparison yields match failure due to unmatching fields.", { CheckerCLI::DEFAULT }, true);
        }
        return true;
    }

    void CheckerCLI::PrintVectorFromTTree(const std::vector<int>& intVector, const std::vector<double>& doubleVector, const std::vector<float>& floatVector, const std::vector<bool>& boolVector) {
        // If all vectors are empty, exit the function.
        if (intVector.empty() && floatVector.empty() && doubleVector.empty() && boolVector.empty()) {
            return;
        }

        // Print the header for TTree subfields.
        PrintStyled("*** TTree Subfields ***", { CheckerCLI::MEDIUM_BLUE });

        int width = 1; // Set default width for printing elements.


        // Print the integer vector
        std::cout << "Integer Vector:\n";
        for (size_t i = 0; i < intVector.size(); ++i) {
            // Print each element, separating with '|' if it's not the last element.
            PrintStyled(std::to_string(intVector[i]), { CheckerCLI::DEFAULT }, width, false);
            if (i < intVector.size() - 1) PrintStyled(" | ", { CheckerCLI::DEFAULT }, false);
            else PrintStyled(" ", { CheckerCLI::DEFAULT }, true, true);
        }
        // Print the float vector
        std::cout << "Float Vector:\n";
        for (size_t i = 0; i < floatVector.size(); ++i) {
            PrintStyled(std::to_string(floatVector[i]), { CheckerCLI::DEFAULT }, width, false);
            if (i < floatVector.size() - 1) PrintStyled(" | ", { CheckerCLI::DEFAULT }, false);
            else PrintStyled(" ", { CheckerCLI::DEFAULT }, true, true);
        }
        // Print the double vector
        std::cout << "Double Vector:\n";
        for (size_t i = 0; i < doubleVector.size(); ++i) {
            PrintStyled(std::to_string(doubleVector[i]), { CheckerCLI::DEFAULT }, width, false);
            if (i < doubleVector.size() - 1) PrintStyled(" | ", { CheckerCLI::DEFAULT }, false);
            else PrintStyled(" ", { CheckerCLI::DEFAULT }, true, true);
        }
        // Print the boolean vector
        std::cout << "Bool Vector:\n";
        for (size_t i = 0; i < boolVector.size(); ++i) {
            PrintStyled(std::to_string(boolVector[i]), { CheckerCLI::DEFAULT }, width, false);
            if (i < boolVector.size() - 1) PrintStyled(" | ", { CheckerCLI::DEFAULT }, false);
            else PrintStyled(" ", { CheckerCLI::DEFAULT }, true, true);
        }
    }

    void CheckerCLI::PrintVectorFromRNTuple(const std::vector<int>& intVector, const std::vector<float>& floatVector, const std::vector<double>& doubleVector, const std::vector<bool>& boolVector) {
        // If all vectors are empty, exit the function
        if (intVector.empty() && floatVector.empty() && doubleVector.empty() && boolVector.empty()) {
            return;
        }

        // Print the header for RNTuple subfields
        PrintStyled("*** RNTuple Subfields ***", { CheckerCLI::MEDIUM_BLUE });

        int width = 1; // Set default width for printing elements

        // Print the integer vector
        std::cout << "Integer Vector:\n";
        // Print each element, separating with '|' if it's not the last element.
        for (size_t i = 0; i < intVector.size(); ++i) {
            PrintStyled(std::to_string(intVector[i]), { CheckerCLI::DEFAULT }, width, false);
            if (i < intVector.size() - 1) PrintStyled(" | ", { CheckerCLI::DEFAULT }, false);
            else PrintStyled(" ", { CheckerCLI::DEFAULT }, true, true);
        }
        // Print the float vector
        std::cout << "Float Vector:\n";
        for (size_t i = 0; i < floatVector.size(); ++i) {
            PrintStyled(std::to_string(floatVector[i]), { CheckerCLI::DEFAULT }, width, false);
            if (i < floatVector.size() - 1) PrintStyled(" | ", { CheckerCLI::DEFAULT }, false);
            else PrintStyled(" ", { CheckerCLI::DEFAULT }, true, true);
        }
        // Print the double vector
        std::cout << "Double Vector:\n";
        for (size_t i = 0; i < doubleVector.size(); ++i) {
            PrintStyled(std::to_string(doubleVector[i]), { CheckerCLI::DEFAULT }, width, false);
            if (i < doubleVector.size() - 1) PrintStyled(" | ", { CheckerCLI::DEFAULT }, false);
            else PrintStyled(" ", { CheckerCLI::DEFAULT }, true, true);
        }
        // Print the boolean vector
        std::cout << "Bool Vector:\n";
        for (size_t i = 0; i < boolVector.size(); ++i) {
            PrintStyled(std::to_string(boolVector[i]), { CheckerCLI::DEFAULT }, width, false);
            if (i < boolVector.size() - 1) PrintStyled(" | ", { CheckerCLI::DEFAULT }, false);
            else PrintStyled(" ", { CheckerCLI::DEFAULT }, true, true);
        }
    }

    void CheckerCLI::IntHist_ChiSquareComparison(const std::vector<int>& ttreeVector, const std::vector<int>& rntupleVector) {
        // If either vector is empty, exit the function.
        if (ttreeVector.empty() || rntupleVector.empty()) {
            return;
        }

        // Print the header for histograms
        PrintStyled("*** Histograms ***", { CheckerCLI::MEDIUM_BLUE });

        // Determine the minimum and maximum values between both vectors for histogram range
        int minValue = std::min(*std::min_element(ttreeVector.begin(), ttreeVector.end()),
            *std::min_element(rntupleVector.begin(), rntupleVector.end()));
        int maxValue = std::max(*std::max_element(ttreeVector.begin(), ttreeVector.end()),
            *std::max_element(rntupleVector.begin(), rntupleVector.end()));

        int bins = 100;

        // Create histograms for TTree and RNTuple data
        TH1I* ttreeHist = new TH1I("TTree Histogram", "TTree Data Distribution", bins, minValue, maxValue);
        TH1I* rntupleHist = new TH1I("RNTuple Histogram", "RNTuple Data Distribution", bins, minValue, maxValue);

        // Fill histograms with respective data
        for (const auto& val : ttreeVector) {
            ttreeHist->Fill(val);
        }
        for (const auto& val : rntupleVector) {
            rntupleHist->Fill(val);
        }

        // Draw histograms on same canvas - different colours
        TCanvas* canvas1 = new TCanvas("canvas1", "Histogram Comparison", 800, 600);
        ttreeHist->SetLineColor(kRed);
        rntupleHist->SetLineColor(kBlue);
        ttreeHist->Draw();
        rntupleHist->Draw("SAME");

        // Save the canvas as a PNG image
        canvas1->SaveAs("comparison_int.png");

        // Perform Chi-square test to compare histograms
        double chiSquare = ttreeHist->Chi2Test(rntupleHist, "CHI2");
        int chiSquareDisplayValue = (chiSquare == 0) ? 0 : 1;
        // Print Chi-square result, using color to indicate if there's a match
        PrintStyled("ChiSquare Value ", { CheckerCLI::DEFAULT }, false);
        PrintStyled(" " + std::to_string(chiSquareDisplayValue) + " ", { (chiSquare == 0) ? CheckerCLI::BG_GREEN : CheckerCLI::RED }, true, true);
    }

    std::vector<std::tuple<int, double, double>> CheckerCLI::HistTTree(const std::vector<int>& intData,
        const std::vector<float>& floatData,
        const std::vector<double>& doubleData,
        const std::vector<bool>& boolData) {

        // Create a canvas divided into 4 sections for each data type.
        TCanvas* canvas2 = new TCanvas("TTree_Combined_Canvas", "TTree Combined Histogram", 1200, 800);
        canvas2->Divide(2, 2);

        std::vector<std::tuple<int, double, double>> statvals;

        // Create and display histogram for integer data
        canvas2->cd(1);
        if (!intData.empty()) {
            TH1I* hist = new TH1I("TTree_Int_Hist", "TTree Int Histogram;Value;Entries",
                100, *std::min_element(intData.begin(), intData.end()),
                *std::max_element(intData.begin(), intData.end()));
            for (auto val : intData) {
                hist->Fill(val);
            }
            hist->SetLineColor(kRed);
            hist->Draw();

            statvals.push_back({ static_cast<int>(hist->GetEntries()), hist->GetMean(), hist->GetStdDev() });
        }
        else {
            statvals.push_back({ 0, 0.0, 0.0 });
        }

        // Float Data Histogram
        canvas2->cd(2);
        if (!floatData.empty()) {
            TH1F* hist = new TH1F("TTree_Float_Hist", "TTree Float Histogram;Value;Entries",
                100, *std::min_element(floatData.begin(), floatData.end()),
                *std::max_element(floatData.begin(), floatData.end()));
            for (auto val : floatData) {
                hist->Fill(val);
            }
            hist->SetLineColor(kBlue);
            hist->Draw();

            statvals.push_back({ static_cast<int>(hist->GetEntries()), hist->GetMean(), hist->GetStdDev() });
        }
        else {
            statvals.push_back({ 0, 0.0, 0.0 });
        }

        // Double Data Histogram
        canvas2->cd(3);
        if (!doubleData.empty()) {
            TH1D* hist = new TH1D("TTree_Double_Hist", "TTree Double Histogram;Value;Entries",
                100, *std::min_element(doubleData.begin(), doubleData.end()),
                *std::max_element(doubleData.begin(), doubleData.end()));
            for (auto val : doubleData) {
                hist->Fill(val);
            }
            hist->SetLineColor(kGreen);
            hist->Draw();

            statvals.push_back({ static_cast<int>(hist->GetEntries()), hist->GetMean(), hist->GetStdDev() });
        }
        else {
            statvals.push_back({ 0, 0.0, 0.0 });
        }

        // Bool Data Histogram
        canvas2->cd(4);
        if (!boolData.empty()) {
            TH1I* hist = new TH1I("TTree_Bool_Hist", "TTree Bool Histogram;Value;Entries",
                2, 0, 2);
            for (auto val : boolData) {
                hist->Fill(val);
            }
            hist->SetLineColor(kMagenta);
            hist->Draw();

            statvals.push_back({ static_cast<int>(hist->GetEntries()), hist->GetMean(), hist->GetStdDev() });
        }
        else {
            statvals.push_back({ 0, 0.0, 0.0 });
        }

        canvas2->SaveAs("TTree_Combined_Histogram.png");

        return statvals;
    }

    std::vector<std::tuple<int, double, double>> CheckerCLI::HistRNTuple(const std::vector<int>& intData,
        const std::vector<float>& floatData,
        const std::vector<double>& doubleData,
        const std::vector<bool>& boolData) {

        // Create a canvas divided into 4 sections for each data type.
        TCanvas* canvas3 = new TCanvas("RNTuple_Combined_Canvas", "RNTuple Combined Histogram", 1200, 800);
        canvas3->Divide(2, 2);

        std::vector<std::tuple<int, double, double>> statvals;

        // Create and display histogram for integer data
        canvas3->cd(1);
        if (!intData.empty()) {
            TH1I* hist = new TH1I("RNTuple_Int_Hist", "RNTuple Int Histogram;Value;Entries",
                100, *std::min_element(intData.begin(), intData.end()),
                *std::max_element(intData.begin(), intData.end()));
            for (auto val : intData) {
                hist->Fill(val);
            }
            hist->SetLineColor(kRed);
            hist->Draw();

            statvals.push_back({ static_cast<int>(hist->GetEntries()), hist->GetMean(), hist->GetStdDev() });
        }
        else {
            statvals.push_back({ 0, 0.0, 0.0 });
        }

        // Float Data Histogram
        canvas3->cd(2);
        if (!floatData.empty()) {
            TH1F* hist = new TH1F("RNTuple_Float_Hist", "RNTuple Float Histogram;Value;Entries",
                100, *std::min_element(floatData.begin(), floatData.end()),
                *std::max_element(floatData.begin(), floatData.end()));
            for (auto val : floatData) {
                hist->Fill(val);
            }
            hist->SetLineColor(kBlue);
            hist->Draw();

            statvals.push_back({ static_cast<int>(hist->GetEntries()), hist->GetMean(), hist->GetStdDev() });
        }
        else {
            statvals.push_back({ 0, 0.0, 0.0 });
        }

        // Double Data Histogram
        canvas3->cd(3);
        if (!doubleData.empty()) {
            TH1D* hist = new TH1D("RNTuple_Double_Hist", "RNTuple Double Histogram;Value;Entries",
                100, *std::min_element(doubleData.begin(), doubleData.end()),
                *std::max_element(doubleData.begin(), doubleData.end()));
            for (auto val : doubleData) {
                hist->Fill(val);
            }
            hist->SetLineColor(kGreen);
            hist->Draw();

            statvals.push_back({ static_cast<int>(hist->GetEntries()), hist->GetMean(), hist->GetStdDev() });
        }
        else {
            statvals.push_back({ 0, 0.0, 0.0 });
        }

        // Bool Data Histogram
        canvas3->cd(4);
        if (!boolData.empty()) {
            TH1I* hist = new TH1I("RNTuple_Bool_Hist", "RNTuple Bool Histogram;Value;Entries",
                2, 0, 2);
            for (auto val : boolData) {
                hist->Fill(val);
            }
            hist->SetLineColor(kMagenta);
            hist->Draw();

            statvals.push_back({ static_cast<int>(hist->GetEntries()), hist->GetMean(), hist->GetStdDev() });
        }
        else {
            statvals.push_back({ 0, 0.0, 0.0 });
        }

        // Save the combined canvas as a PNG image
        canvas3->SaveAs("RNTuple_Combined_Histogram.png");

        return statvals;
    }

    void CheckerCLI::HistogramDrawStat(const std::vector<std::tuple<int, double, double>>& dataT,
        // If verbosity is disabled, exit the function without doing anything.
        const std::vector<std::tuple<int, double, double>>& dataR) {
        if (!fVerbose) {
            return;
        }

        // Flags to track valid comparisons made and if all data matched.
        bool anyValidComparison = false;
        bool allMatched = true;
        int width = 15; // Width for formatting the output display

        // Lambda function to print a comparison table for each data type.
        auto PrintComparisonTable = [&](const std::string& dataType, int countTTree, double meanTTree, double stddevTTree,
            int countRNTuple, double meanRNTuple, double stddevRNTuple) {
                // Flags to detect mismatches in count, mean, and standard deviation - here to reset for each comparison
                bool countMismatch = countTTree != countRNTuple;
                bool meanMismatch = meanTTree != meanRNTuple;
                bool stddevMismatch = stddevTTree != stddevRNTuple;

                // If either TTree or RNTuple has data, proceed with comparison
                if (countTTree > 0 || countRNTuple > 0) {
                    if (!anyValidComparison) {
                        // Print header once before any valid comparison is displayed
                        PrintStyled("\n*** Histograms ***", { CheckerCLI::MEDIUM_BLUE }, true, true);
                        PrintStyled(" ", { CheckerCLI::DEFAULT }, width, false, false);
                        PrintStyled("| ", { CheckerCLI::DEFAULT }, false, false);
                        PrintStyled("TTree Value", { CheckerCLI::DEFAULT }, width, false, false);
                        PrintStyled("RNTuple Value", { CheckerCLI::DEFAULT }, width, true);
                        PrintStyled(std::string("---------------------------------------------"), { CheckerCLI::DEFAULT }, true);
                        anyValidComparison = true; // Set the flag indicating a valid comparison was made
                    }

                    // Update the flag if any mismatch is detected
                    if (countMismatch || meanMismatch || stddevMismatch) {
                        allMatched = false;
                    }

                    // Print count comparison for the data type
                    PrintStyled(dataType + " Count", { CheckerCLI::DEFAULT }, width, false, false);
                    PrintStyled(std::string("|  "), { CheckerCLI::DEFAULT }, false, false);
                    PrintStyled(std::to_string(countTTree), { countMismatch ? CheckerCLI::RED : CheckerCLI::DEFAULT }, width, false, false);
                    PrintStyled(std::to_string(countRNTuple), { countMismatch ? CheckerCLI::RED : CheckerCLI::DEFAULT }, width, false, true);

                    // Print means
                    PrintStyled(dataType + " Mean", { CheckerCLI::DEFAULT }, width, false, false);
                    PrintStyled(std::string("|  "), { CheckerCLI::DEFAULT }, false);
                    PrintStyled(std::to_string(meanTTree), { meanMismatch ? CheckerCLI::RED : CheckerCLI::DEFAULT }, width, false, false);
                    PrintStyled(std::to_string(meanRNTuple), { meanMismatch ? CheckerCLI::RED : CheckerCLI::DEFAULT }, width, false, true);

                    // Print standard deviations
                    PrintStyled(dataType + " StdDev", { CheckerCLI::DEFAULT }, width, false, false);
                    PrintStyled(std::string("|  "), { CheckerCLI::DEFAULT }, false);
                    PrintStyled(std::to_string(stddevTTree), { stddevMismatch ? CheckerCLI::RED : CheckerCLI::DEFAULT }, width, false, false);
                    PrintStyled(std::to_string(stddevRNTuple), { stddevMismatch ? CheckerCLI::RED : CheckerCLI::DEFAULT }, width, true, true);
                }
            };

        // Ensure that the data vectors are aligned (both have the same size) and have exactly 4 elements
        if (dataT.size() == dataR.size() && dataT.size() == 4) {
            // Extract and compare statistics for each data type: Int, Float, Double, and Bool

            // Integer data comparison
            auto [countIntTTree, meanIntTTree, stddevIntTTree] = dataT[0];
            auto [countIntRNTuple, meanIntRNTuple, stddevIntRNTuple] = dataR[0];
            PrintComparisonTable("Int", countIntTTree, meanIntTTree, stddevIntTTree, countIntRNTuple, meanIntRNTuple, stddevIntRNTuple);

            // Float
            auto [countFloatTTree, meanFloatTTree, stddevFloatTTree] = dataT[1];
            auto [countFloatRNTuple, meanFloatRNTuple, stddevFloatRNTuple] = dataR[1];
            PrintComparisonTable("Float", countFloatTTree, meanFloatTTree, stddevFloatTTree, countFloatRNTuple, meanFloatRNTuple, stddevFloatRNTuple);

            // Double
            auto [countDoubleTTree, meanDoubleTTree, stddevDoubleTTree] = dataT[2];
            auto [countDoubleRNTuple, meanDoubleRNTuple, stddevDoubleRNTuple] = dataR[2];
            PrintComparisonTable("Double", countDoubleTTree, meanDoubleTTree, stddevDoubleTTree, countDoubleRNTuple, meanDoubleRNTuple, stddevDoubleRNTuple);

            // Bool
            auto [countBoolTTree, meanBoolTTree, stddevBoolTTree] = dataT[3];
            auto [countBoolRNTuple, meanBoolRNTuple, stddevBoolRNTuple] = dataR[3];
            PrintComparisonTable("Bool", countBoolTTree, meanBoolTTree, stddevBoolTTree, countBoolRNTuple, meanBoolRNTuple, stddevBoolRNTuple);

            // After all comparisons, output whether all statistics match across TTree and RNTuple
            PrintStyled("\nAll histogram statistics match: ", { CheckerCLI::DEFAULT }, false);
            if (allMatched) {
                // If all statistics match, print "TRUE" in green
                PrintStyled("TRUE", { CheckerCLI::BLACK, CheckerCLI::BG_GREEN }, true, true);
            }
            else {
                // If any statistic does not match, print "FALSE" in red
                PrintStyled("FALSE", { CheckerCLI::BLACK, CheckerCLI::BG_RED }, true, true);
            }
        }
        else {
            // If the data vectors are not aligned or have unexpected sizes, print an error message
            PrintStyled("Error: Data vectors are not aligned or have unexpected sizes.", { CheckerCLI::RED }, true, true);
        }
    }
} // namespace Checker
