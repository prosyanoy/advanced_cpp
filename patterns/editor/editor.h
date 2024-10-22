#pragma once

#include <memory>
#include <list>
#include <stack>

class Command {
public:
    virtual int GetFlag() = 0;
    // declares an interface for executing an operation.
    virtual void Execute() = 0;
    virtual ~Command() = default;

protected:
    Command() = default;
};

template <typename Receiver>
class TypeCommand : public Command {
public:
    static constexpr int flag = 1;
    virtual int GetFlag() {
        return flag;
    }

    typedef void (Receiver::*Action)(char c, std::list<char>* buffer,
                                     typename std::list<char>::iterator* cursor_ptr);
    TypeCommand(char c, std::list<char>* buffer, typename std::list<char>::iterator* cursor_ptr,
                std::shared_ptr<Receiver> receiver, Action action)
        : character_(c),
          buffer_(buffer),
          cursor_ptr_(cursor_ptr),
          receiver_(receiver.get()),
          action_(action) {
    }
    TypeCommand(const TypeCommand&) = delete;
    const TypeCommand& operator=(const TypeCommand&) = delete;
    virtual void Execute() {
        (receiver_->*action_)(character_, buffer_, cursor_ptr_);
    }

    char GetChar() {
        return character_;
    }

private:
    char character_;
    std::list<char>* buffer_;
    typename std::list<char>::iterator* cursor_ptr_;
    Receiver* receiver_;
    Action action_;
};

template <typename Receiver>
class ShiftLeftCommand : public Command {
public:
    static constexpr int flag = 2;
    virtual int GetFlag() {
        return flag;
    }

    typedef void (Receiver::*Action)(std::list<char>* buffer,
                                     typename std::list<char>::iterator* cursor_ptr);
    ShiftLeftCommand(std::list<char>* buffer, typename std::list<char>::iterator* cursor_ptr,
                     std::shared_ptr<Receiver> receiver, Action action)
        : buffer_(buffer), cursor_ptr_(cursor_ptr), receiver_(receiver.get()), action_(action) {
    }
    ShiftLeftCommand(const ShiftLeftCommand&) = delete;
    const ShiftLeftCommand& operator=(const ShiftLeftCommand&) = delete;
    virtual void Execute() {
        (receiver_->*action_)(buffer_, cursor_ptr_);
    }

private:
    std::list<char>* buffer_;
    typename std::list<char>::iterator* cursor_ptr_;
    Receiver* receiver_;
    Action action_;
};

template <typename Receiver>
class ShiftRightCommand : public Command {
public:
    static constexpr int flag = 3;
    virtual int GetFlag() {
        return flag;
    }

    typedef void (Receiver::*Action)(std::list<char>* buffer,
                                     typename std::list<char>::iterator* cursor_ptr);
    ShiftRightCommand(std::list<char>* buffer, typename std::list<char>::iterator* cursor_ptr,
                      std::shared_ptr<Receiver> receiver, Action action)
        : buffer_(buffer), cursor_ptr_(cursor_ptr), receiver_(receiver.get()), action_(action) {
    }
    ShiftRightCommand(const ShiftRightCommand&) = delete;
    const ShiftRightCommand& operator=(const ShiftRightCommand&) = delete;
    virtual void Execute() {
        (receiver_->*action_)(buffer_, cursor_ptr_);
    }

private:
    std::list<char>* buffer_;
    typename std::list<char>::iterator* cursor_ptr_;
    Receiver* receiver_;
    Action action_;
};

template <typename Receiver>
class BackspaceCommand : public Command {
public:
    static constexpr int flag = 4;
    virtual int GetFlag() {
        return flag;
    }

    typedef void (Receiver::*Action)(std::list<char>* buffer,
                                     typename std::list<char>::iterator* cursor_ptr);
    BackspaceCommand(char c, std::list<char>* buffer,
                     typename std::list<char>::iterator* cursor_ptr,
                     std::shared_ptr<Receiver> receiver, Action action)
        : character_(c),
          buffer_(buffer),
          cursor_ptr_(cursor_ptr),
          receiver_(receiver.get()),
          action_(action) {
    }
    BackspaceCommand(const BackspaceCommand&) = delete;
    const BackspaceCommand& operator=(const BackspaceCommand&) = delete;
    virtual void Execute() {
        (receiver_->*action_)(buffer_, cursor_ptr_);
    }

    char GetChar() {
        return character_;
    }

private:
    char character_;
    std::list<char>* buffer_;
    typename std::list<char>::iterator* cursor_ptr_;
    Receiver* receiver_;
    Action action_;
};

template <typename Receiver>
class UndoCommand : public Command {
public:
    static constexpr int flag = 5;
    virtual int GetFlag() {
        return flag;
    }

    typedef void (Receiver::*Action)(std::shared_ptr<Command> cmd, std::list<char>* buffer,
                                     typename std::list<char>::iterator* cursor_ptr);
    UndoCommand(std::shared_ptr<Command> cmd, std::list<char>* buffer,
                typename std::list<char>::iterator* cursor_ptr, std::shared_ptr<Receiver> receiver,
                Action action)
        : cmd_(cmd),
          buffer_(buffer),
          cursor_ptr_(cursor_ptr),
          receiver_(receiver.get()),
          action_(action) {
    }
    UndoCommand(const UndoCommand&) = delete;
    const UndoCommand& operator=(const UndoCommand&) = delete;
    virtual void Execute() {
        (receiver_->*action_)(cmd_, buffer_, cursor_ptr_);
    }

private:
    std::shared_ptr<Command> cmd_;
    std::list<char>* buffer_;
    typename std::list<char>::iterator* cursor_ptr_;
    Receiver* receiver_;
    Action action_;
};

