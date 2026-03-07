#include <unity.h>
#include <string.h>

// Minimal test stub - real tests are in src/test_*.cpp
void setUp(void) {}
void tearDown(void) {}

void test_stub_passes(void) {
    TEST_ASSERT_TRUE(1);
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_stub_passes);
    return UNITY_END();
}
