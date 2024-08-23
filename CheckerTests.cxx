/// \file CheckerTests.cxx
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

#include <gtest/gtest.h>
#include <TFile.h>
#include <TTree.h>
#include <ROOT/RNTuple.hxx>
#include <ROOT/RNTupleModel.hxx>
#include <ROOT/RNTupleWriteOptions.hxx>
#include <ROOT/RNTupleWriter.hxx>
#include <ROOT/RNTupleInspector.hxx>
#include "Checker.hxx"
#include <chrono>
#include <iostream>
#include <cstdio>
#include <map>
#include <variant>
#include <vector>
#include <string>

const double entryNo = 1e5;
const std::vector<std::string> fieldsbranches = { "value", "weight", "energy", "isNew" };

void createTTrees(const char* ttreeFile) {
    std::vector<std::tuple<std::string, std::variant<int*, float*, bool*, double*>, std::string, std::variant<int, float, bool, double>>> fields;

    int value = 0;
    float weight = 0.0f;
    double energy = 0.0;
    bool isNew = false;

    fields.push_back(std::make_tuple(fieldsbranches[0], &value, "value/I", 0));
    fields.push_back(std::make_tuple(fieldsbranches[1], &weight, "weight/F", 0.0f));
    fields.push_back(std::make_tuple(fieldsbranches[2], &energy, "energy/D", 0.0));
    fields.push_back(std::make_tuple(fieldsbranches[3], &isNew, "isNew/O", false));

    auto start = std::chrono::high_resolution_clock::now();

    std::remove(ttreeFile);
    auto* file = new TFile(ttreeFile, "RECREATE");

    for (int index = 0; index < 5; ++index) {
        std::string treeName = "tree_" + std::to_string(index);
        auto* tree = new TTree(treeName.c_str(), ("Tree " + std::to_string(index)).c_str());

        for (const auto& field : fields) {
            const std::string& name = std::get<0>(field);
            const std::string& branch_desc = std::get<2>(field);

            std::visit([&](auto&& arg) {
                tree->Branch(name.c_str(), arg, branch_desc.c_str());
                }, std::get<1>(field));
        }

        for (int i = 0; i < entryNo; ++i) {
            for (auto& field : fields) {
                std::visit([&](auto&& arg) {
                    using T = std::decay_t<decltype(*arg)>;
                    if constexpr (std::is_same_v<T, int>) {
                        *arg = i;
                    }
                    else if constexpr (std::is_same_v<T, float>) {
                        *arg = i * 0.1f;
                    }
                    else if constexpr (std::is_same_v<T, double>) {
                        *arg = i * 1.5;
                    }
                    else if constexpr (std::is_same_v<T, bool>) {
                        *arg = (i % 2 == 0);
                    }
                    }, std::get<1>(field));
            }
            tree->Fill();
        }

        tree->Write();
    }
    file->Close();

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> diff = end - start;
}

void createRNTuples(const char* rntupleFile) {

    auto start = std::chrono::high_resolution_clock::now();

    std::remove(rntupleFile);
    auto* file = new TFile(rntupleFile, "RECREATE");

    for (int index = 0; index < 5; ++index) {
        std::string tupleName = "rntuple_" + std::to_string(index);
        auto model = ROOT::Experimental::RNTupleModel::Create();

        if (index < 2) {
            auto fieldValue = model->MakeField<int>(fieldsbranches[0]);
            auto fieldWeight = model->MakeField<float>(fieldsbranches[1]);
            auto fieldEnergy = model->MakeField<double>(fieldsbranches[2]);
            auto fieldIsNew = model->MakeField<bool>(fieldsbranches[3]);

            const auto* options = new ROOT::Experimental::RNTupleWriteOptions();
            const auto writer = ROOT::Experimental::RNTupleWriter::Append(std::move(model), tupleName, *file, *options);

            for (int i = 0; i < entryNo; ++i) {
                if (index == 1 && i == 42) continue;
                *fieldValue = i;
                *fieldWeight = i * 0.1f;
                *fieldEnergy = i * 1.5;
                *fieldIsNew = (i % 2 == 0);
                writer->Fill();
            }
        }
        else if (index == 2) {
            auto fieldValue = model->MakeField<int>(fieldsbranches[0]);
            auto fieldEnergy = model->MakeField<double>(fieldsbranches[1]);
            auto fieldIsNew = model->MakeField<bool>(fieldsbranches[3]);

            const auto* options = new ROOT::Experimental::RNTupleWriteOptions();
            const auto writer = ROOT::Experimental::RNTupleWriter::Append(std::move(model), tupleName, *file, *options);

            for (int i = 0; i < entryNo; ++i) {
                *fieldValue = i;
                *fieldEnergy = i * 1.5;
                *fieldIsNew = (i % 2 == 0);
                writer->Fill();
            }
        }
        else if (index == 3) {
            auto fieldValue = model->MakeField<int>(fieldsbranches[0]);
            auto fieldWeight = model->MakeField<float>(fieldsbranches[1]);
            auto fieldEnergy = model->MakeField<double>("mass");
            auto fieldIsNew = model->MakeField<bool>(fieldsbranches[3]);

            const auto* options = new ROOT::Experimental::RNTupleWriteOptions();
            const auto writer = ROOT::Experimental::RNTupleWriter::Append(std::move(model), tupleName, *file, *options);

            for (int i = 0; i < entryNo; ++i) {
                *fieldValue = i;
                *fieldWeight = i * 0.1f;
                *fieldEnergy = i * 1.5;
                *fieldIsNew = (i % 2 == 0);
                writer->Fill();
            }
        }
        else if (index == 4) {
            auto fieldValue = model->MakeField<int>(fieldsbranches[0]);
            auto fieldWeight = model->MakeField<float>(fieldsbranches[1]);
            auto fieldEnergy = model->MakeField<double>(fieldsbranches[2]);
            auto fieldIsNew = model->MakeField<bool>(fieldsbranches[3]);

            const auto* options = new ROOT::Experimental::RNTupleWriteOptions();
            const auto writer = ROOT::Experimental::RNTupleWriter::Append(std::move(model), tupleName, *file, *options);

            for (int i = 0; i < entryNo; ++i) {
                *fieldValue = i;
                *fieldWeight = i * 0.1f;
                *fieldEnergy = (i % 2 == 0);
                *fieldIsNew = (i % 2 == 0);
                writer->Fill();
            }
        }
    }
    const auto inspector = ROOT::Experimental::RNTupleInspector::Create("rntuple_0", rntupleFile);
    std::regex typePattern(".*");
    auto rntupleFieldCount = inspector->GetFieldCountByType(typePattern, true);
    file->Write();
    file->Close();
    delete file;

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> diff = end - start;
}

