#include "test.h"

#include "./LPP_2_3_1/LivePP/API/x64/LPP_API_x64_CPP.h"

#include <cstdio>
#include <mutex>

static std::mutex s_mu;
static bool s_run_tests;
static std::condition_variable s_cv;

extern "C" __declspec(dllimport) unsigned long __stdcall GetLastError();

int test_main(int argc, char **argv) {
  test_init(argc, argv);

  TEST_CASE("FooTest") {
    printf("foo\n");
    printf("foo\n");
  }

  TEST_CASE("BarTest") {
    printf("bar\n");
    printf("bar\n");
    printf("bar\n");
  }

  TEST_CASE("BazTest") {
    printf("baz\n");
    printf("baz\n");
    printf("baz\n");
    printf("baz\n");
  }

  TEST_CASE("QuxTest") {
    printf("qux\n");
    printf("qux\n");
    printf("qux\n");
    printf("qux\n");
    printf("qux\n");
  }

  test_exit();
  return 0;
}

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
