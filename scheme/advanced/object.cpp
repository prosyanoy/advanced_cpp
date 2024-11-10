#include <object.h>
#include <algorithm>
#include <limits>

std::shared_ptr<Object> Evaluate(std::shared_ptr<Object> obj, std::shared_ptr<Environment> env) {
    if (!obj) {
        throw RuntimeError("Cannot evaluate empty list");
    } else if (Is<Number>(obj) || Is<RealBoolean>(obj)) {
        return obj;
    } else if (Is<Symbol>(obj)) {
        std::shared_ptr<Object> sp;
        if (!(env->Get(As<Symbol>(obj)->GetName(), sp))) {
            return sp;
        }
    }
    if (Is<Variable>(obj)) {
        throw NameError("Variable " + As<Variable>(obj)->GetName() + " is undefined");
    } else if (Is<Symbol>(obj)) {
        return obj;
    } else if (Is<Cell>(obj)) {
        auto cell = As<Cell>(obj);
        auto first = cell->GetFirst();
        auto second = cell->GetSecond();

        auto func = Evaluate(first, env);

        if (Is<Symbol>(func)) {
            return As<Symbol>(func)->Do(second, env);
        } else {
            throw RuntimeError("First element is not a function");
        }
    } else {
        throw RuntimeError("Unknown expression type");
    }
}

//////////////////// Environment
Environment::Environment(std::shared_ptr<Environment> parent) : parent_(parent) {
}

void Environment::Define(const std::string& name, std::shared_ptr<Object> value) {
    vars_[name] = value;
}

void Environment::Set(const std::string& name, std::shared_ptr<Object> value) {
    if (vars_.find(name) != vars_.end()) {
        vars_[name] = value;
    } else if (parent_) {
        parent_->Set(name, value);
    } else {
        throw NameError("Variable " + name + " is undefined");
    }
}

int Environment::Get(const std::string& name, std::shared_ptr<Object>& sp) {
    if (vars_.find(name) != vars_.end()) {
        sp = vars_[name];
        return 0;
    } else if (parent_) {
        return parent_->Get(name, sp);
    } else {
        return 1;
    }
}

//////////////////// Number
Number::Number(int val) : Boolean(true), value_(val) {
}

int Number::GetValue() const {
    return value_;
}

//////////////////// Symbol
Symbol::Symbol(std::string name) : name_(name) {
}
const std::string& Symbol::GetName() const {
    return name_;
}

//////// Variable
Variable::Variable(std::string name) : Symbol(name) {
}

std::shared_ptr<Object> Variable::Do(std::shared_ptr<Object>, std::shared_ptr<Environment> env) {
    throw NameError("Not a valid operation");
}
//////////////////// Quote

Quote::Quote(bool is_symbol) : Symbol("quote"), is_symbol_(is_symbol) {
}

std::shared_ptr<Object> Quote::Do(std::shared_ptr<Object> args, std::shared_ptr<Environment> env) {
    if (IsSymbol()) {
        if (!args || !Is<Cell>(args)) {
            throw SyntaxError("Syntax error in quote");
        }
        auto cell = As<Cell>(args);
        if (cell->GetSecond()) {
            throw SyntaxError("quote takes exactly one argument");
        }
        return cell->GetFirst();
    }
    return args;
}

bool Quote::IsSymbol() {
    return is_symbol_;
}

//////////////////// Boolean

Boolean::Boolean(bool b) : boolean_(b) {
}

bool Boolean::GetBoolean() {
    return boolean_;
}

//////////////////// RealBoolean
RealBoolean::RealBoolean(bool b) : Boolean(b) {
}

//////////////////// #t #f

BooleanTrue::BooleanTrue() : RealBoolean(true) {
}

BooleanFalse::BooleanFalse() : RealBoolean(false) {
}

//////////////////// Integers
//////// IntegerPredicate
IntegerPredicate::IntegerPredicate() : Symbol("number?") {
}

std::shared_ptr<Object> IntegerPredicate::Do(std::shared_ptr<Object> args,
                                             std::shared_ptr<Environment> env) {
    if (!args || !Is<Cell>(args)) {
        throw RuntimeError("Invalid arguments");
    }
    auto cell = As<Cell>(args);
    if (cell->GetSecond()) {
        throw RuntimeError("number? takes exactly one argument");
    }
    auto arg = Evaluate(cell->GetFirst(), env);
    if (Is<Number>(arg)) {
        return std::make_shared<BooleanTrue>();
    } else {
        return std::make_shared<BooleanFalse>();
    }
}

//////// IntegerEqual
IntegerEqual::IntegerEqual() : Symbol("=") {
}

