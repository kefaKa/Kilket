#include <filesystem>
#include <sys/inotify.h>
#include <vector>
#include <string>


#include "../../src/include/session_logger.h"
#include "../../src/include/types.h"
#include "../utest.h"

namespace fs = std::filesystem;
using namespace kilket;
using namespace std;

bool KILKET_DEBUG = false;
bool KILKET_VERBOSE = false;
bool KILKET_QUIET = false;

// ---------------------------------------------------------------------------
// Fixture
// ---------------------------------------------------------------------------
struct SessionLoggerFixture
{
    SessionLogger *sl;
    ExecutionResult *er;
    ExecutionResult *er2;
};

UTEST_F_SETUP(SessionLoggerFixture)
{
    WatchEvent e(0, "file", "/tmp/sl_test_file.txt", IN_MODIFY);
    utest_fixture->er = new ExecutionResult(1, 0, e, "log", vector<string>{"ls"});
    utest_fixture->er2 = new ExecutionResult(2, 0, e, "log", vector<string>{"ls"});
    auto s = SessionLogger::create("/tmp/sl_test_file.txt");
    if(s.isErr())
    {
        cout << s.getErrMessage() << endl;
        exit(1);
    }

    utest_fixture->sl = s.unwrap();

    fs::create_directories("/tmp/sl_test");
}

UTEST_F_TEARDOWN(SessionLoggerFixture)
{
    delete utest_fixture->sl;
    delete utest_fixture->er2;
    delete utest_fixture->er;
    fs::remove_all("/tmp/sl_test");
}
