#ifndef TEST_HELPER_H
#define TEST_HELPER_H

#include <iostream>
#include <cstdio>
#include <string>
#include <exception>
#include <stdexcept>

// Biến đếm tổng số test cases pass/fail toàn hệ thống
inline int g_totalPass = 0;
inline int g_totalFail = 0;

// Macro kiểm tra điều kiện assertion (Nếu sai sẽ ném ngoại lệ kèm thông tin file:dòng)
#define TEST_ASSERT(condition, message) \
    do { \
        if (!(condition)) { \
            throw std::runtime_error(std::string("Assertion failed: ") + (message) + " (" + __FILE__ + ":" + std::to_string(__LINE__) + ")"); \
        } \
    } while(0)

#define TEST_ASSERT_TRUE(condition) \
    TEST_ASSERT(condition, #condition)

// Macro thực thi 1 Test Case lẻ, bắt lỗi và in kết quả chi tiết
#define RUN_TEST_CASE(func) \
    do { \
        try { \
            func(); \
            std::cout << "    [PASS] " << #func << std::endl; \
            std::fflush(stdout); \
            g_totalPass++; \
        } catch (const std::exception& e) { \
            std::cout << "    [FAIL] " << #func << " -> " << e.what() << std::endl; \
            std::fflush(stdout); \
            g_totalFail++; \
        } catch (...) { \
            std::cout << "    [FAIL] " << #func << " -> Unknown exception" << std::endl; \
            std::fflush(stdout); \
            g_totalFail++; \
        } \
    } while(0)

// Macro thực thi 1 Test Block (inline), bắt lỗi và in kết quả chi tiết
#define RUN_INLINE_TEST(name, block) \
    do { \
        try { \
            block \
            std::cout << "    [PASS] " << name << std::endl; \
            std::fflush(stdout); \
            g_totalPass++; \
        } catch (const std::exception& e) { \
            std::cout << "    [FAIL] " << name << " -> " << e.what() << std::endl; \
            std::fflush(stdout); \
            g_totalFail++; \
        } catch (...) { \
            std::cout << "    [FAIL] " << name << " -> Unknown exception" << std::endl; \
            std::fflush(stdout); \
            g_totalFail++; \
        } \
    } while(0)

#endif // TEST_HELPER_H