std::shared_ptr<Object> IntegerEqual::Do(std::shared_ptr<Object> args,
                                         std::shared_ptr<Environment> env) {
    if (!args) {
        return std::make_shared<BooleanTrue>();
    }

    if (!Is<Cell>(args)) {
        throw RuntimeError("Invalid arguments");
    }

    auto cell = As<Cell>(args);
    auto first_arg = Evaluate(cell->GetFirst(), env);
    if (!Is<Number>(first_arg)) {
        throw RuntimeError("Invalid arguments: expected numbers");
    }
    int first_value = As<Number>(first_arg)->GetValue();

    auto rest = cell->GetSecond();
    while (rest) {
        if (!Is<Cell>(rest)) {
            throw RuntimeError("Invalid arguments");
        }
        cell = As<Cell>(rest);
        auto next_arg = Evaluate(cell->GetFirst(), env);
        if (!Is<Number>(next_arg)) {
            throw RuntimeError("Invalid arguments: expected numbers");
        }
        int next_value = As<Number>(next_arg)->GetValue();

        if (first_value != next_value) {
            return std::make_shared<BooleanFalse>();
        }
        rest = cell->GetSecond();
    }
    return std::make_shared<BooleanTrue>();
}

//////// IntegerMore
IntegerMore::IntegerMore() : Symbol(">") {
}

std::shared_ptr<Object> IntegerMore::Do(std::shared_ptr<Object> args,
                                        std::shared_ptr<Environment> env) {
    if (!args) {
        return std::make_shared<BooleanTrue>();
    }

    if (!Is<Cell>(args)) {
        throw RuntimeError("Invalid arguments");
    }

    auto cell = As<Cell>(args);
    auto prev_arg = Evaluate(cell->GetFirst(), env);
    if (!Is<Number>(prev_arg)) {
        throw RuntimeError("Invalid arguments: expected numbers");
    }
    int prev_value = As<Number>(prev_arg)->GetValue();

    auto rest = cell->GetSecond();
    while (rest) {
        if (!Is<Cell>(rest)) {
            throw RuntimeError("Invalid arguments");
        }
        cell = As<Cell>(rest);
        auto curr_arg = Evaluate(cell->GetFirst(), env);
        if (!Is<Number>(curr_arg)) {
            throw RuntimeError("Invalid arguments: expected numbers");
        }
        int curr_value = As<Number>(curr_arg)->GetValue();

        if (prev_value <= curr_value) {
            return std::make_shared<BooleanFalse>();
        }
        prev_value = curr_value;
        rest = cell->GetSecond();
    }
    return std::make_shared<BooleanTrue>();
}

//////// IntegerLess
IntegerLess::IntegerLess() : Symbol("<") {
}

std::shared_ptr<Object> IntegerLess::Do(std::shared_ptr<Object> args,
                                        std::shared_ptr<Environment> env) {
    if (!args) {
        return std::make_shared<BooleanTrue>();
    }

    if (!Is<Cell>(args)) {
        throw RuntimeError("Invalid arguments");
    }

    auto cell = As<Cell>(args);
    auto prev_arg = Evaluate(cell->GetFirst(), env);
    if (!Is<Number>(prev_arg)) {
        throw RuntimeError("Invalid arguments: expected numbers");
    }
    int prev_value = As<Number>(prev_arg)->GetValue();

    auto rest = cell->GetSecond();
    while (rest) {
        if (!Is<Cell>(rest)) {
            throw RuntimeError("Invalid arguments");
        }
        cell = As<Cell>(rest);
        auto curr_arg = Evaluate(cell->GetFirst(), env);
        if (!Is<Number>(curr_arg)) {
            throw RuntimeError("Invalid arguments: expected numbers");
        }
        int curr_value = As<Number>(curr_arg)->GetValue();

        if (prev_value >= curr_value) {
            return std::make_shared<BooleanFalse>();
        }
        prev_value = curr_value;
        rest = cell->GetSecond();
    }
    return std::make_shared<BooleanTrue>();
}

//////// IntegerMoreEqual

IntegerMoreEqual::IntegerMoreEqual() : Symbol(">=") {
}

std::shared_ptr<Object> IntegerMoreEqual::Do(std::shared_ptr<Object> args,
                                             std::shared_ptr<Environment> env) {
    if (!args) {
        return std::make_shared<BooleanTrue>();
    }

    if (!Is<Cell>(args)) {
        throw RuntimeError("Invalid arguments");
    }

    auto cell = As<Cell>(args);
    auto prev_arg = Evaluate(cell->GetFirst(), env);
    if (!Is<Number>(prev_arg)) {
        throw RuntimeError("Invalid arguments: expected numbers");
    }
    int prev_value = As<Number>(prev_arg)->GetValue();

    auto rest = cell->GetSecond();
    while (rest) {
        if (!Is<Cell>(rest)) {
            throw RuntimeError("Invalid arguments");
        }
        cell = As<Cell>(rest);
        auto curr_arg = Evaluate(cell->GetFirst(), env);
        if (!Is<Number>(curr_arg)) {
            throw RuntimeError("Invalid arguments: expected numbers");
        }
        int curr_value = As<Number>(curr_arg)->GetValue();

        if (prev_value < curr_value) {
            return std::make_shared<BooleanFalse>();
        }
        prev_value = curr_value;
        rest = cell->GetSecond();
    }
    return std::make_shared<BooleanTrue>();
}

