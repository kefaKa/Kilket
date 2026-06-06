#include <cstdlib>
#include <filesystem>


#include "../../src/include/kilket_core.h"
#include "../utest.h"
#include "../../src/include/types.h"

using namespace std;
using namespace kilket;
namespace fs = std::filesystem;

bool KILKET_DEBUG = false;
bool KILKET_VERBOSE = false;
bool KILKET_QUIET = false;


struct KilketFixture
{
    KilketCore* fh;
};

UTEST_F_SETUP(KilketFixture)
{
    fs::create_directories("/tmp/fh_test");
    fs::create_directories("/tmp/fh_test2");
    fs::create_directories("/tmp/fh_test/task_test");
    setenv("KILKET_CONFIG_DIR_TEST", "/tmp/fh_test", 1);

    auto f = KilketCore::create();
    ASSERT_TRUE(f.isOk());
    utest_fixture->fh = f.unwrap();
}

UTEST_F_TEARDOWN(KilketFixture)
{
    delete utest_fixture->fh;
    fs::remove_all("/tmp/fh_test");
    unsetenv("KILKET_CONFIG_DIR_TEST");
}

// ---------------------------------------------------------------------------------
// CREATE / DELELTE TASK
// ---------------------------------------------------------------------------------

UTEST_F(KilketFixture, create_task)
{
    EXPECT_TRUE(utest_fixture->fh->create_task("test_task", "/tmp/fh_test").isOk());
}
UTEST_F(KilketFixture, create_empty_task)
{
    EXPECT_TRUE(utest_fixture->fh->create_task("", "/tmp/fh_test").isErr());
}
UTEST_F(KilketFixture, create_task_twice)
{
    ASSERT_TRUE(utest_fixture->fh->create_task("test_task", "/tmp/fh_test").isOk());
    EXPECT_TRUE(utest_fixture->fh->create_task("test_task", "/tmp/fh_test").isErr());
}
UTEST_F(KilketFixture, create_task_with_invalid_path)
{
    EXPECT_TRUE(utest_fixture->fh->create_task("test_task", "/tmp/fh_test/non_existent").isErr());
}
UTEST_F(KilketFixture, delete_task)
{
    ASSERT_TRUE(utest_fixture->fh->create_task("test_task", "/tmp/fh_test").isOk());
    auto r = utest_fixture->fh->delete_task("/tmp/fh_test");
    EXPECT_TRUE(r.isOk());
    if(r.isErr()) cout << r.getErrMessage() << endl;
}
UTEST_F(KilketFixture, delete_non_existent_task)
{
    EXPECT_TRUE(utest_fixture->fh->delete_task("non_existent_task").isErr());
}
UTEST_F(KilketFixture, delete_task_twice)
{
    ASSERT_TRUE(utest_fixture->fh->create_task("test_task", "/tmp/fh_test").isOk());
    EXPECT_TRUE(utest_fixture->fh->delete_task("/tmp/fh_test").isOk());
    EXPECT_TRUE(utest_fixture->fh->delete_task("/tmp/fh_test").isErr());
}

// ----------------------------------------------------------------------------------
// SET / DELETE TASK PATH
// ----------------------------------------------------------------------------------

