#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static int tests_run = 0;
static int tests_failed = 0;

#define ASSERT_TRUE(condition, message)                  \
    do {                                                 \
        tests_run++;                                     \
        if (!(condition)) {                              \
            tests_failed++;                              \
            printf("FAIL: %s\n", message);              \
        } else {                                         \
            printf("PASS: %s\n", message);              \
        }                                                \
    } while (0)

int main(void)
{
    int result;

    printf("Running client tests...\n");

    ASSERT_TRUE(access("./bin/poker", X_OK) == 0, "client executable exists and is executable");

    result = system("./bin/poker > /tmp/test_client_output.txt 2>&1");

    ASSERT_TRUE(result != 0, "client rejects missing command line arguments");

    printf("Client tests run: %d\n", tests_run);
    printf("Client tests failed: %d\n", tests_failed);

    if (tests_failed == 0) {
        printf("Client test passed.\n");
        return 0;
    }

    printf("Client test failed.\n");
    return 1;
}