//////// IntegerLessEqual
IntegerLessEqual::IntegerLessEqual() : Symbol("<=") {
}

std::shared_ptr<Object> IntegerLessEqual::Do(std::shared_ptr<Object> args,
                                             std::shared_ptr<Environment> env) {
    if (!args) {
        return std::make_shared<BooleanTrue>();
    }

    if (!Is<Cell>(args)) {
        throw RuntimeError("Invalid arguments");
    }

    auto cell = As<Cell>(args);
    auto prev_arg = Evaluate(cell->GetFirst(), env);
    if (!Is<Number>(prev_arg)) {
        throw RuntimeError("Invalid arguments: expected numbers");
    }
    int prev_value = As<Number>(prev_arg)->GetValue();

    auto rest = cell->GetSecond();
    while (rest) {
        if (!Is<Cell>(rest)) {
            throw RuntimeError("Invalid arguments");
        }
        cell = As<Cell>(rest);
        auto curr_arg = Evaluate(cell->GetFirst(), env);
        if (!Is<Number>(curr_arg)) {
            throw RuntimeError("Invalid arguments: expected numbers");
        }
        int curr_value = As<Number>(curr_arg)->GetValue();

        if (prev_value > curr_value) {
            return std::make_shared<BooleanFalse>();
        }
        prev_value = curr_value;
        rest = cell->GetSecond();
    }
    return std::make_shared<BooleanTrue>();
}

//////// IntegerPlus
IntegerPlus::IntegerPlus() : Symbol("+") {
}

std::shared_ptr<Object> IntegerPlus::Do(std::shared_ptr<Object> args,
                                        std::shared_ptr<Environment> env) {
    int sum = 0;
    while (args) {
        if (!Is<Cell>(args)) {
            throw RuntimeError("Invalid arguments");
        }
        auto cell = As<Cell>(args);
        auto arg = Evaluate(cell->GetFirst(), env);
        if (!Is<Number>(arg)) {
            throw RuntimeError("Invalid arguments: expected numbers");
        }
        sum += As<Number>(arg)->GetValue();
        args = cell->GetSecond();
    }
    return std::make_shared<Number>(sum);
}

//////// IntegerMinus
IntegerMinus::IntegerMinus() : Symbol("-") {
}

std::shared_ptr<Object> IntegerMinus::Do(std::shared_ptr<Object> args,
                                         std::shared_ptr<Environment> env) {
    if (!args || !Is<Cell>(args)) {
        throw RuntimeError("Invalid arguments");
    }

    auto cell = As<Cell>(args);
    auto first_arg = Evaluate(cell->GetFirst(), env);
    if (!Is<Number>(first_arg)) {
        throw RuntimeError("Invalid arguments: expected numbers");
    }
    int result = As<Number>(first_arg)->GetValue();

    auto rest = cell->GetSecond();
    if (!rest) {
        // Unary minus
        return std::make_shared<Number>(-result);
    }

    while (rest) {
        if (!Is<Cell>(rest)) {
            throw RuntimeError("Invalid arguments");
        }
        cell = As<Cell>(rest);
        auto next_arg = Evaluate(cell->GetFirst(), env);
        if (!Is<Number>(next_arg)) {
            throw RuntimeError("Invalid arguments: expected numbers");
        }
        result -= As<Number>(next_arg)->GetValue();
        rest = cell->GetSecond();
    }
    return std::make_shared<Number>(result);
}

//////// IntegerMultiply
IntegerMultiply::IntegerMultiply() : Symbol("*") {
}

std::shared_ptr<Object> IntegerMultiply::Do(std::shared_ptr<Object> args,
                                            std::shared_ptr<Environment> env) {
    int product = 1;
    while (args) {
        if (!Is<Cell>(args)) {
            throw RuntimeError("Invalid arguments");
        }
        auto cell = As<Cell>(args);
        auto arg = Evaluate(cell->GetFirst(), env);
        if (!Is<Number>(arg)) {
            throw RuntimeError("Invalid arguments: expected numbers");
        }
        product *= As<Number>(arg)->GetValue();
        args = cell->GetSecond();
    }
    return std::make_shared<Number>(product);
}

//////// IntegerDivide
IntegerDivide::IntegerDivide() : Symbol("/") {
}

