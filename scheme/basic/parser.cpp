#include <parser.h>
#include <object.h>

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
            throw SyntaxError("Unexpected )");
        }
    } else if (auto number_token = std::get_if<ConstantToken>(&first_token)) {
        tokenizer->Next();
        return std::make_shared<Number>(number_token->value);
    } else if (std::get_if<TrueToken>(&first_token)) {
        tokenizer->Next();
        return std::make_shared<BooleanTrue>();
    } else if (std::get_if<FalseToken>(&first_token)) {
        tokenizer->Next();
        return std::make_shared<BooleanFalse>();
    } else if (auto symbol_token = std::get_if<SymbolToken>(&first_token)) {
        std::string symbol_name = symbol_token->name;
        tokenizer->Next();
        // Quote
        if (symbol_name == "quote") {
            return std::make_shared<Quote>(true);
            // Integer
        } else if (symbol_name == "number?") {
            return std::make_shared<IntegerPredicate>();
        } else if (symbol_name == "=") {
            return std::make_shared<IntegerEqual>();
        } else if (symbol_name == ">") {
            return std::make_shared<IntegerMore>();
        } else if (symbol_name == "<") {
            return std::make_shared<IntegerLess>();
        } else if (symbol_name == ">=") {
            return std::make_shared<IntegerMoreEqual>();
        } else if (symbol_name == "<=") {
            return std::make_shared<IntegerLessEqual>();
        } else if (symbol_name == "+") {
            return std::make_shared<IntegerPlus>();
        } else if (symbol_name == "-") {
            return std::make_shared<IntegerMinus>();
        } else if (symbol_name == "*") {
            return std::make_shared<IntegerMultiply>();
        } else if (symbol_name == "/") {
            return std::make_shared<IntegerDivide>();
        } else if (symbol_name == "max") {
            return std::make_shared<IntegerMax>();
        } else if (symbol_name == "min") {
            return std::make_shared<IntegerMin>();
        } else if (symbol_name == "abs") {
            return std::make_shared<IntegerAbs>();
            // Booleans
        } else if (symbol_name == "boolean?") {
            return std::make_shared<BooleanPredicate>();
        } else if (symbol_name == "not") {
            return std::make_shared<BooleanNot>();
        } else if (symbol_name == "and") {
            return std::make_shared<BooleanAnd>();
        } else if (symbol_name == "or") {
            return std::make_shared<BooleanOr>();
            // Lists
        } else if (symbol_name == "pair?") {
            return std::make_shared<PairPredicate>();
        } else if (symbol_name == "null?") {
            return std::make_shared<NullPredicate>();
        } else if (symbol_name == "list?") {
            return std::make_shared<ListPredicate>();
        } else if (symbol_name == "cons") {
            return std::make_shared<Cons>();
        } else if (symbol_name == "car") {
            return std::make_shared<Car>();
        } else if (symbol_name == "cdr") {
            return std::make_shared<Cdr>();
        } else if (symbol_name == "list") {
            return std::make_shared<List>();
        } else if (symbol_name == "list-ref") {
            return std::make_shared<ListRef>();
        } else if (symbol_name == "list-tail") {
            return std::make_shared<ListTail>();
        } else {
            return std::make_shared<InvalidSymbol>(symbol_name);
        }
    } else if (std::get_if<QuoteToken>(&first_token)) {
        tokenizer->Next();
        auto root = std::make_shared<Cell>();
        root->SetFirst(std::make_shared<Quote>(false));
        root->SetSecond(Read(tokenizer));
        return root;
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
/*    if (Is<Quote>(first) && As<Quote>(first)->IsSymbol()) {
        return ReadList(tokenizer);
    }*/
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
