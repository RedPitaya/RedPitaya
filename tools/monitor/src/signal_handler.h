#ifndef SIGNAL_HANDLER_H
#define SIGNAL_HANDLER_H

#include <atomic>
#include <chrono>
#include <csignal>
#include <cstdio>
#include <functional>
#include <thread>
#include <vector>

// ============================================================================
// Singleton signal handler for graceful shutdown on Ctrl+C (SIGINT).
// Allows registering callbacks that will be called when signal is received.
// ============================================================================
class SignalHandler {
   public:
    // Get the singleton instance
    static SignalHandler& getInstance() {
        static SignalHandler instance;
        return instance;
    }

    // Register a callback to be called on signal
    void registerCallback(std::function<void()> callback) { callbacks_.push_back(std::move(callback)); }

    // Check if shutdown was requested
    bool isShutdownRequested() const { return shutdownRequested_; }

    // Request shutdown programmatically
    void requestShutdown() {
        shutdownRequested_ = true;
        executeCallbacks();
    }

    // Wait until shutdown is requested (blocks)
    void waitForShutdown() {
        while (!shutdownRequested_) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }

   private:
    SignalHandler() : shutdownRequested_(false) {
        // Register signal handlers
        std::signal(SIGINT, signalHandler);
        std::signal(SIGTERM, signalHandler);
    }

    ~SignalHandler() = default;

    // Delete copy/move
    SignalHandler(const SignalHandler&) = delete;
    SignalHandler& operator=(const SignalHandler&) = delete;

    // Static signal handler function
    static void signalHandler(int signal) {
        if (signal == SIGINT || signal == SIGTERM) {
            printf("\nShutdown signal received (Ctrl+C). Cleaning up...\n");
            getInstance().requestShutdown();
        }
    }

    // Execute all registered callbacks
    void executeCallbacks() {
        for (auto& callback : callbacks_) {
            if (callback) {
                callback();
            }
        }
    }

    std::atomic<bool> shutdownRequested_;
    std::vector<std::function<void()>> callbacks_;
};

#endif  // SIGNAL_HANDLER_H