std::shared_ptr<Object> IntegerDivide::Do(std::shared_ptr<Object> args,
                                          std::shared_ptr<Environment> env) {
    if (!args || !Is<Cell>(args)) {
        throw RuntimeError("Invalid arguments");
    }

    auto cell = As<Cell>(args);
    auto first_arg = Evaluate(cell->GetFirst(), env);
    if (!Is<Number>(first_arg)) {
        throw RuntimeError("Invalid arguments: expected numbers");
    }
    int result = As<Number>(first_arg)->GetValue();

    auto rest = cell->GetSecond();
    if (!rest) {
        // One argument: reciprocal
        if (result == 0) {
            throw RuntimeError("Division by zero");
        }
        return std::make_shared<Number>(1 / result);
    }

    while (rest) {
        if (!Is<Cell>(rest)) {
            throw RuntimeError("Invalid arguments");
        }
        cell = As<Cell>(rest);
        auto next_arg = Evaluate(cell->GetFirst(), env);
        if (!Is<Number>(next_arg)) {
            throw RuntimeError("Invalid arguments: expected numbers");
        }
        int divisor = As<Number>(next_arg)->GetValue();
        if (divisor == 0) {
            throw RuntimeError("Division by zero");
        }
        result /= divisor;
        rest = cell->GetSecond();
    }
    return std::make_shared<Number>(result);
}

//////// IntegerMax
IntegerMax::IntegerMax() : Symbol("max") {
}

std::shared_ptr<Object> IntegerMax::Do(std::shared_ptr<Object> args,
                                       std::shared_ptr<Environment> env) {
    if (!args) {
        throw RuntimeError("max requires at least one argument");
    }

    int max_value = std::numeric_limits<int>::min();
    while (args) {
        if (!Is<Cell>(args)) {
            throw RuntimeError("Invalid arguments");
        }
        auto cell = As<Cell>(args);
        auto arg = Evaluate(cell->GetFirst(), env);
        if (!Is<Number>(arg)) {
            throw RuntimeError("Invalid arguments: expected numbers");
        }
        int value = As<Number>(arg)->GetValue();
        if (value > max_value) {
            max_value = value;
        }
        args = cell->GetSecond();
    }
    return std::make_shared<Number>(max_value);
}

//////// IntegerMin
IntegerMin::IntegerMin() : Symbol("min") {
}

std::shared_ptr<Object> IntegerMin::Do(std::shared_ptr<Object> args,
                                       std::shared_ptr<Environment> env) {
    if (!args) {
        throw RuntimeError("min requires at least one argument");
    }

    int min_value = std::numeric_limits<int>::max();
    while (args) {
        if (!Is<Cell>(args)) {
            throw RuntimeError("Invalid arguments");
        }
        auto cell = As<Cell>(args);
        auto arg = Evaluate(cell->GetFirst(), env);
        if (!Is<Number>(arg)) {
            throw RuntimeError("Invalid arguments: expected numbers");
        }
        int value = As<Number>(arg)->GetValue();
        if (value < min_value) {
            min_value = value;
        }
        args = cell->GetSecond();
    }
    return std::make_shared<Number>(min_value);
}

//////// IntegerAbs

IntegerAbs::IntegerAbs() : Symbol("abs") {
}

std::shared_ptr<Object> IntegerAbs::Do(std::shared_ptr<Object> args,
                                       std::shared_ptr<Environment> env) {
    if (!args || !Is<Cell>(args)) {
        throw RuntimeError("Invalid arguments");
    }

    auto cell = As<Cell>(args);
    auto arg = Evaluate(cell->GetFirst(), env);
    if (!Is<Number>(arg)) {
        throw RuntimeError("Invalid arguments: expected a number");
    }
    if (cell->GetSecond()) {
        throw RuntimeError("abs takes exactly one argument");
    }
    int value = As<Number>(arg)->GetValue();
    return std::make_shared<Number>(std::abs(value));
}

//////////////////// Booleans
//////// BooleanPredicate
BooleanPredicate::BooleanPredicate() : Symbol("boolean?") {
}

std::shared_ptr<Object> BooleanPredicate::Do(std::shared_ptr<Object> args,
                                             std::shared_ptr<Environment> env) {
    if (!args || !Is<Cell>(args)) {
        throw RuntimeError("Invalid arguments");
    }
    auto cell = As<Cell>(args);
    if (cell->GetSecond()) {
        throw RuntimeError("boolean? takes exactly one argument");
    }
    auto arg = Evaluate(cell->GetFirst(), env);
    if (Is<RealBoolean>(arg)) {
        return std::make_shared<BooleanTrue>();
    } else {
        return std::make_shared<BooleanFalse>();
    }
}

//////// BooleanNot
BooleanNot::BooleanNot() : Symbol("not") {
}

std::shared_ptr<Object> BooleanNot::Do(std::shared_ptr<Object> args,
                                       std::shared_ptr<Environment> env) {
    if (!args || !Is<Cell>(args)) {
        throw RuntimeError("Invalid arguments");
    }
    auto cell = As<Cell>(args);
    if (cell->GetSecond()) {
        throw RuntimeError("not takes exactly one argument");
    }
    auto arg = Evaluate(cell->GetFirst(), env);
    if (Is<BooleanFalse>(arg)) {
        return std::make_shared<BooleanTrue>();
    } else {
        return std::make_shared<BooleanFalse>();
    }
}

//////// BooleanAnd
BooleanAnd::BooleanAnd() : Symbol("and") {
}

