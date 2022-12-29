#include "generators/json_generator.h"
#include "parser/parser.h"
#include "tests_configure.h"
#include "utils/project_utils.h"

#include <gtest/gtest.h>
#include <nlohmann/json.hpp>

#include <fstream>
#include <sstream>

using json = nlohmann::json;

namespace busrpc { namespace test {

TEST(JsonGeneratorTest, generate_Creates_Correct_Documentation_For_The_Test_Project)
{
    TmpDir tmp;
    CreateTestProject(tmp);
    Parser parser(tmp.path(), BUSRPC_TESTS_PROTOBUF_ROOT);
    auto [projectPtr, ecol] = parser.parse();

    ASSERT_EQ(ecol.errors().size(), 0);

    std::fstream out("aaa.json");
    out << std::setw(2);
    JsonGenerator generator(out);

    ASSERT_NO_THROW(generator.generate(projectPtr.get()));

    json doc;

    //ASSERT_NO_THROW(doc = json::parse(out.str()));

    ASSERT_TRUE(doc.contains(Errc_Enum_Name));
    EXPECT_EQ(doc[Errc_Enum_Name]["name"], Errc_Enum_Name);
}
}} // namespace busrpc::test
