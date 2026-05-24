#include <filesystem>
#include <fstream>
#include <cstdlib>
#include <iostream>


#include "../../src/include/types.h"
#include "../utest.h"
#include "../../src/include/json.hpp"
#include "../config_test.h"
#include "version.h"

namespace fs = std::filesystem;
using namespace kilket;
using namespace std;
using namespace nlohmann;

// ---------------------------------------------------------------------------
// Fixture
// ---------------------------------------------------------------------------
struct ConfigManagerFixture
{
    ConfigManager *cm;
    Task *t;
    Task *t2;
    Task *t3;
};

UTEST_F_SETUP(ConfigManagerFixture)
{
    setenv("KILKET_CONFIG_DIR_TEST", "/tmp/cm_test", 1);
    auto r = ConfigManager::create();
    ASSERT_TRUE(r.isOk());
    utest_fixture->cm = r.unwrap();

    fs::create_directories("/tmp/cm_test");
    fs::create_directories("/tmp/cm_test2");

    utest_fixture->t = new Task("/tmp/cm_test", "test_task", 3, {"ls"},
        {"/tmp/cm_test_file.txt"}, {"/tmp/cm_test"}, vector<string>{"ls"}, vector<string>{"ls"}, {"*.o"}, {".git"}, true, true);

    utest_fixture->t2 = new Task("test_task2", "/tmp/cm_test2", 3, {"ls"},
        {"/tmp/cm_test_file.txt"}, {"/tmp/cm_test"}, vector<string>{"ls"}, vector<string>{"cd, ls"}, {"*.o"}, {".git"}, true, true);

    utest_fixture->t3 = new Task("/tmp/cm_test", "test_task", 3, {"cd", "make"},
        {"/tmp/cm_test_file.txt"}, {"/tmp/cm_test"}, vector<string>{"ls"}, vector<string>{"cd, ls"}, {"*.o"}, {".git"}, true, true);
}

UTEST_F_TEARDOWN(ConfigManagerFixture)
{
    delete utest_fixture->cm;
    delete utest_fixture->t;
    delete utest_fixture->t2;
    delete utest_fixture->t3;
    fs::remove_all("/tmp/cm_test");
    fs::remove_all("/tmp/cm_test2");
    unsetenv("KILKET_CONFIG_DIR_TEST");
}

// --------------------------------------------------------------------------------------------
// Create
// --------------------------------------------------------------------------------------------
UTEST_F(ConfigManagerFixture, create)
{
    ASSERT_NE(utest_fixture->cm, nullptr);
    EXPECT_TRUE(fs::exists("/tmp/cm_test/config.json"));
    EXPECT_TRUE(ConfigManagerTest::get_config(utest_fixture->cm)["version"] == KILKET_VERSION);
}

// --------------------------------------------------------------------------------------------
// log task
// --------------------------------------------------------------------------------------------

UTEST_F(ConfigManagerFixture, log_task)
{
    auto c = utest_fixture->cm->log_task(*utest_fixture->t);
    EXPECT_TRUE(c.isOk());
}

UTEST_F(ConfigManagerFixture, log_multiple_tasks)
{
    auto c = utest_fixture->cm->log_task(*utest_fixture->t);
    auto c2 = utest_fixture->cm->log_task(*utest_fixture->t2);
    EXPECT_TRUE(c.isOk());
    EXPECT_TRUE(c2.isOk());
}

UTEST_F(ConfigManagerFixture, log_empty_task)
{
    auto c = utest_fixture->cm->log_task(Task());
    EXPECT_TRUE(c.isErr());
}

UTEST_F(ConfigManagerFixture, log_task_twice)
{
    EXPECT_TRUE(utest_fixture->cm->log_task(*utest_fixture->t).isOk());
    EXPECT_TRUE(utest_fixture->cm->log_task(*utest_fixture->t).isErr());
}