class CheckerTest : public ::testing::Test {

protected:
    void SetUp() override {
        createTTrees(ttreeFile);
        createRNTuples(rntupleFile);
    }

    void TearDown() override {
        std::remove(ttreeFile);
        std::remove(rntupleFile);
    }

    const char* ttreeFile = "test_ttree.root";
    const char* rntupleFile = "test_rntuple.root";
};

TEST_F(CheckerTest, TTreeExists) {
    Checker::Checker checker(ttreeFile, rntupleFile, "tree_0", "rntuple_0");
    EXPECT_TRUE(checker.TTreeExists());
}

TEST_F(CheckerTest, RNTupleExists) {
    Checker::Checker checker(ttreeFile, rntupleFile, "tree_0", "rntuple_0");
    EXPECT_TRUE(checker.RNTupleExists());
}

TEST_F(CheckerTest, CountEntries) {
    Checker::Checker checker(ttreeFile, rntupleFile, "tree_0", "rntuple_0");
    auto [ttreeEntries, rntupleEntries] = checker.CountEntries();
    EXPECT_EQ(ttreeEntries, rntupleEntries);
    EXPECT_TRUE(ttreeEntries == entryNo);
    EXPECT_TRUE(rntupleEntries == entryNo);
}

TEST_F(CheckerTest, CountEntriesDif) {
    Checker::Checker checker(ttreeFile, rntupleFile, "tree_0", "rntuple_1");
    auto [ttreeEntries, rntupleEntries] = checker.CountEntries();
    EXPECT_NE(ttreeEntries, rntupleEntries);
    EXPECT_TRUE(ttreeEntries == entryNo);
    EXPECT_FALSE(rntupleEntries == entryNo);
}

TEST_F(CheckerTest, CountFields) {
    Checker::Checker checker(ttreeFile, rntupleFile, "tree_0", "rntuple_0");
    auto [ttreeFields, rntupleFields] = checker.CountFields();
    EXPECT_EQ(ttreeFields, rntupleFields);
    EXPECT_TRUE(ttreeFields == fieldsbranches.size());
    EXPECT_TRUE(rntupleFields == fieldsbranches.size());
}

TEST_F(CheckerTest, CountFieldsDif) {
    Checker::Checker checker(ttreeFile, rntupleFile, "tree_0", "rntuple_2");
    auto [ttreeFields, rntupleFields] = checker.CountFields();
    EXPECT_NE(ttreeFields, rntupleFields);
    EXPECT_TRUE(ttreeFields == fieldsbranches.size());
    EXPECT_FALSE(rntupleFields == fieldsbranches.size());
}

TEST_F(CheckerTest, CompareFieldNames) {
    Checker::Checker checker(ttreeFile, rntupleFile, "tree_0", "rntuple_0");
    auto fieldNames = checker.CompareFieldNames();
    int i = 0;
    for (const auto& [ttreeField, rntupleField] : fieldNames) {
        EXPECT_EQ(ttreeField, rntupleField);
        EXPECT_TRUE(fieldsbranches[i] == ttreeField);
        EXPECT_TRUE(fieldsbranches[i] == rntupleField);
        ++i;
    }
}

