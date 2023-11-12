#include "./LPP_2_3_1/LivePP/API/x64/LPP_API_x64_CPP.h"

#include <cstdio>
#include <mutex>

static std::mutex s_mu;
static bool s_run_tests;
static std::condition_variable s_cv;

enum test_status {
  TEST_STATUS_NOT_RUN,
  TEST_STATUS_OK,
  TEST_STATUS_FAIL,
};

struct test_case {
  const char *name_;
  test_case *next_; // The test case that came before this
  test_status status_;
};

struct test_context {
  int argc_;
  char **argv_;
  test_case *curr_;
  test_case *last_; // Most recently registered test
};

void test_init(int argc, char **argv);
int test_main(int argc, char **argv);
void test_exit();
void test_register(test_case *test);
int test_eval(test_case *test);
void test_begin(test_case *test, test_status status);
void test_end(test_case *test);

#define _TEST_CONCAT2(a, b) a##b
#define _TEST_CONCAT(a, b) _TEST_CONCAT2(a, b)

#define TEST_CASE(label)                                                       \
  static test_case _TEST_CONCAT(test_, __LINE__) = {label, NULL,               \
                                                    TEST_STATUS_NOT_RUN};      \
  test_register(&_TEST_CONCAT(test_, __LINE__));                               \
  if (test_eval(&_TEST_CONCAT(test_, __LINE__)))

extern "C" __declspec(dllimport) unsigned long __stdcall GetLastError();

int main(int argc, char **argv) {
  lpp::LppDefaultAgent lpp_agent =
      lpp::LppCreateDefaultAgentANSI(nullptr, "LPP_2_3_1\\LivePP");
  if (!lpp::LppIsValidDefaultAgent(&lpp_agent)) {
    printf("cannot load LPP under LPP_2_3_1\\LivePP: %u\n", GetLastError());
    return 1;
  }

  lpp_agent.EnableModule(lpp::LppGetCurrentModulePath(),
                         lpp::LPP_MODULES_OPTION_ALL_IMPORT_MODULES, nullptr,
                         nullptr);

  s_run_tests = true;
  for (;;) {
    std::unique_lock<std::mutex> lck(s_mu);
    if (s_run_tests) {
      test_main(argc, argv);
      s_run_tests = false;
    }
    s_cv.wait(lck);
  }

  lpp::LppDestroyDefaultAgent(&lpp_agent);
  return 0;
}

int test_main(int argc, char **argv) {
  test_init(argc, argv);

  TEST_CASE("FooTest") {
    printf("foo\n");
    printf("foo\n");
    printf("foo\n");

    printf("foo\n");
    printf("foo\n");
    printf("foo\n");
  }

  TEST_CASE("BarTest") {
    printf("bar\n");
    printf("bar\n");
    // printf("bar\n");
  }

  TEST_CASE("BazTest") { printf("baz\n"); }

  test_exit();
  return 0;
}

// test runner

static test_context g_test_context;

void test_init(int argc, char *argv[]) {
  memset(&g_test_context, 0, sizeof(test_context));
  g_test_context.argc_ = argc;
  g_test_context.argv_ = argv;
}

void test_exit() {
  test_case *test = g_test_context.last_;

  if (test) {
    test_end(test);
  }

  puts(""); // new line

  int passed = 0, failed = 0, skipped = 0;

  while (test) {
    switch (test->status_) {
    case TEST_STATUS_NOT_RUN: {
      skipped++;
      break;
    }
    case TEST_STATUS_OK: {
      passed++;
      break;
    }
    case TEST_STATUS_FAIL: {
      failed++;
      break;
    }
    default:
      break; // whatever
    }
    test = test->next_;
  }

  printf("%i %s passed, %i %s failed, %i %s skipped\n", passed,
         passed == 1 ? "test" : "tests", failed, failed == 1 ? "test" : "tests",
         skipped, skipped == 1 ? "test" : "tests");
}

void test_register(test_case *test) {
  test->next_ = g_test_context.last_;
  g_test_context.last_ = test;
}

int test_eval(test_case *test) {
  if (test->next_) {
    test_end(test->next_);
  }

  // 1: run test; 0: skip test
  int eval = 1;

  for (int i = 1; i < g_test_context.argc_; i++) {
    if (strncmp(g_test_context.argv_[i], "-", 1) == 0) {
      i++;
      continue;
    } else {
      eval = 0;
      break;
    }
  }

  if (eval == 0) {
    for (int i = 1; i < g_test_context.argc_; i++) {
      if (strncmp(g_test_context.argv_[i], "-", 1) == 0) {
        i++;
        continue;
      } else if (strcmp(g_test_context.argv_[i], test->name_) == 0) {
        eval = 1;
        break;
      }
    }
  }

  if (eval == 1) {
    test_begin(test, TEST_STATUS_OK);
  } else {
    test_begin(test, TEST_STATUS_NOT_RUN);
  }

  return eval;
}

void test_begin(test_case *test, test_status status) {
  g_test_context.curr_ = test;
  g_test_context.curr_->status_ = status;
  putchar('.'); // progress indicator
  fflush(stdout);
}

void test_end(test_case *test) {
  // ...
}

static void FunctionCalledAfterPatching(lpp::LppHotReloadPostpatchHookId,
                                        const wchar_t *const,
                                        const wchar_t *const *const,
                                        unsigned int,
                                        const wchar_t *const *const,
                                        unsigned int) {
  std::unique_lock<std::mutex> lck(s_mu);
  s_run_tests = true;
  s_cv.notify_one();
}

LPP_HOTRELOAD_POSTPATCH_HOOK(FunctionCalledAfterPatching);
