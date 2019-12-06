#include "pch.h"

#include <vector>

struct Model {
    static size_t model_capacity() { return 20; }
};
namespace Total {
    const auto NA = std::numeric_limits<size_t>::max();
}

int t_low = 0;
int column_dbf_from = 0;
int column_dbf_to = 20;

std::vector<size_t> expected(const Model& model = Model()) {
    std::vector<size_t> result(model.model_capacity());
    for (size_t i = 0; i < model.model_capacity(); i++) {
        const auto t = static_cast<int>(i + t_low);
        result[i] = (column_dbf_from <= t && t <= column_dbf_to) ? i : Total::NA;
    }

    return result;
}

std::vector<size_t> row_indices(const Model& model = Model()) {
    std::vector<size_t> result(model.model_capacity(), Total::NA);
    for (size_t i = 0; i < model.model_capacity(); i++) {
        const auto t = static_cast<int>(i + t_low);
        if (column_dbf_from <= t && t <= column_dbf_to)
            result[i] = i;
    }
    return result;
}

TEST(row_indices, all) {
    for (t_low = -25; t_low < 25; ++t_low) {
        for (column_dbf_from = -30; column_dbf_from < 30; ++column_dbf_from) {
            for (column_dbf_to = column_dbf_from; column_dbf_to < 35; ++column_dbf_to) {
                EXPECT_EQ(expected(), row_indices());
            }
        }
    }
}