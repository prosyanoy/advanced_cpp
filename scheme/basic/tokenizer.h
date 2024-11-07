#pragma once

#include <variant>
#include <optional>
#include <istream>

struct EmptyToken {
    bool operator==(const EmptyToken&) const {
        return true;
    }
};

struct TrueToken {
    bool operator==(const TrueToken&) const {
        return true;
    }
};

struct FalseToken {
    bool operator==(const FalseToken&) const {
        return true;
    }
};

struct SymbolToken {
    std::string name;

    bool operator==(const SymbolToken& other) const {
        return name == other.name;
    }
};

struct QuoteToken {
    bool operator==(const QuoteToken&) const {
        return true;
    }
};

struct DotToken {
    bool operator==(const DotToken&) const {
        return true;
    }
};

enum class BracketToken { OPEN, CLOSE };

struct ConstantToken {
    int value;

    bool operator==(const ConstantToken& other) const {
        return value == other.value;
    }
};

using Token =
    std::variant<ConstantToken, BracketToken, SymbolToken, QuoteToken, DotToken,
                 TrueToken, FalseToken, EmptyToken>;

class Tokenizer {
    std::istream* i_;
    Token next_;

public:
    Tokenizer(std::istream* in);

    Token Get();

    bool IsEnd();

    void Next();

    Token GetToken();
};