#include <parser.h>

std::shared_ptr<Object> Read(Tokenizer* tokenizer) {
    if (tokenizer->IsEnd()) {
        throw SyntaxError("Unexpected end of input");
    }
    Token first_token = tokenizer->GetToken();
    if (BracketToken* x = std::get_if<BracketToken>(&first_token)) {
        if (*x == BracketToken::OPEN) {
            tokenizer->Next();
            return ReadList(tokenizer);
        } else {
            tokenizer->Next();
            return nullptr;
        }
    } else if (auto number_token = std::get_if<ConstantToken>(&first_token)) {
        tokenizer->Next();
        return std::make_shared<Number>(number_token->value);
    } else if (auto symbol_token = std::get_if<SymbolToken>(&first_token)) {
        tokenizer->Next();
        return std::make_shared<Symbol>(symbol_token->name);
    } else {
        throw SyntaxError("Incorrect first token");
    }
}

std::shared_ptr<Object> ReadList(Tokenizer* tokenizer) {
    if (tokenizer->IsEnd()) {
        throw SyntaxError("Unexpected end of input");
    }
    Token token = tokenizer->GetToken();
    if (BracketToken* x = std::get_if<BracketToken>(&token)) {
        if (*x == BracketToken::CLOSE) {
            tokenizer->Next();
            return nullptr;
        }
    }
    auto first = Read(tokenizer);
    auto root = std::make_shared<Cell>();
    root->SetFirst(first);
    if (tokenizer->IsEnd()) {
        throw SyntaxError("Unexpected end of input");
    }
    Token next_token = tokenizer->GetToken();
    if (BracketToken* x = std::get_if<BracketToken>(&next_token)) {
        if (*x == BracketToken::CLOSE) {
            tokenizer->Next();
            return root;
        }
    }
    if (std::get_if<DotToken>(&next_token)) {
        tokenizer->Next();
        root->SetSecond(Read(tokenizer));
        if (tokenizer->IsEnd()) {
            throw SyntaxError("Unexpected end of input");
        }
        next_token = tokenizer->GetToken();
        if (BracketToken* x = std::get_if<BracketToken>(&next_token)) {
            if (*x == BracketToken::CLOSE) {
                tokenizer->Next();
                return root;
            }
        }
        throw SyntaxError("Expected closing bracket after dotted pair");
    }
    root->SetSecond(ReadList(tokenizer));
    return root;
}