std::shared_ptr<Object> BooleanAnd::Do(std::shared_ptr<Object> args,
                                       std::shared_ptr<Environment> env) {
    if (!args) {
        return std::make_shared<BooleanTrue>();
    }
    while (args) {
        if (!Is<Cell>(args)) {
            throw RuntimeError("Invalid arguments");
        }
        auto cell = As<Cell>(args);
        auto arg = Evaluate(cell->GetFirst(), env);
        if (Is<BooleanFalse>(arg)) {
            return arg;
        }
        if (!cell->GetSecond()) {
            return arg;
        }
        args = cell->GetSecond();
    }
    return std::make_shared<BooleanTrue>();
}

//////// BooleanOr
BooleanOr::BooleanOr() : Symbol("or") {
}

std::shared_ptr<Object> BooleanOr::Do(std::shared_ptr<Object> args,
                                      std::shared_ptr<Environment> env) {
    if (!args) {
        return std::make_shared<BooleanFalse>();
    }
    while (args) {
        if (!Is<Cell>(args)) {
            throw RuntimeError("Invalid arguments");
        }
        auto cell = As<Cell>(args);
        auto arg = Evaluate(cell->GetFirst(), env);
        if (!Is<BooleanFalse>(arg)) {
            return arg;
        }
        args = cell->GetSecond();
    }
    return std::make_shared<BooleanFalse>();
}

//////// PairPredicate
PairPredicate::PairPredicate() : Symbol("pair?") {
}
std::shared_ptr<Object> PairPredicate::Do(std::shared_ptr<Object> args,
                                          std::shared_ptr<Environment> env) {
    if (!args || !Is<Cell>(args) || As<Cell>(args)->GetSecond()) {
        throw RuntimeError("pair? expects exactly one argument");
    }
    auto arg = Evaluate(As<Cell>(args)->GetFirst(), env);
    if (Is<Cell>(arg)) {
        return std::make_shared<BooleanTrue>();
    } else {
        return std::make_shared<BooleanFalse>();
    }
}

//////// NullPredicate
NullPredicate::NullPredicate() : Symbol("null?") {
}

std::shared_ptr<Object> NullPredicate::Do(std::shared_ptr<Object> args,
                                          std::shared_ptr<Environment> env) {
    if (!args || !Is<Cell>(args) || As<Cell>(args)->GetSecond()) {
        throw RuntimeError("null? expects exactly one argument");
    }
    auto arg = Evaluate(As<Cell>(args)->GetFirst(), env);
    if (!arg) {
        return std::make_shared<BooleanTrue>();
    } else {
        return std::make_shared<BooleanFalse>();
    }
}

//////// ListPredicate
ListPredicate::ListPredicate() : Symbol("list?") {
}

std::shared_ptr<Object> ListPredicate::Do(std::shared_ptr<Object> args,
                                          std::shared_ptr<Environment> env) {
    if (!args || !Is<Cell>(args) || As<Cell>(args)->GetSecond()) {
        throw RuntimeError("list? expects exactly one argument");
    }
    auto arg = Evaluate(As<Cell>(args)->GetFirst(), env);
    // Проверяем, является ли аргумент корректным списком
    auto current = arg;
    while (current) {
        if (Is<Cell>(current)) {
            current = As<Cell>(current)->GetSecond();
        } else {
            return std::make_shared<BooleanFalse>();
        }
    }
    return std::make_shared<BooleanTrue>();
}

//////// Cons
Cons::Cons() : Symbol("cons") {
}

std::shared_ptr<Object> Cons::Do(std::shared_ptr<Object> args, std::shared_ptr<Environment> env) {
    if (!args || !Is<Cell>(args)) {
        throw RuntimeError("cons expects exactly two arguments");
    }
    auto first_cell = As<Cell>(args);
    auto rest = first_cell->GetSecond();
    if (!rest || !Is<Cell>(rest) || As<Cell>(rest)->GetSecond()) {
        throw RuntimeError("cons expects exactly two arguments");
    }
    auto first_arg = Evaluate(first_cell->GetFirst(), env);
    auto second_arg = Evaluate(As<Cell>(rest)->GetFirst(), env);
    auto new_cell = std::make_shared<Cell>();
    new_cell->SetFirst(first_arg);
    new_cell->SetSecond(second_arg);
    return new_cell;
}

//////// Car
Car::Car() : Symbol("car") {
}

std::shared_ptr<Object> Car::Do(std::shared_ptr<Object> args, std::shared_ptr<Environment> env) {
    if (!args || !Is<Cell>(args) || As<Cell>(args)->GetSecond()) {
        throw RuntimeError("car expects exactly one argument");
    }
    auto arg = Evaluate(As<Cell>(args)->GetFirst(), env);
    if (!Is<Cell>(arg)) {
        throw RuntimeError("car expects a pair");
    }
    return As<Cell>(arg)->GetFirst();
}

//////// Cdr
Cdr::Cdr() : Symbol("cdr") {
}