UTEST_F(KilketFixture, set_task_path)
{
    ASSERT_TRUE(utest_fixture->fh->create_task("test_task", "/tmp/fh_test").isOk());
    auto r = utest_fixture->fh->set_task_path("/tmp/fh_test", "/tmp/fh_test/task_test");
    EXPECT_TRUE(r.isOk());
}
UTEST_F(KilketFixture, set_task_path_non_existent_task)
{
    EXPECT_TRUE(utest_fixture->fh->set_task_path("non_existent_task", "/tmp/fh_test/task_test").isErr());
}
UTEST_F(KilketFixture, set_task_path_invalid_path)
{
    ASSERT_TRUE(utest_fixture->fh->create_task("test_task", "/tmp/fh_test").isOk());
    EXPECT_TRUE(utest_fixture->fh->set_task_path("/tmp/fh_test", "/invalid/path").isErr());
}
UTEST_F(KilketFixture, set_empty_task_path)
{
    ASSERT_TRUE(utest_fixture->fh->create_task("test_task", "/tmp/fh_test").isOk());
    EXPECT_TRUE(utest_fixture->fh->set_task_path("/tmp/fh_test", "").isErr());
}
UTEST_F(KilketFixture, delete_task_path)
{
    ASSERT_TRUE(utest_fixture->fh->create_task("test_task", "/tmp/fh_test").isOk());
    ASSERT_TRUE(utest_fixture->fh->set_task_path("/tmp/fh_test", "/tmp/fh_test/task_test").isOk());
    EXPECT_TRUE(utest_fixture->fh->delete_task_path("/tmp/fh_test", "/tmp/fh_test/task_test").isOk());
}
UTEST_F(KilketFixture, delete_non_existent_task_path)
{
    ASSERT_TRUE(utest_fixture->fh->create_task("test_task", "/tmp/fh_test").isOk());
    EXPECT_TRUE(utest_fixture->fh->delete_task_path("/non_existent_task", "/tmp/fh_test/task_test").isErr());
}
UTEST_F(KilketFixture, delete_task_path_with_wrong_path)
{
    EXPECT_TRUE(utest_fixture->fh->delete_task_path("/tmp/fh_test", "/wrong/path").isErr());
}
UTEST_F(KilketFixture, delete_empty_task_path)
{
    EXPECT_TRUE(utest_fixture->fh->delete_task_path("/tmp/fh_test", "").isErr());
}

// ----------------------------------------------------------------------------------
// SET / DELETE TASK COMMAND
// ----------------------------------------------------------------------------------

UTEST_F(KilketFixture, set_task_command)
{
    ASSERT_TRUE(utest_fixture->fh->create_task("test_task", "/tmp/fh_test").isOk());
    EXPECT_TRUE(utest_fixture->fh->set_task_command("/tmp/fh_test", "ls").isOk());
}
UTEST_F(KilketFixture, set_task_command_non_existent_task)
{
    EXPECT_TRUE(utest_fixture->fh->set_task_command("non_existent_task", "ls").isErr());
}
UTEST_F(KilketFixture, set_task_command_empty_command)
{
    ASSERT_TRUE(utest_fixture->fh->create_task("test_task", "/tmp/fh_test").isOk());
    EXPECT_TRUE(utest_fixture->fh->set_task_command("/tmp/fh_test", "").isErr());
}
UTEST_F(KilketFixture, set_task_command_twice)
{
    ASSERT_TRUE(utest_fixture->fh->create_task("test_task", "/tmp/fh_test").isOk());
    EXPECT_TRUE(utest_fixture->fh->set_task_command("/tmp/fh_test", "ls").isOk());
    EXPECT_TRUE(utest_fixture->fh->set_task_command("/tmp/fh_test", "ls").isErr());
}
UTEST_F(KilketFixture, delete_task_command)
{
    ASSERT_TRUE(utest_fixture->fh->create_task("test_task", "/tmp/fh_test").isOk());
    EXPECT_TRUE(utest_fixture->fh->set_task_command("/tmp/fh_test", "ls").isOk());
    EXPECT_TRUE(utest_fixture->fh->delete_task_command("/tmp/fh_test", "ls").isOk());
}
UTEST_F(KilketFixture, delete_task_command_twice)
{
    ASSERT_TRUE(utest_fixture->fh->create_task("test_task", "/tmp/fh_test").isOk());
    EXPECT_TRUE(utest_fixture->fh->set_task_command("/tmp/fh_test", "ls").isOk());
    EXPECT_TRUE(utest_fixture->fh->delete_task_command("/tmp/fh_test", "ls").isOk());
    EXPECT_TRUE(utest_fixture->fh->delete_task_command("/tmp/fh_test", "ls").isErr());
}
UTEST_F(KilketFixture, delete_nonexistent_task_command)
{
    ASSERT_TRUE(utest_fixture->fh->create_task("test_task", "/tmp/fh_test").isOk());
    EXPECT_TRUE(utest_fixture->fh->delete_task_command("/tmp/fh_test", "ls").isErr());
}

