#include <tokenizer.h>
#include <error.h>

bool IsNumber(char c) {
    return c != '\0' && c >= '0' && c <= '9';
}

bool IsSymbol(char c) {
    return c != '\0' &&
           ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '+' || c == '-' || c == '.' ||
            c == '*' || c == '/' || c == '<' || c == '=' || c == '>' || c == '!' || c == '?' ||
            c == ':' || c == '$' || c == '%' || c == '_' || c == '&' || c == '~' || c == '^');
}

Tokenizer::Tokenizer(std::istream* in) : i_(in) {
    Next();
}

Token Tokenizer::Get() {
    char c;
    while (i_->peek() == ' ' || i_->peek() == '\n') {
        i_->get();
    }
    i_->get(c);
    if (c == '\0') {
        return EmptyToken();
    } else if (c == '\'') {
        return QuoteToken();
    } else if (c == '.') {
        return DotToken();
    } else if (c == '(') {
        return BracketToken::OPEN;
    } else if (c == ')') {
        return BracketToken::CLOSE;
    } else {
        if (c == ' ') {
            while (i_->peek() == ' ' || i_->peek() == '\n') {
                i_->get();
            }
            i_->get(c);
            if (c == '\0') {
                return EmptyToken();
            }
        }
        if ((c == '-' && IsNumber(i_->peek())) || IsNumber(c)) {
            std::string number;
            number += c;
            while (IsNumber(i_->peek())) {
                char ch = i_->get();
                number += ch;
            }
            return ConstantToken{std::stoi(number)};
        } else if (IsSymbol(c)) {
            std::string name;
            name += c;
            while (IsSymbol(i_->peek()) || IsNumber(i_->peek())) {
                char ch = i_->get();
                name += ch;
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