#pragma once

#include <string>
#include <object.h>

class Interpreter {
    std::shared_ptr<Environment> global_env_;

public:
    Interpreter();

    std::string Run(const std::string&);
    std::string Print(std::shared_ptr<Object>);

    std::string PrintCell(std::shared_ptr<Object>);
    std::string PrintList(std::shared_ptr<Object>);

    std::string PrintInteger(std::shared_ptr<Object>);
    std::string PrintRealBoolean(std::shared_ptr<Object>);
    std::string PrintSymbol(std::shared_ptr<Object>);
};
