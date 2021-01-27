#pragma once

#include <coroutine>
#include <iostream>
#include <stdexcept>
#include <thread>
#include <variant>

#include "gameCommand.pb.h"

class GameCommand;
struct CommandAwaiter;

struct [[nodiscard]] Resumable {
    struct promise_type {
        using coro_handle = std::coroutine_handle<promise_type>;
        // if it's a coro_handle then it's next coroutine in a call stack basically
        std::variant<std::monostate, coro_handle, GameCommand*> awaitable;

        auto get_return_object() {
            return coro_handle::from_promise(*this);
        }
        auto initial_suspend() { return std::suspend_always(); }
        auto final_suspend() noexcept { return std::suspend_always(); }
        void return_void() {}
        void unhandled_exception() {
            std::terminate();
        }
    };

    using coro_handle = std::coroutine_handle<promise_type>;
    ~Resumable() { if (mHandle) mHandle.destroy(); }
    Resumable(coro_handle handle) : mHandle(handle) {}
    Resumable(const Resumable&) = delete;
    Resumable(Resumable&& rhs) : mHandle(rhs.mHandle) { rhs.mHandle = nullptr; }
    bool resume() {
        if (not mHandle.done())
            mHandle.resume();
        return mHandle.done();
    }
    bool done() { return mHandle.done(); }

    // Awaitable interface
    bool await_ready() { return resume(); }
    void await_suspend(coro_handle waiter) { waiter.promise().awaitable = mHandle; }
    void await_resume() {}

    // returns whether top level coroutine is finished so we can destroy it
    bool passCommand(GameCommand cmd) {
        auto h = mHandle;
        while(std::holds_alternative<coro_handle>(h.promise().awaitable))
            h = std::get<coro_handle>(h.promise().awaitable);
        assert(std::holds_alternative<GameCommand*>(h.promise().awaitable));
        *std::get<GameCommand*>(h.promise().awaitable) = cmd;
        h.promise().awaitable = std::monostate{};
        while (!mHandle.done()) {
            h.resume();
            if (h.done()) {
                h = mHandle;
                while(std::holds_alternative<coro_handle>(h.promise().awaitable) &&
                      !std::get<coro_handle>(h.promise().awaitable).done())
                    h = std::get<coro_handle>(h.promise().awaitable);
                h.promise().awaitable = std::monostate{};
            }
            else return false;
        }
        return true;
    }

private:
    coro_handle mHandle;
};

struct CommandAwaiter {
    GameCommand mCmd;

    bool await_ready() { return false; }
    void await_suspend(std::coroutine_handle<Resumable::promise_type> waiter) { waiter.promise().awaitable = &mCmd; }
    GameCommand await_resume() { return mCmd; }
};

inline CommandAwaiter waitForCommand() {
    return CommandAwaiter{};
}