std::shared_ptr<Object> Cdr::Do(std::shared_ptr<Object> args, std::shared_ptr<Environment> env) {
    if (!args || !Is<Cell>(args) || As<Cell>(args)->GetSecond()) {
        throw RuntimeError("cdr expects exactly one argument");
    }
    auto arg = Evaluate(As<Cell>(args)->GetFirst(), env);
    if (!Is<Cell>(arg)) {
        throw RuntimeError("cdr expects a pair");
    }
    return As<Cell>(arg)->GetSecond();
}

//////// List
List::List() : Symbol("list") {
}

std::shared_ptr<Object> List::Do(std::shared_ptr<Object> args, std::shared_ptr<Environment> env) {
    if (!args) {
        return nullptr;
    }
    auto head = std::make_shared<Cell>();
    auto current_cell = head;
    auto rest = args;
    while (rest) {
        if (!Is<Cell>(rest)) {
            throw RuntimeError("Invalid arguments to list");
        }
        auto cell = As<Cell>(rest);
        auto arg = Evaluate(cell->GetFirst(), env);
        current_cell->SetFirst(arg);
        rest = cell->GetSecond();
        if (rest) {
            auto next_cell = std::make_shared<Cell>();
            current_cell->SetSecond(next_cell);
            current_cell = next_cell;
        } else {
            current_cell->SetSecond(nullptr);
        }
    }
    return head;
}

//////// ListRef
ListRef::ListRef() : Symbol("list-ref") {
}

std::shared_ptr<Object> ListRef::Do(std::shared_ptr<Object> args,
                                    std::shared_ptr<Environment> env) {
    if (!args || !Is<Cell>(args)) {
        throw RuntimeError("list-ref expects exactly two arguments");
    }
    auto cell = As<Cell>(args);
    auto list_arg = Evaluate(cell->GetFirst(), env);
    auto rest = cell->GetSecond();
    if (!rest || !Is<Cell>(rest) || As<Cell>(rest)->GetSecond()) {
        throw RuntimeError("list-ref expects exactly two arguments");
    }
    auto index_arg = Evaluate(As<Cell>(rest)->GetFirst(), env);
    if (!Is<Number>(index_arg)) {
        throw RuntimeError("list-ref expects a number as the second argument");
    }
    int index = As<Number>(index_arg)->GetValue();
    if (index < 0) {
        throw RuntimeError("list-ref index must be non-negative");
    }
    auto current = list_arg;
    while (index > 0) {
        if (!Is<Cell>(current)) {
            throw RuntimeError("list-ref index out of bounds");
        }
        current = As<Cell>(current)->GetSecond();
        --index;
    }
    if (!Is<Cell>(current)) {
        throw RuntimeError("list-ref index out of bounds");
    }
    return As<Cell>(current)->GetFirst();
}

//////// ListTail
ListTail::ListTail() : Symbol("list-tail") {
}

std::shared_ptr<Object> ListTail::Do(std::shared_ptr<Object> args,
                                     std::shared_ptr<Environment> env) {
    if (!args || !Is<Cell>(args)) {
        throw RuntimeError("list-tail expects exactly two arguments");
    }
    auto cell = As<Cell>(args);
    auto list_arg = Evaluate(cell->GetFirst(), env);
    auto rest = cell->GetSecond();
    if (!rest || !Is<Cell>(rest) || As<Cell>(rest)->GetSecond()) {
        throw RuntimeError("list-tail expects exactly two arguments");
    }
    auto index_arg = Evaluate(As<Cell>(rest)->GetFirst(), env);
    if (!Is<Number>(index_arg)) {
        throw RuntimeError("list-tail expects a number as the second argument");
    }
    int index = As<Number>(index_arg)->GetValue();
    if (index < 0) {
        throw RuntimeError("list-tail index must be non-negative");
    }
    auto current = list_arg;
    while (index > 0) {
        if (!Is<Cell>(current)) {
            throw RuntimeError("list-tail index out of bounds");
        }
        current = As<Cell>(current)->GetSecond();
        --index;
    }
    return current;
}
//////////////////// Advanced
//////// If
If::If() : Symbol("if") {
}

std::shared_ptr<Object> If::Do(std::shared_ptr<Object> args, std::shared_ptr<Environment> env) {
    if (!args || !Is<Cell>(args)) {
        throw SyntaxError("if expects two or three arguments");
    }
    auto first_cell = As<Cell>(args);
    auto condition = first_cell->GetFirst();
    auto rest = first_cell->GetSecond();
    if (!rest || !Is<Cell>(rest)) {
        throw SyntaxError("if expects two or three arguments");
    }
    auto rest_as_cell = As<Cell>(rest);
    std::shared_ptr<Object> false_value;
    std::shared_ptr<Object> true_value = rest_as_cell->GetFirst();
    if (auto second = As<Cell>(rest)->GetSecond()) {
        if (!Is<Cell>(second)) {
            throw RuntimeError("if expects two or three arguments");
        }
        auto second_as_cell = As<Cell>(second);
        false_value = second_as_cell->GetFirst();
        if (second_as_cell->GetSecond()) {
            throw SyntaxError("if expects two or three arguments");
        }
    } else {
        false_value = nullptr;
    }
    auto cond = Evaluate(condition, env);
    if (Is<RealBoolean>(cond)) {
        if (As<RealBoolean>(cond)->GetBoolean()) {
            return Evaluate(true_value, env);
        }
        return false_value ? Evaluate(false_value, env) : nullptr;
    } else {
        throw RuntimeError("condition is not boolean");
    }
}

