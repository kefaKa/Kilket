#include <filesystem>
#include <fstream>

#include "../../src/include/filewatcher.h"
#include "../../src/include/types.h"
#include "../utest.h"

namespace fs = std::filesystem;
using namespace kilket;
using namespace std;

bool KILKET_DEBUG = false;
bool KILKET_VERBOSE = false;
bool KILKET_QUIET = false;


// ---------------------------------------------------------------------------
// Callbacks
// ---------------------------------------------------------------------------

namespace
{
    Result<void> temp_cb(const WatchEvent &e) { return Result<void>::Ok(); }
    Result<void> temp_cb2(const WatchEvent &e) { return Result<void>::Ok(); }
    Result<void> temp_cb3(const WatchEvent &e) { return Result<void>::Ok(); }

}

// ---------------------------------------------------------------------------
// Fixture
// ---------------------------------------------------------------------------

struct FileWatcherFixture
{
    FileWatcher *fw;
    WatchCallback cb;
    WatchCallback cb2;
    WatchCallback cb3;
};

UTEST_F_SETUP(FileWatcherFixture)
{
    utest_fixture->cb = WatchCallback::from_raw(temp_cb);
    utest_fixture->cb2 = WatchCallback::from_raw(temp_cb2);
    utest_fixture->cb3 = WatchCallback::from_raw(temp_cb3);
    fs::create_directories("/tmp/fh_test");
    fs::create_directories("/tmp/fh_test1");
    fs::create_directories("/tmp/fh_test2");
    fs::create_directories("/tmp/fh_test3");

    auto r = FileWatcher::create();
    ASSERT_TRUE(r.isOk());
    utest_fixture->fw = r.unwrap();
}

UTEST_F_TEARDOWN(FileWatcherFixture)
{
    utest_fixture->fw->stop();
    fs::remove_all("/tmp/fh_test");
    fs::remove_all("/tmp/fh_test1");
    fs::remove_all("/tmp/fh_test2");
    fs::remove_all("/tmp/fh_test3");
    fs::remove("/tmp/fh_test_file.txt");
    delete utest_fixture->fw;
}

// ---------------------------------------------------------------------------
// Initialization
// ---------------------------------------------------------------------------

UTEST_F(FileWatcherFixture, create_success)
{
    ASSERT_NE(utest_fixture->fw, nullptr);
}

// ---------------------------------------------------------------------------
// AddPath
// ---------------------------------------------------------------------------

UTEST_F(FileWatcherFixture, add_path_normal)
{
    EXPECT_TRUE(utest_fixture->fw->add_path("/tmp/fh_test").isOk());
}
UTEST_F(FileWatcherFixture, add_multiple_paths)
{
    EXPECT_TRUE(utest_fixture->fw->add_path("/tmp/fh_test1").isOk());
    EXPECT_TRUE(utest_fixture->fw->add_path("/tmp/fh_test2").isOk());
    EXPECT_TRUE(utest_fixture->fw->add_path("/tmp/fh_test3").isOk());
}
UTEST_F(FileWatcherFixture, add_nonexistent_path)
{
    EXPECT_TRUE(utest_fixture->fw->add_path("/tmp/fh_nonexistent").isErr());
}
UTEST_F(FileWatcherFixture, add_path_twice)
{
    EXPECT_TRUE(utest_fixture->fw->add_path("/tmp/fh_test").isOk());
    EXPECT_TRUE(utest_fixture->fw->add_path("/tmp/fh_test").isErr());
}
UTEST_F(FileWatcherFixture, add_empty_path)
{
    EXPECT_TRUE(utest_fixture->fw->add_path("").isErr());
}
UTEST_F(FileWatcherFixture, add_path_file)
{
    ofstream("/tmp/fh_test_file.txt").close();
    EXPECT_TRUE(utest_fixture->fw->add_path("/tmp/fh_test_file.txt").isOk());
}

// ---------------------------------------------------------------------------
// RemovePath
// ---------------------------------------------------------------------------

UTEST_F(FileWatcherFixture, remove_path)
{
    ASSERT_TRUE(utest_fixture->fw->add_path("/tmp/fh_test").isOk());
    EXPECT_TRUE(utest_fixture->fw->remove_path("/tmp/fh_test").isOk());
}
UTEST_F(FileWatcherFixture, remove_nonexistent_path)
{
    EXPECT_TRUE(utest_fixture->fw->remove_path("/tmp/fh_nonexistent").isErr());
}
UTEST_F(FileWatcherFixture, remove_path_twice)
{
    ASSERT_TRUE(utest_fixture->fw->add_path("/tmp/fh_test").isOk());
    EXPECT_TRUE(utest_fixture->fw->remove_path("/tmp/fh_test").isOk());
    EXPECT_TRUE(utest_fixture->fw->remove_path("/tmp/fh_test").isErr());
}
UTEST_F(FileWatcherFixture, remove_empty_path)
{
    EXPECT_TRUE(utest_fixture->fw->remove_path("").isErr());
}
UTEST_F(FileWatcherFixture, remove_multiple_paths)
{
    ASSERT_TRUE(utest_fixture->fw->add_path("/tmp/fh_test1").isOk());
    ASSERT_TRUE(utest_fixture->fw->add_path("/tmp/fh_test2").isOk());
    ASSERT_TRUE(utest_fixture->fw->add_path("/tmp/fh_test3").isOk());
    EXPECT_TRUE(utest_fixture->fw->remove_path("/tmp/fh_test1").isOk());
    EXPECT_TRUE(utest_fixture->fw->remove_path("/tmp/fh_test2").isOk());
    EXPECT_TRUE(utest_fixture->fw->remove_path("/tmp/fh_test3").isOk());
}
UTEST_F(FileWatcherFixture, remove_path_file)
{
    ofstream("/tmp/fh_test_file.txt").close();
    ASSERT_TRUE(utest_fixture->fw->add_path("/tmp/fh_test_file.txt").isOk());
    EXPECT_TRUE(utest_fixture->fw->remove_path("/tmp/fh_test_file.txt").isOk());
}