// ----------------------------------------------------------------------------------
// DELETE TASK ON SUCCESS / DELETE TASK ON FAILURE
// ----------------------------------------------------------------------------------

UTEST_F(KilketFixture, delete_task_on_success)
{
    ASSERT_TRUE(utest_fixture->fh->create_task("test_task", "/tmp/fh_test").isOk());
    ASSERT_TRUE(utest_fixture->fh->set_task_on_success("/tmp/fh_test", "ls").isOk());
    EXPECT_TRUE(utest_fixture->fh->delete_task_on_success("/tmp/fh_test", "ls").isOk());
}
UTEST_F(KilketFixture, delete_task_on_success_non_existent_task)
{
    EXPECT_TRUE(utest_fixture->fh->delete_task_on_success("non_existent_task", "ls").isErr());
}
UTEST_F(KilketFixture, delete_task_on_success_non_existent_command)
{
    ASSERT_TRUE(utest_fixture->fh->create_task("test_task", "/tmp/fh_test").isOk());
    EXPECT_TRUE(utest_fixture->fh->delete_task_on_success("/tmp/fh_test", "non_existent_command").isErr());
}
UTEST_F(KilketFixture, delete_task_on_success_twice)
{
    ASSERT_TRUE(utest_fixture->fh->create_task("test_task", "/tmp/fh_test").isOk());
    ASSERT_TRUE(utest_fixture->fh->set_task_on_success("/tmp/fh_test", "ls").isOk());
    EXPECT_TRUE(utest_fixture->fh->delete_task_on_success("/tmp/fh_test", "ls").isOk());
    EXPECT_TRUE(utest_fixture->fh->delete_task_on_success("test_task", "ls").isErr());
}
UTEST_F(KilketFixture, delete_task_on_success_empty_command)
{
    ASSERT_TRUE(utest_fixture->fh->create_task("test_task", "/tmp/fh_test").isOk());
    EXPECT_TRUE(utest_fixture->fh->delete_task_on_success("/tmp/fh_test", "").isErr());
}

UTEST_F(KilketFixture, delete_task_on_failure)
{
    ASSERT_TRUE(utest_fixture->fh->create_task("test_task", "/tmp/fh_test").isOk());
    ASSERT_TRUE(utest_fixture->fh->set_task_on_failure("/tmp/fh_test", "ls").isOk());
    EXPECT_TRUE(utest_fixture->fh->delete_task_on_failure("/tmp/fh_test", "ls").isOk());
}
UTEST_F(KilketFixture, delete_task_on_failure_non_existent_task)
{
    EXPECT_TRUE(utest_fixture->fh->delete_task_on_failure("non_existent_task", "ls").isErr());
}
UTEST_F(KilketFixture, delete_task_on_failure_non_existent_command)
{
    ASSERT_TRUE(utest_fixture->fh->create_task("test_task", "/tmp/fh_test").isOk());
    EXPECT_TRUE(utest_fixture->fh->delete_task_on_failure("/tmp/fh_test", "non_existent_command").isErr());
}
UTEST_F(KilketFixture, delete_task_on_failure_twice)
{
    ASSERT_TRUE(utest_fixture->fh->create_task("test_task", "/tmp/fh_test").isOk());
    ASSERT_TRUE(utest_fixture->fh->set_task_on_failure("/tmp/fh_test", "ls").isOk());
    EXPECT_TRUE(utest_fixture->fh->delete_task_on_failure("/tmp/fh_test", "ls").isOk());
    EXPECT_TRUE(utest_fixture->fh->delete_task_on_failure("/tmp/fh_test", "ls").isErr());
}
UTEST_F(KilketFixture, delete_task_on_failure_empty_command)
{
    ASSERT_TRUE(utest_fixture->fh->create_task("test_task", "/tmp/fh_test").isOk());
    EXPECT_TRUE(utest_fixture->fh->delete_task_on_failure("/tmp/fh_test", "").isErr());
}

// ----------------------------------------------------------------------------------
// START / STOP TASK
// ----------------------------------------------------------------------------------