//////// Define
Define::Define() : Symbol("define") {
}

std::shared_ptr<Object> Define::Do(std::shared_ptr<Object> args, std::shared_ptr<Environment> env) {
    if (!args || !Is<Cell>(args)) {
        throw SyntaxError("define expects more than 1 argument");
    }
    auto cell = As<Cell>(args);
    auto variable = cell->GetFirst();
    auto obj = cell->GetSecond();
    if (!obj || !Is<Cell>(obj)) {
        throw SyntaxError("define expects body");
    }
    if (Is<Symbol>(variable)) {
        if (As<Cell>(obj)->GetSecond()) {
            throw SyntaxError("define expects 1 argument as an argument");
        }
        std::string name = As<Symbol>(variable)->GetName();
        auto var = Evaluate(As<Cell>(obj)->GetFirst(), env);
        std::shared_ptr<Object> sp;
        if (Is<Symbol>(var) && !env->Get(name, sp) && As<Symbol>(var)->GetName() == name) {
            throw NameError("Cannot assign to itself");
        }
        env->Define(name, var);
    } else if (Is<Cell>(variable)) {
        auto lambda_name = As<Cell>(variable)->GetFirst();
        if (!Is<Symbol>(lambda_name)) {
            throw RuntimeError("In lambda: not a symbol");
        }
        std::string name = As<Symbol>(lambda_name)->GetName();
        auto first = As<Cell>(variable)->GetSecond();
        std::vector<std::string> variables;
        std::vector<std::shared_ptr<Object>> ops;
        while (first) {
            if (!Is<Cell>(first)) {
                throw SyntaxError("Expected more than 2 arguments for lambda expression");
            }
            auto variables_cell = As<Cell>(first);
            if (!Is<Variable>(variables_cell->GetFirst())) {
                throw RuntimeError("In lambda: not a variable");
            }
            variables.push_back(As<Variable>(variables_cell->GetFirst())->GetName());
            first = variables_cell->GetSecond();
        }

        if (!obj) {
            throw SyntaxError("Expected more than 2 arguments in lambda expression");
        }
        while (obj) {
            if (!Is<Cell>(obj)) {
                throw SyntaxError("Expected list");
            }
            auto ops_cell = As<Cell>(obj);
            auto operation = ops_cell->GetFirst();
            ops.push_back(operation);
            obj = ops_cell->GetSecond();
        }
        auto var = std::make_shared<MyLambda>(variables, ops, env, name);
        env->Define(name, var);
    } else {
        throw SyntaxError("Not a variable or cell");
    }
    return nullptr;
}

//////// Set
Set::Set() : Symbol("set!") {
}

std::shared_ptr<Object> Set::Do(std::shared_ptr<Object> args, std::shared_ptr<Environment> env) {
    if (!args || !Is<Cell>(args)) {
        throw SyntaxError("set! expects exactly two arguments");
    }
    auto cell = As<Cell>(args);
    auto variable = cell->GetFirst();
    auto obj = cell->GetSecond();
    if (!obj || !Is<Cell>(obj) || As<Cell>(obj)->GetSecond()) {
        throw SyntaxError("set! expects exactly two arguments");
    }
    if (!Is<Variable>(variable)) {
        throw RuntimeError("Not a variable");
    }
    std::string name = As<Variable>(variable)->GetName();
    auto var = Evaluate(As<Cell>(obj)->GetFirst(), env);
    env->Set(name, var);
    return nullptr;
}

//////// SetCar
SetCar::SetCar() : Symbol("set-car!") {
}

std::shared_ptr<Object> SetCar::Do(std::shared_ptr<Object> args, std::shared_ptr<Environment> env) {
    if (!args || !Is<Cell>(args)) {
        throw RuntimeError("set-car! expects exactly two arguments");
    }
    auto cell = As<Cell>(args);
    auto pair = Evaluate(cell->GetFirst(), env);
    if (!Is<Cell>(cell->GetSecond()) || As<Cell>(cell->GetSecond())->GetSecond()) {
        throw RuntimeError("set-car! expects exactly two arguments");
    }
    auto obj = Evaluate(As<Cell>(cell->GetSecond())->GetFirst(), env);
    if (!obj) {
        throw RuntimeError("set-car! expects exactly two arguments");
    }
    if (!Is<Cell>(pair)) {
        throw RuntimeError("Variable is not a cell");
    }
    As<Cell>(pair)->SetFirst(obj);
    return nullptr;
}

