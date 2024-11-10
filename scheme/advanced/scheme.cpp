#include "scheme.h"
#include "tokenizer.h"
#include "parser.h"
#include <sstream>

Interpreter::Interpreter() : global_env_(std::make_shared<Environment>()) {
}

std::string Interpreter::Run(const std::string& str) {
    std::stringstream ss{str};
    Tokenizer tokenizer{&ss};

    auto obj = Read(&tokenizer);
    auto result = Evaluate(obj, global_env_);
    return Print(result);
}

std::string Interpreter::Print(std::shared_ptr<Object> obj) {
    if (!obj) {
        return "()";
    } else if (Is<Number>(obj)) {
        return PrintInteger(obj);
    } else if (Is<RealBoolean>(obj)) {
        return PrintRealBoolean(obj);
    } else if (Is<Variable>(obj)) {
        std::shared_ptr<Object> sp;
        std::string name = As<Variable>(obj)->GetName();
        if (global_env_->Get(name, sp)) {
            return As<Variable>(obj)->GetName();
        }
        return Print(sp);
    } else if (Is<Symbol>(obj)) {
        return PrintSymbol(obj);
    } else if (Is<Cell>(obj)) {
        return PrintCell(obj);
    } else {
        throw RuntimeError("Unknown object type");
    }
}

std::string Interpreter::PrintCell(std::shared_ptr<Object> obj) {
    std::string result = "(";
    result += PrintList(obj);
    result += ")";
    return result;
}

std::string Interpreter::PrintList(std::shared_ptr<Object> obj) {
    std::string result;
    auto cell = As<Cell>(obj);
    auto first = cell->GetFirst();
    auto second = cell->GetSecond();
    result += Print(first);

    if (second) {
        if (Is<Cell>(second)) {
            result += " ";
            result += PrintList(second);
        } else {
            result += " . ";
            result += Print(second);
        }
    }
    return result;
}

std::string Interpreter::PrintInteger(std::shared_ptr<Object> obj) {
    return std::to_string(As<Number>(obj)->GetValue());
}

std::string Interpreter::PrintRealBoolean(std::shared_ptr<Object> obj) {
    return As<Boolean>(obj)->GetBoolean() ? "#t" : "#f";
}

std::string Interpreter::PrintSymbol(std::shared_ptr<Object> obj) {
    return As<Symbol>(obj)->GetName();
}