std::string NormaliseTypeName(const std::string& typeName) {
    if (typeName == "Int_t") return "std::int32_t";
    if (typeName == "Float_t") return "float";
    if (typeName == "Double_t") return "double";
    if (typeName == "Bool_t") return "bool";
    return typeName;
}

TEST_F(CheckerTest, CompareFieldTypes) {
    Checker::Checker checker(ttreeFile, rntupleFile, "tree_0", "rntuple_0");
    auto fieldTypes = checker.CompareFieldTypes();

    for (const auto& [fieldName, ttreeType, rntupleType] : fieldTypes) {
        if (ttreeType == "No match") {
            EXPECT_EQ(rntupleType, "No match");
        }
        else if (rntupleType == "No match") {
            EXPECT_EQ(ttreeType, "No match");
        }
        else {
            EXPECT_EQ(NormaliseTypeName(ttreeType), NormaliseTypeName(rntupleType))
                << "Mismatch in field '" << fieldName << "': "
                << "TTree type '" << ttreeType << "', RNTuple type '" << rntupleType << "'";
        }
    }
}

TEST_F(CheckerTest, ReadIntFromTTree) {
    Checker::Checker checker(ttreeFile, rntupleFile, "tree_0", "rntuple_0");
    std::vector<int> intValues = checker.ReadIntFromTTree();
    EXPECT_EQ(intValues.size(), entryNo);
    for (int i = 0; i < intValues.size(); ++i) {
        EXPECT_EQ(intValues[i], i);
    }
}

TEST_F(CheckerTest, ReadFloatFromTTree) {
    Checker::Checker checker(ttreeFile, rntupleFile, "tree_0", "rntuple_0");
    std::vector<float> floatValues = checker.ReadFloatFromTTree();
    EXPECT_EQ(floatValues.size(), entryNo);
    for (int i = 0; i < floatValues.size(); ++i) {
        EXPECT_FLOAT_EQ(floatValues[i], i * 0.1f);
    }
}

TEST_F(CheckerTest, ReadDoubleFromTTree) {
    Checker::Checker checker(ttreeFile, rntupleFile, "tree_0", "rntuple_0");
    std::vector<double> doubleValues = checker.ReadDoubleFromTTree();
    EXPECT_EQ(doubleValues.size(), entryNo);
    for (int i = 0; i < doubleValues.size(); ++i) {
        EXPECT_DOUBLE_EQ(doubleValues[i], i * 1.5);
    }
}

TEST_F(CheckerTest, ReadBoolFromTTree) {
    Checker::Checker checker(ttreeFile, rntupleFile, "tree_0", "rntuple_0");
    std::vector<bool> boolValues = checker.ReadBoolFromTTree();
    EXPECT_EQ(boolValues.size(), entryNo);
    for (int i = 0; i < boolValues.size(); ++i) {
        EXPECT_EQ(boolValues[i], i % 2 == 0);
    }
}

TEST_F(CheckerTest, ReadIntFromRNTuple) {
    Checker::Checker checker(ttreeFile, rntupleFile, "tree_0", "rntuple_0");
    std::vector<int> intValues = checker.ReadIntFromRNTuple();
    EXPECT_EQ(intValues.size(), entryNo);
    for (int i = 0; i < intValues.size(); ++i) {
        EXPECT_EQ(intValues[i], i);
    }
}

TEST_F(CheckerTest, ReadFloatFromRNTuple) {
    Checker::Checker checker(ttreeFile, rntupleFile, "tree_0", "rntuple_0");
    std::vector<float> floatValues = checker.ReadFloatFromRNTuple();
    EXPECT_EQ(floatValues.size(), entryNo);
    for (int i = 0; i < floatValues.size(); ++i) {
        EXPECT_FLOAT_EQ(floatValues[i], i * 0.1f);
    }
}

TEST_F(CheckerTest, ReadDoubleFromRNTuple) {
    Checker::Checker checker(ttreeFile, rntupleFile, "tree_0", "rntuple_0");
    std::vector<double> doubleValues = checker.ReadDoubleFromRNTuple();
    EXPECT_EQ(doubleValues.size(), entryNo);
    for (int i = 0; i < doubleValues.size(); ++i) {
        EXPECT_DOUBLE_EQ(doubleValues[i], i * 1.5);
    }
}

TEST_F(CheckerTest, ReadBoolFromRNTuple) {
    Checker::Checker checker(ttreeFile, rntupleFile, "tree_0", "rntuple_0");
    std::vector<bool> boolValues = checker.ReadBoolFromRNTuple();
    EXPECT_EQ(boolValues.size(), entryNo);
    for (int i = 0; i < boolValues.size(); ++i) {
        EXPECT_EQ(boolValues[i], i % 2 == 0);
    }
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
