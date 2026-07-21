#include "core/net/CancelToken.h"

namespace aicpp::net {

namespace {
std::atomic<bool> g_cancelRequested{false};
std::atomic<bool> g_streamInFlight{false};
}  // namespace

std::atomic<bool>& cancelRequested() { return g_cancelRequested; }
std::atomic<bool>& streamInFlight() { return g_streamInFlight; }

}  // namespace aicpp::net
