
#ifndef INCLUDE_GUARD__UNIT_TEST_HELPERS_H__
#define INCLUDE_GUARD__UNIT_TEST_HELPERS_H__

#define TEST_ASSERT(x) if (!(x)) { return 1; }
#define TEST_ASSERT_TRUE(x) if (!(x)) { return 1; }
#define TEST_ASSERT_FALSE(x) if ((x)) { return 1; }

#endif // INCLUDE_GUARD__UNIT_TEST_HELPERS_H__

