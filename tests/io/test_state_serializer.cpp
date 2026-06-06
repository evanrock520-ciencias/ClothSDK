#include "gtest/gtest.h"
#include "io/StateSerializer.hpp"
#include <filesystem>

using namespace Tissu;
namespace fs = std::filesystem;

class StateSerializerTest : public ::testing::Test {
protected:
    void SetUp() override {
        m_tempFile = fs::temp_directory_path() / "tissu_test_config.json";
    }

    void TearDown() override {
        if (fs::exists(m_tempFile))
            fs::remove(m_tempFile);
    }

    fs::path m_tempFile;
};