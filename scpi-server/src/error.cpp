#include "scpi/error.h"
#include <map>
#include <mutex>
#include <queue>
#include "error.h"
#include "scpi/ieee488.h"

std::map<scpi_t*, queue<rp_error_t>> g_errorList;
std::mutex g_errMutex;

auto rp_resetErrorList(scpi_t* context) -> void {
    std::lock_guard<std::mutex> lock(g_errMutex);
    g_errorList[context] = queue<rp_error_t>();
}

auto rp_addError(scpi_t* context, rp_error_t& err) -> void {
    std::lock_guard<std::mutex> lock(g_errMutex);
    g_errorList[context].push(err);
}

auto rp_popError(scpi_t* context) -> rp_error_t {
    std::lock_guard<std::mutex> lock(g_errMutex);
    if (g_errorList[context].empty())
        return rp_error_t();
    auto err = g_errorList[context].front();
    g_errorList[context].pop();
    return err;
}

auto rp_errorCount(scpi_t* context) -> size_t {
    std::lock_guard<std::mutex> lock(g_errMutex);
    if (g_errorList.count(context) > 0)
        return g_errorList[context].size();
    return 0;
}

auto rp_errorPush(scpi_t* context, rp_error_t& err) -> void {
    rp_addError(context, err);
    SCPI_RegSetBits(context, SCPI_REG_ESR, ESR_EER);
    SCPI_RegSetBits(context, SCPI_REG_STB, STB_QMA);
    if (context) {
        context->cmd_error = TRUE;
    }
}