template <typename Receiver>
class RedoCommand : public Command {
public:
    static constexpr int flag = 6;
    virtual int GetFlag() {
        return flag;
    }

    typedef void (Receiver::*Action)(std::shared_ptr<Command> cmd);
    RedoCommand(std::shared_ptr<Command> cmd, typename std::list<char>::iterator* cursor_ptr,
                std::shared_ptr<Receiver> receiver, Action action)
        : cmd_(cmd), cursor_(*cursor_ptr), receiver_(receiver.get()), action_(action) {
    }
    RedoCommand(const RedoCommand&) = delete;
    const RedoCommand& operator=(const RedoCommand&) = delete;
    virtual void Execute() {
        (receiver_->*action_)(cmd_);
    }

private:
    std::shared_ptr<Command> cmd_;
    typename std::list<char>::iterator cursor_;
    Receiver* receiver_;
    Action action_;
};

struct Receiver {
    // 1
    void Type(char c, std::list<char>* buffer, typename std::list<char>::iterator* cursor) {
        buffer->insert(*cursor, c);
    }

    // 2
    void ShiftLeft(std::list<char>* buffer, typename std::list<char>::iterator* cursor) {
        if (*cursor != buffer->begin()) {
            *cursor = std::next(*cursor, -1);
        }
    }

    // 3
    void ShiftRight(std::list<char>* buffer, typename std::list<char>::iterator* cursor) {
        if (*cursor != std::next(buffer->end(), -1)) {
            *cursor = std::next(*cursor);
        }
    }

    // 4
    void Backspace(std::list<char>* buffer, typename std::list<char>::iterator* cursor) {
        if (*cursor != buffer->begin()) {
            (*buffer).erase(std::next(*cursor, -1));
            // *cursor = std::next(*cursor, -1);
        }
    }

    // 5
    void Undo(std::shared_ptr<Command> cmd, std::list<char>* buffer,
              typename std::list<char>::iterator* cursor) {
        int flag = cmd->GetFlag();
        if (flag == 1) {
            Backspace(buffer, cursor);
        } else if (flag == 2) {
            ShiftRight(buffer, cursor);
        } else if (flag == 3) {
            ShiftLeft(buffer, cursor);
        } else if (flag == 4) {
            if (auto backspace = dynamic_cast<BackspaceCommand<Receiver>*>(cmd.get());
                backspace != nullptr) {
                char c = backspace->GetChar();
                Type(c, buffer, cursor);
            }
        }
    }

    // 6
    void Redo(std::shared_ptr<Command> cmd) {
        cmd->Execute();
    }
};

struct MyString {
    std::string s;
};
class Editor {
public:
    const std::string& GetText() const {
        uptr->s.clear();
        auto it = buffer_.begin();
        while (*it != '\0') {
            uptr->s += *it;
            ++it;
        }
        return uptr->s;
    }

    void ExecuteCommand(std::shared_ptr<Command> cmd) {
        cmd->Execute();
        undoStack.push(cmd);
        while (!redoStack.empty()) {
            redoStack.pop();
        }
    }

    void Type(char c) {
        std::shared_ptr<Command> command = std::make_shared<TypeCommand<Receiver>>(
            c, &buffer_, &cursor_, receiver_, &Receiver::Type);
        ExecuteCommand(command);
    }

    void ShiftLeft() {
        if (cursor_ != buffer_.begin()) {
            std::shared_ptr<Command> command = std::make_shared<ShiftLeftCommand<Receiver>>(
                &buffer_, &cursor_, receiver_, &Receiver::ShiftLeft);
            ExecuteCommand(command);
        }
    }

    void ShiftRight() {
        if (cursor_ != std::next(buffer_.end(), -1)) {
            std::shared_ptr<Command> command = std::make_shared<ShiftRightCommand<Receiver>>(
                &buffer_, &cursor_, receiver_, &Receiver::ShiftRight);
            ExecuteCommand(command);
        }
    }

    void Backspace() {
        if (cursor_ != buffer_.begin()) {
            char c = *std::next(cursor_, -1);
            std::shared_ptr<Command> command = std::make_shared<BackspaceCommand<Receiver>>(
                c, &buffer_, &cursor_, receiver_, &Receiver::Backspace);
            ExecuteCommand(command);
        }
    }

    void Undo() {
        if (!undoStack.empty()) {
            auto prev_cmd = undoStack.top();
            redoStack.push(prev_cmd);
            undoStack.pop();
            std::shared_ptr<Command> command = std::make_shared<UndoCommand<Receiver>>(
                prev_cmd, &buffer_, &cursor_, receiver_,
                &Receiver::Undo);  // In template: no matching function for call to '__construct_at'
            command->Execute();
        }
    }

    void Redo() {
        if (!redoStack.empty()) {
            auto prev_cmd = redoStack.top();
            undoStack.push(prev_cmd);
            redoStack.pop();
            std::shared_ptr<Command> command = std::make_shared<RedoCommand<Receiver>>(
                prev_cmd, &cursor_, receiver_,
                &Receiver::Redo);  // In template: no matching function for call to '__construct_at'
            command->Execute();
        }
    }

private:
    std::unique_ptr<MyString> uptr = std::make_unique<MyString>();

    std::shared_ptr<Receiver> receiver_ = std::make_shared<Receiver>();
    std::list<char> buffer_ = {'\0'};
    typename std::list<char>::iterator cursor_ = buffer_.begin();

    std::stack<std::shared_ptr<Command>> undoStack;
    std::stack<std::shared_ptr<Command>> redoStack;

    std::string s_;
};