//////// SetCdr
SetCdr::SetCdr() : Symbol("set-cdr!") {
}

std::shared_ptr<Object> SetCdr::Do(std::shared_ptr<Object> args, std::shared_ptr<Environment> env) {
    if (!args || !Is<Cell>(args)) {
        throw RuntimeError("set-car! expects exactly two arguments");
    }
    auto cell = As<Cell>(args);
    auto pair = Evaluate(cell->GetFirst(), env);
    if (!Is<Cell>(cell->GetSecond()) || As<Cell>(cell->GetSecond())->GetSecond()) {
        throw RuntimeError("set-car! expects exactly two arguments");
    }
    auto obj = Evaluate(As<Cell>(cell->GetSecond())->GetFirst(), env);
    if (!obj) {
        throw RuntimeError("set-car! expects exactly two arguments");
    }
    if (!Is<Cell>(pair)) {
        throw RuntimeError("Variable is not a cell");
    }
    As<Cell>(pair)->SetSecond(obj);
    return nullptr;
}

//////// SymbolPredicate
SymbolPredicate::SymbolPredicate() : Symbol("symbol?") {
}

std::shared_ptr<Object> SymbolPredicate::Do(std::shared_ptr<Object> args,
                                            std::shared_ptr<Environment> env) {
    if (!args || !Is<Cell>(args)) {
        throw RuntimeError("Invalid arguments");
    }
    auto cell = As<Cell>(args);
    if (cell->GetSecond()) {
        throw RuntimeError("symbol? takes exactly one argument");
    }
    auto arg = Evaluate(cell->GetFirst(), env);
    if (Is<Symbol>(arg)) {
        return std::make_shared<BooleanTrue>();
    } else {
        return std::make_shared<BooleanFalse>();
    }
}

//////// Lambda

Lambda::Lambda() : Symbol("lambda") {
}

std::shared_ptr<Object> Lambda::Do(std::shared_ptr<Object> args, std::shared_ptr<Environment> env) {
    if (!args || !Is<Cell>(args)) {
        throw SyntaxError("Expected more than 2 arguments in lambda expression");
    }

    auto lambda_cell = As<Cell>(args);
    auto first = lambda_cell->GetFirst();

    std::vector<std::string> variables;
    std::vector<std::shared_ptr<Object>> ops;
    while (first) {
        if (!Is<Cell>(first)) {
            throw SyntaxError("Expected more than 2 arguments for lambda expression");
        }
        auto variables_cell = As<Cell>(first);
        if (!Is<Variable>(variables_cell->GetFirst())) {
            throw RuntimeError("In lambda: not a variable");
        }
        variables.push_back(As<Variable>(variables_cell->GetFirst())->GetName());
        first = variables_cell->GetSecond();
    }

    auto operations = lambda_cell->GetSecond();
    if (!operations) {
        throw SyntaxError("Expected more than 2 arguments in lambda expression");
    }
    while (operations) {
        if (!Is<Cell>(operations)) {
            throw SyntaxError("Expected list");
        }
        auto cell = As<Cell>(operations);
        auto operation = cell->GetFirst();
        ops.push_back(operation);
        operations = cell->GetSecond();
    }
    return std::make_shared<MyLambda>(variables, ops, env);
}

//////// MyLambda
MyLambda::MyLambda(const std::vector<std::string>& variables,
                   const std::vector<std::shared_ptr<Object>>& ops,
                   std::shared_ptr<Environment> env, std::string name = "")
    : Symbol(name), vars_(variables), ops_(ops), closure_env_(env) {
}

std::shared_ptr<Object> MyLambda::Do(std::shared_ptr<Object> args,
                                     std::shared_ptr<Environment> env) {
    auto local_env = std::make_shared<Environment>(closure_env_);

    auto arg_list = args;
    for (const auto& param : vars_) {
        if (!Is<Cell>(arg_list)) {
            throw RuntimeError("Insufficient arguments for lambda");
        }
        auto cell = As<Cell>(arg_list);
        auto value = Evaluate(cell->GetFirst(), env);
        local_env->Define(param, value);
        arg_list = cell->GetSecond();
    }

    if (arg_list) {
        throw RuntimeError("Too many arguments for lambda");
    }
    std::shared_ptr<Object> result;
    for (const auto& expr : ops_) {
        result = Evaluate(expr, local_env);
    }
    return result;
}

//////////////////// Cell
Cell::Cell() : first_(nullptr), second_(nullptr) {
}

void Cell::SetFirst(std::shared_ptr<Object> f) {
    first_ = f;
}

void Cell::SetSecond(std::shared_ptr<Object> s) {
    second_ = s;
}

std::shared_ptr<Object> Cell::GetFirst() const {
    return first_;
}

std::shared_ptr<Object> Cell::GetSecond() const {
    return second_;
}