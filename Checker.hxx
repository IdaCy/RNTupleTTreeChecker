/// \file Checker.hxx
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

#ifndef CHECKER_HXX
#define CHECKER_HXX

#include <ROOT/RNTuple.hxx>
#include <ROOT/RNTupleModel.hxx>
#include <ROOT/RNTupleReader.hxx>
#include <ROOT/RPageStorageFile.hxx>
#include <ROOT/RError.hxx>
#include <ROOT/RField.hxx>
#include <ROOT/RNTupleUtil.hxx>
#include "TBranchElement.h"

#include <TTree.h>
#include <TFile.h>
#include <TLeaf.h>
#include <TBranch.h>
#include <TKey.h>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>


 /**
  * @class Checker
  * @brief A class to compare ROOT TTrees and RNTuples for structural and data consistency.
  *
  * The Checker class provides methods to verify the existence of TTrees and RNTuples, compare their entries and fields,
  * and read data from both structures. It supports various data types and handles both scalar and vector data.
  */
namespace Checker {
    struct CheckerConfig;

    class Checker {

    public:

        /**
         * @brief Constructs a Checker object for comparing TTree and RNTuple data.
         *
         * This constructor opens the provided TTree and RNTuple files, initializes the respective data structures,
         * and checks for the existence of the specified TTree and RNTuple objects.
         *
         * @param - ttreeFile Path to the file containing the TTree.
         * @param - rntupleFile Path to the file containing the RNTuple.
         * @param - ttreeName Name of the TTree within the file.
         * @param - rntupleName Name of the RNTuple within the file.
         *
         * @throws std::runtime_error if the TTree or RNTuple cannot be found or opened.
         */
        Checker(const std::string& ttreeFile, const std::string& rntupleFile, const std::string& ttreeName, const std::string& rntupleName);

        /**
         * @brief Destructor for the Checker class.
         *
         * This destructor ensures that the TFile associated with the TTree is properly closed.
         */
        ~Checker();

        /**
         * @brief Checks if the TTree exists within the provided file.
         *
         * This function attempts to retrieve the TTree from the TFile and checks if it is valid.
         *
         * @return True if the TTree exists, otherwise false.
         */
        bool TTreeExists();

        /**
         * @brief Checks if the RNTuple exists within the provided file.
         *
         * This function iterates over the keys in the RNTuple file to determine if the specified RNTuple exists.
         *
         * @return True if the RNTuple exists, otherwise false.
         */
        bool RNTupleExists();


        /**
         * @brief Counts the number of entries in both TTree and RNTuple.
         *
         * @return A pair where the first element is the number of entries in the TTree,
         * and the second is the number of entries in the RNTuple.
         */
        std::pair<int, int> CountEntries();

        /**
         * @brief Counts the number of fields in both TTree and RNTuple.
         *
         * @return A pair where the first element is the number of fields in the TTree, and the second is the number of fields in the RNTuple.
         */
        std::pair<int, int> CountFields();

        /**
         * @brief Compares field names between TTree and RNTuple.
         *
         * This function returns a vector of pairs where each pair consists of a TTree field name and the corresponding RNTuple field name.
         * If a field in TTree does not have a match in RNTuple, "No match" is returned at the place of a name.
         *
         * The result is a vector of pairs, where each pair contains:
         * - the TTree field name
         * - the RNTuple field name.
         *
         * @return A vector of pairs comparing field names between TTree and RNTuple.
         */
        std::vector<std::pair<std::string, std::string>> CompareFieldNames();

        /**
         * @brief Compares the field types between TTree and RNTuple.
         *
         * This function iterates over the branches of the TTree and fields of the RNTuple, comparing the field types of both.
         * The result is a vector of tuples, where each tuple contains:
         * - the field name,
         * - the type from the TTree,
         * - the type from the RNTuple.
         *
         * If a field is present in the TTree but not in the RNTuple, the RNTuple type will be "No match" and vice versa.
         *
         * @return A vector of tuples where each tuple contains the field name, TTree type, and RNTuple type.
         */
        std::vector<std::tuple<std::string, std::string, std::string>> CompareFieldTypes();

        /**
         * @brief Reads integer values from the branches of a TTree.
         *
         * This function iterates over the branches of the TTree and extracts values from branches of type "Int_t".
         * The values are stored in a vector and returned.
         *
         * Note: If a branch's data is malformed, it will be skipped.
         *
         * @return A vector containing all integer values from the branches of the TTree.
         * @throws std::runtime_error If the TTree pointer is null or if there are no branches.
         */
        std::vector<int> ReadIntFromTTree();
        std::vector<float> ReadFloatFromTTree();       // same, for float
        std::vector<double> ReadDoubleFromTTree();     //       for double
        std::vector<bool> ReadBoolFromTTree();         //       for bool

    /**
     * @brief Reads integer values from fields in an RNTuple.
     *
     * This function iterates over all fields in the RNTuple, extracting integer values from fields of type "int".
     * The values are stored in a vector and returned.
     *
     * @return A vector containing all integer values from the fields of the RNTuple.
     * @throws std::runtime_error If the RNTupleReader pointer is null or if an error occurs while reading the values.
     */
        std::vector<int> ReadIntFromRNTuple();
        std::vector<float> ReadFloatFromRNTuple();      // same, for float
        std::vector<double> ReadDoubleFromRNTuple();    //       for double
        std::vector<bool> ReadBoolFromRNTuple();        //       for bool

