#include <object.h>
#include <algorithm>
#include <limits>

std::shared_ptr<Object> Evaluate(std::shared_ptr<Object> obj) {
    if (!obj) {
        throw RuntimeError("Cannot evaluate empty list");
    } else if (Is<Number>(obj) || Is<RealBoolean>(obj)) {
        return obj;
    } else if (Is<Symbol>(obj)) {
        // Handle variable lookup here
        return obj;
    } else if (Is<Cell>(obj)) {
        auto cell = As<Cell>(obj);
        auto first = cell->GetFirst();
        auto second = cell->GetSecond();

        auto func = Evaluate(first);

        if (Is<Symbol>(func)) {
            return As<Symbol>(func)->Do(second, true);
        } else {
            throw RuntimeError("First element is not a function");
        }
    } else {
        throw RuntimeError("Unknown expression type");
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

//////// InvalidSymbol
InvalidSymbol::InvalidSymbol(std::string name) : Symbol(name) {
}

std::shared_ptr<Object> InvalidSymbol::Do(std::shared_ptr<Object>, bool) {
    throw NameError("Not a valid operation");
}
//////////////////// Quote

Quote::Quote(bool is_symbol) : Symbol("quote"), is_symbol_(is_symbol) {
}

std::shared_ptr<Object> Quote::Do(std::shared_ptr<Object> args, bool) {
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

std::shared_ptr<Object> IntegerPredicate::Do(std::shared_ptr<Object> args, bool) {
    if (!args || !Is<Cell>(args)) {
        throw RuntimeError("Invalid arguments");
    }
    auto cell = As<Cell>(args);
    if (cell->GetSecond()) {
        throw RuntimeError("number? takes exactly one argument");
    }
    auto arg = Evaluate(cell->GetFirst());
    if (Is<Number>(arg)) {
        return std::make_shared<BooleanTrue>();
    } else {
        return std::make_shared<BooleanFalse>();
    }
}

//////// IntegerEqual
IntegerEqual::IntegerEqual() : Symbol("=") {
}

std::shared_ptr<Object> IntegerEqual::Do(std::shared_ptr<Object> args, bool) {
    if (!args) {
        return std::make_shared<BooleanTrue>();
    }

    if (!Is<Cell>(args)) {
        throw RuntimeError("Invalid arguments");
    }

    auto cell = As<Cell>(args);
    auto first_arg = Evaluate(cell->GetFirst());
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
        auto next_arg = Evaluate(cell->GetFirst());
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

std::shared_ptr<Object> IntegerMore::Do(std::shared_ptr<Object> args, bool) {
    if (!args) {
        return std::make_shared<BooleanTrue>();
    }

    if (!Is<Cell>(args)) {
        throw RuntimeError("Invalid arguments");
    }

    auto cell = As<Cell>(args);
    auto prev_arg = Evaluate(cell->GetFirst());
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
        auto curr_arg = Evaluate(cell->GetFirst());
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

std::shared_ptr<Object> IntegerLess::Do(std::shared_ptr<Object> args, bool) {
    if (!args) {
        return std::make_shared<BooleanTrue>();
    }

    if (!Is<Cell>(args)) {
        throw RuntimeError("Invalid arguments");
    }

    auto cell = As<Cell>(args);
    auto prev_arg = Evaluate(cell->GetFirst());
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
        auto curr_arg = Evaluate(cell->GetFirst());
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

std::shared_ptr<Object> IntegerMoreEqual::Do(std::shared_ptr<Object> args, bool) {
    if (!args) {
        return std::make_shared<BooleanTrue>();
    }

    if (!Is<Cell>(args)) {
        throw RuntimeError("Invalid arguments");
    }

    auto cell = As<Cell>(args);
    auto prev_arg = Evaluate(cell->GetFirst());
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
        auto curr_arg = Evaluate(cell->GetFirst());
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

std::shared_ptr<Object> IntegerLessEqual::Do(std::shared_ptr<Object> args, bool) {
    if (!args) {
        return std::make_shared<BooleanTrue>();
    }

    if (!Is<Cell>(args)) {
        throw RuntimeError("Invalid arguments");
    }

    auto cell = As<Cell>(args);
    auto prev_arg = Evaluate(cell->GetFirst());
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
        auto curr_arg = Evaluate(cell->GetFirst());
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

std::shared_ptr<Object> IntegerPlus::Do(std::shared_ptr<Object> args, bool) {
    int sum = 0;
    while (args) {
        if (!Is<Cell>(args)) {
            throw RuntimeError("Invalid arguments");
        }
        auto cell = As<Cell>(args);
        auto arg = Evaluate(cell->GetFirst());
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

std::shared_ptr<Object> IntegerMinus::Do(std::shared_ptr<Object> args, bool) {
    if (!args || !Is<Cell>(args)) {
        throw RuntimeError("Invalid arguments");
    }

    auto cell = As<Cell>(args);
    auto first_arg = Evaluate(cell->GetFirst());
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
        auto next_arg = Evaluate(cell->GetFirst());
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

std::shared_ptr<Object> IntegerMultiply::Do(std::shared_ptr<Object> args, bool) {
    int product = 1;
    while (args) {
        if (!Is<Cell>(args)) {
            throw RuntimeError("Invalid arguments");
        }
        auto cell = As<Cell>(args);
        auto arg = Evaluate(cell->GetFirst());
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

std::shared_ptr<Object> IntegerDivide::Do(std::shared_ptr<Object> args, bool) {
    if (!args || !Is<Cell>(args)) {
        throw RuntimeError("Invalid arguments");
    }

    auto cell = As<Cell>(args);
    auto first_arg = Evaluate(cell->GetFirst());
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
        auto next_arg = Evaluate(cell->GetFirst());
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

std::shared_ptr<Object> IntegerMax::Do(std::shared_ptr<Object> args, bool) {
    if (!args) {
        throw RuntimeError("max requires at least one argument");
    }

    int max_value = std::numeric_limits<int>::min();
    while (args) {
        if (!Is<Cell>(args)) {
            throw RuntimeError("Invalid arguments");
        }
        auto cell = As<Cell>(args);
        auto arg = Evaluate(cell->GetFirst());
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

std::shared_ptr<Object> IntegerMin::Do(std::shared_ptr<Object> args, bool) {
    if (!args) {
        throw RuntimeError("min requires at least one argument");
    }

    int min_value = std::numeric_limits<int>::max();
    while (args) {
        if (!Is<Cell>(args)) {
            throw RuntimeError("Invalid arguments");
        }
        auto cell = As<Cell>(args);
        auto arg = Evaluate(cell->GetFirst());
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

std::shared_ptr<Object> IntegerAbs::Do(std::shared_ptr<Object> args, bool) {
    if (!args || !Is<Cell>(args)) {
        throw RuntimeError("Invalid arguments");
    }

    auto cell = As<Cell>(args);
    auto arg = Evaluate(cell->GetFirst());
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

std::shared_ptr<Object> BooleanPredicate::Do(std::shared_ptr<Object> args, bool) {
    if (!args || !Is<Cell>(args)) {
        throw RuntimeError("Invalid arguments");
    }
    auto cell = As<Cell>(args);
    if (cell->GetSecond()) {
        throw RuntimeError("boolean? takes exactly one argument");
    }
    auto arg = Evaluate(cell->GetFirst());
    if (Is<RealBoolean>(arg)) {
        return std::make_shared<BooleanTrue>();
    } else {
        return std::make_shared<BooleanFalse>();
    }
}

//////// BooleanNot
BooleanNot::BooleanNot() : Symbol("not") {
}

std::shared_ptr<Object> BooleanNot::Do(std::shared_ptr<Object> args, bool) {
    if (!args || !Is<Cell>(args)) {
        throw RuntimeError("Invalid arguments");
    }
    auto cell = As<Cell>(args);
    if (cell->GetSecond()) {
        throw RuntimeError("not takes exactly one argument");
    }
    auto arg = Evaluate(cell->GetFirst());
    if (Is<BooleanFalse>(arg)) {
        return std::make_shared<BooleanTrue>();
    } else {
        return std::make_shared<BooleanFalse>();
    }
}

//////// BooleanAnd
BooleanAnd::BooleanAnd() : Symbol("and") {
}

std::shared_ptr<Object> BooleanAnd::Do(std::shared_ptr<Object> args, bool) {
    if (!args) {
        return std::make_shared<BooleanTrue>();
    }
    while (args) {
        if (!Is<Cell>(args)) {
            throw RuntimeError("Invalid arguments");
        }
        auto cell = As<Cell>(args);
        auto arg = Evaluate(cell->GetFirst());
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

std::shared_ptr<Object> BooleanOr::Do(std::shared_ptr<Object> args, bool) {
    if (!args) {
        return std::make_shared<BooleanFalse>();
    }
    while (args) {
        if (!Is<Cell>(args)) {
            throw RuntimeError("Invalid arguments");
        }
        auto cell = As<Cell>(args);
        auto arg = Evaluate(cell->GetFirst());
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
std::shared_ptr<Object> PairPredicate::Do(std::shared_ptr<Object> args, bool) {
    if (!args || !Is<Cell>(args) || As<Cell>(args)->GetSecond()) {
        throw RuntimeError("pair? expects exactly one argument");
    }
    auto arg = Evaluate(As<Cell>(args)->GetFirst());
    if (Is<Cell>(arg)) {
        return std::make_shared<BooleanTrue>();
    } else {
        return std::make_shared<BooleanFalse>();
    }
}

//////// NullPredicate
NullPredicate::NullPredicate() : Symbol("null?") {
}

std::shared_ptr<Object> NullPredicate::Do(std::shared_ptr<Object> args, bool) {
    if (!args || !Is<Cell>(args) || As<Cell>(args)->GetSecond()) {
        throw RuntimeError("null? expects exactly one argument");
    }
    auto arg = Evaluate(As<Cell>(args)->GetFirst());
    if (!arg) {
        return std::make_shared<BooleanTrue>();
    } else {
        return std::make_shared<BooleanFalse>();
    }
}

//////// ListPredicate
ListPredicate::ListPredicate() : Symbol("list?") {
}

std::shared_ptr<Object> ListPredicate::Do(std::shared_ptr<Object> args, bool) {
    if (!args || !Is<Cell>(args) || As<Cell>(args)->GetSecond()) {
        throw RuntimeError("list? expects exactly one argument");
    }
    auto arg = Evaluate(As<Cell>(args)->GetFirst());
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

std::shared_ptr<Object> Cons::Do(std::shared_ptr<Object> args, bool) {
    if (!args || !Is<Cell>(args)) {
        throw RuntimeError("cons expects exactly two arguments");
    }
    auto first_cell = As<Cell>(args);
    auto rest = first_cell->GetSecond();
    if (!rest || !Is<Cell>(rest) || As<Cell>(rest)->GetSecond()) {
        throw RuntimeError("cons expects exactly two arguments");
    }
    auto first_arg = Evaluate(first_cell->GetFirst());
    auto second_arg = Evaluate(As<Cell>(rest)->GetFirst());
    auto new_cell = std::make_shared<Cell>();
    new_cell->SetFirst(first_arg);
    new_cell->SetSecond(second_arg);
    return new_cell;
}

//////// Car
Car::Car() : Symbol("car") {
}

std::shared_ptr<Object> Car::Do(std::shared_ptr<Object> args, bool) {
    if (!args || !Is<Cell>(args) || As<Cell>(args)->GetSecond()) {
        throw RuntimeError("car expects exactly one argument");
    }
    auto arg = Evaluate(As<Cell>(args)->GetFirst());
    if (!Is<Cell>(arg)) {
        throw RuntimeError("car expects a pair");
    }
    return As<Cell>(arg)->GetFirst();
}

//////// Cdr
Cdr::Cdr() : Symbol("cdr") {
}

std::shared_ptr<Object> Cdr::Do(std::shared_ptr<Object> args, bool) {
    if (!args || !Is<Cell>(args) || As<Cell>(args)->GetSecond()) {
        throw RuntimeError("cdr expects exactly one argument");
    }
    auto arg = Evaluate(As<Cell>(args)->GetFirst());
    if (!Is<Cell>(arg)) {
        throw RuntimeError("cdr expects a pair");
    }
    return As<Cell>(arg)->GetSecond();
}

//////// List
List::List() : Symbol("list") {
}

std::shared_ptr<Object> List::Do(std::shared_ptr<Object> args, bool) {
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
        auto arg = Evaluate(cell->GetFirst());
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

std::shared_ptr<Object> ListRef::Do(std::shared_ptr<Object> args, bool) {
    if (!args || !Is<Cell>(args)) {
        throw RuntimeError("list-ref expects exactly two arguments");
    }
    auto cell = As<Cell>(args);
    auto list_arg = Evaluate(cell->GetFirst());
    auto rest = cell->GetSecond();
    if (!rest || !Is<Cell>(rest) || As<Cell>(rest)->GetSecond()) {
        throw RuntimeError("list-ref expects exactly two arguments");
    }
    auto index_arg = Evaluate(As<Cell>(rest)->GetFirst());
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

std::shared_ptr<Object> ListTail::Do(std::shared_ptr<Object> args, bool) {
    if (!args || !Is<Cell>(args)) {
        throw RuntimeError("list-tail expects exactly two arguments");
    }
    auto cell = As<Cell>(args);
    auto list_arg = Evaluate(cell->GetFirst());
    auto rest = cell->GetSecond();
    if (!rest || !Is<Cell>(rest) || As<Cell>(rest)->GetSecond()) {
        throw RuntimeError("list-tail expects exactly two arguments");
    }
    auto index_arg = Evaluate(As<Cell>(rest)->GetFirst());
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