#pragma once

#include <memory>
#include <string>
#include <unordered_set>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <stdexcept>
#include <exception>
#include <functional>

class AbstractTest {
public:
    virtual void SetUp() = 0;
    virtual void TearDown() = 0;
    virtual void Run() = 0;
    virtual ~AbstractTest() {
    }
};

class TestRegistry {
    std::unordered_set<std::string> registered_tests_;
    std::unordered_map<std::string, std::function<std::unique_ptr<AbstractTest>()>>
        test_constructors_;

public:
    TestRegistry() = default;
    TestRegistry(const TestRegistry&) = delete;
    TestRegistry(TestRegistry&&) = delete;
    TestRegistry& operator=(const TestRegistry&) = delete;
    TestRegistry& operator=(TestRegistry&&) = delete;

    static TestRegistry& Instance() {
        static TestRegistry instance;
        return instance;
    }

    template <class TestClass>
    void RegisterClass(const std::string& class_name) {
        if (registered_tests_.find(class_name) != registered_tests_.end()) {
            throw std::runtime_error("Test already registered!");
        }
        registered_tests_.insert(class_name);
        test_constructors_[class_name] = []() -> std::unique_ptr<AbstractTest> {
            return std::make_unique<TestClass>();
        };
    }

    std::unique_ptr<AbstractTest> CreateTest(const std::string& class_name) {
        if (registered_tests_.find(class_name) == registered_tests_.end()) {
            throw std::out_of_range("Test not registered: " + class_name);
        }
        return test_constructors_[class_name]();
    }

    void RunTest(const std::string& test_name) {
        auto test = CreateTest(test_name);
        try {
            test->SetUp();
            test->Run();
        } catch (...) {
            test->TearDown();
            throw;
        }
        test->TearDown();
    }

    template <class Predicate>
    std::vector<std::string> ShowTests(Predicate callback) const {
        std::vector<std::string> filtered_tests;
        for (const auto& test_name : registered_tests_) {
            if (callback(test_name)) {
                filtered_tests.push_back(test_name);
            }
        }
        std::sort(filtered_tests.begin(), filtered_tests.end());
        return filtered_tests;
    }

    std::vector<std::string> ShowAllTests() const {
        std::vector<std::string> test_list(registered_tests_.begin(), registered_tests_.end());
        std::sort(test_list.begin(), test_list.end());
        return test_list;
    }

    template <class Predicate>
    void RunTests(Predicate callback) {
        auto running_tests = ShowTests(callback);
        for (const auto& test_name : running_tests) {
            RunTest(test_name);
        }
    }

    void Clear() {
        registered_tests_.clear();
        test_constructors_.clear();
    }
};

class FullMatch {
public:
    explicit FullMatch(const std::string& pattern) : pattern_(pattern) {
    }

    bool operator()(const std::string& test_name) const {
        return test_name == pattern_;
    }

private:
    std::string pattern_;
};

class Substr {
public:
    explicit Substr(const std::string& substr) : substr_(substr) {
    }

    bool operator()(const std::string& test_name) const {
        return test_name.find(substr_) != std::string::npos;
    }

private:
    std::string substr_;
};