    /**
     * @brief This function reads a vector of integers from a ROOT TTree.
     *
     * It iterates through the branches of the TTree, identifying branches that
     * contain a "vector<int>". For each identified branch, it retrieves the
     * corresponding integer vectors from all entries and accumulates them
     * into a single vector.
     *
     * @throws std::runtime_error If the TTree pointer is null or the TTree has no branches.
     * @return std::vector<int> The combined integer vector from all entries in the TTree.
     */
        std::vector<int> ReadIntVectorFromTTree();
        std::vector<float> ReadFloatVectorFromTTree();   // same, for float
        std::vector<double> ReadDoubleVectorFromTTree(); //       for double
        std::vector<bool> ReadBoolVectorFromTTree();     //       for bool

    /**
     * @brief This function reads a vector of integers from a ROOT RNTuple.
     *
     * It scans the fields of the RNTuple for any that match the "std::vector<int>" type.
     * For each matching field, it retrieves the integer vectors from all entries and
     * combines them into a single vector.
     *
     * @throws std::runtime_error If the RNTupleReader pointer is null.
     * @throws std::out_of_range If the entry ID exceeds the number of entries in the RNTuple.
     * @return std::vector<int> The combined integer vector from all entries in the RNTuple.
     */
        std::vector<int> ReadIntVectorFromRNTuple();
        std::vector<float> ReadFloatVectorFromRNTuple();  // same, for float
        std::vector<double> ReadDoubleVectorFromRNTuple();//       for double
        std::vector<bool> ReadBoolVectorFromRNTuple();    //       for bool

        /**
         * --- HELPER FUNCTION ---
         *
         * @brief Extracts the subfield type from a vector type string.
         *
         * This helper function extracts the type inside a vector, such as "int" from "vector<int>".
         *
         * @param vectorType The type string representing a vector, e.g., "vector<int>".
         * @return The subfield type inside the vector, e.g., "int".
         */
        size_t CountSubFieldsInBranch(TBranch* branch, const std::string& branchTypeName);

        /**
         * --- HELPER FUNCTION ---
         *
         * @brief Counts the number of subfields in a specific TTree branch.
         *
         * This function counts the total number of subfields in a vector branch of a TTree. It handles different vector types of "vector<int>", "vector<float>", "vector<double>", and "vector<bool>".
         *
         * @param branch Pointer to the TTree branch.
         * @param branchTypeName The type of the TTree branch, e.g., "vector<int>".
         * @return The total number of subfields within the branch.
         */
        size_t CountSubFieldsInRNTuple(const std::string& fieldName, const std::string& fieldTypeName);

        /**
         * --- HELPER FUNCTION ---
         *
         * @brief Counts the number of subfields in an RNTuple field.
         *
         * This function identifies fields of vector types within the RNTuple and counts the total number of subfields within these vectors. It supports vectors of integers, floats, doubles, and booleans.
         *
         * @param - branchName The name of the field in the RNTuple.
         * @param - rntupleSubFieldType The subfield type inside the vector, e.g., "int" from "std::vector<int>".
         *
         * @return The total number of subfields within the RNTuple field.
         */
        std::string ExtractSubFieldType(const std::string& vectorType);

        /**
         * @brief Compares subfields between vector fields in TTree and RNTuple.
         *
         * This function compares vector fields between TTree and RNTuple, counting the number of subfields in each. It returns a vector of tuples where each tuple contains:
         * - The name of the branch/field,
         * - A vector of TTree subfield types,
         * - A vector of RNTuple subfield types,
         * - The total number of subfields in the TTree,
         * - The total number of subfields in the RNTuple.
         *
         * @return A vector of tuples representing the comparison of subfields between TTree and RNTuple.
         *
         * @throws std::runtime_error if the RNTupleReader is null.
         * @throws std::out_of_range if an entry ID in the RNTuple is out of range.
         * @throws std::exception if any other error occurs during the subfield counting process.
         */
        std::vector<std::tuple<std::string, std::vector<std::string>, std::vector<std::string>, size_t, size_t>> CompareSubFields();

    private:
        std::string fTTreeFile;         // Path to the ROOT file containing the TTree
        std::string fRNTupleFile;       //                     & containing RNTuple
        std::string fTTreeName;         // Name of the TTree from ROOT file for TTree specified for check
        std::string fRNTupleName;       //           & RNTuple in ROOT file for RNTuple

        std::unique_ptr<TFile> tfile;   // Unique pointer to the ROOT file containing the TTree
        std::unique_ptr<TFile> rfile;   // Unique pointer to the ROOT file containing the RNTuple

        // TTree's and RNTuple's continued read access in Checker:
        TTree* ttree;                                                     // Pointer to the TTree
        std::unique_ptr<ROOT::Experimental::RNTupleReader> rntupleReader; // Pointer to the RNTuple reader
    };
} // namespace Checker

#endif // CHECKER_HXX
