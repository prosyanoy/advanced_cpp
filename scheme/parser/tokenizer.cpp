#include <tokenizer.h>
#include <error.h>

bool IsNumber(int c) {
    return c != EOF && c >= '0' && c <= '9';
}

bool IsSymbol(int c) {
    return c != EOF &&
           ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '+' || c == '-' || c == '.' ||
            c == '*' || c == '/' || c == '<' || c == '=' || c == '>' || c == '!' || c == '?' ||
            c == ':' || c == '$' || c == '%' || c == '_' || c == '&' || c == '~' || c == '^');
}

Tokenizer::Tokenizer(std::istream* in) : i_(in) {
    Next();
}

Token Tokenizer::Get() {
    int c;
    while ((c = i_->peek()) != EOF && (c == ' ' || c == '\n')) {
        i_->get();
    }
    c = i_->get();
    if (c == EOF) {
        return EmptyToken();
    }

    if (c == '\'') {
        return QuoteToken();
    } else if (c == '.') {
        return DotToken();
    } else if (c == '(') {
        return BracketToken::OPEN;
    } else if (c == ')') {
        return BracketToken::CLOSE;
    } else {
        if (c == ' ') {
            while ((c = i_->peek()) != EOF && (c == ' ' || c == '\n')) {
                i_->get();
            }
            c = i_->get();
            if (c == EOF) {
                return EmptyToken();
            }
        }
        if ((c == '-' && IsNumber(i_->peek())) || IsNumber(c)) {
            std::string number;
            number += static_cast<char>(c);
            while (IsNumber(i_->peek())) {
                number += static_cast<char>(i_->get());
            }
            return ConstantToken{std::stoi(number)};
        } else if (IsSymbol(c)) {
            std::string name;
            name += static_cast<char>(c);
            while (IsSymbol(i_->peek()) || IsNumber(i_->peek())) {
                name += static_cast<char>(i_->get());
            }
            return SymbolToken{name};
        } else {
            throw SyntaxError("Wrong token");
        }
    }
}

bool Tokenizer::IsEnd() {
    return std::get_if<EmptyToken>(&next_);
}

void Tokenizer::Next() {
    next_ = Get();
}

Token Tokenizer::GetToken() {
    if (EmptyToken* x = std::get_if<EmptyToken>(&next_)) {
        throw SyntaxError("No tokens left");
    }
    return next_;
}