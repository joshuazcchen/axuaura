#pragma once
#include <functional>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace test {
    struct Case { std::string name; std::function<void()> fn; };
    inline std::vector<Case>& registry() { static std::vector<Case> r; return r; }
    inline void reg(const std::string& name, std::function<void()> fn) { registry().push_back({name, fn}); }

    inline int run_all() {
        int passed = 0, failed = 0;
        for (auto& c : registry()) {
            try { c.fn(); std::cout << "PASS: " << c.name << "\n"; ++passed; }
            catch (const std::exception& e) { std::cout << "FAIL: " << c.name << ": " << e.what() << "\n"; ++failed; }
            catch (...) { std::cout << "FAIL: " << c.name << ": unknown exception\n"; ++failed; }
        }
        std::cout << "\n" << passed << " passed, " << failed << " failed.\n";
        return failed ? 1 : 0;
    }

    inline void assert_true(bool cond, const std::string& msg = "assertion failed") {
        if (!cond) throw std::runtime_error(msg);
    }
    inline void assert_eq(int a, int b, const std::string& msg = "") {
        if (a != b) throw std::runtime_error((msg.empty() ? "" : msg + ": ") + std::to_string(a) + " != " + std::to_string(b));
    }
    inline void assert_eq(const std::string& a, const std::string& b, const std::string& msg = "") {
        if (a != b) throw std::runtime_error((msg.empty() ? "" : msg + ": ") + "\"" + a + "\" != \"" + b + "\"");
    }
} // namespace test

#define TEST(name) \
    static void _test_##name(); \
    static const bool _reg_##name = (test::reg(#name, _test_##name), true); \
    static void _test_##name()
