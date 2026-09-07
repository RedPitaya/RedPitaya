/**
 * Lookups that go through the embedded interpreter without an open device:
 * status and error texts come from the upstream status_messages /
 * error_messages dictionaries, so these tests fail if the bridge stops
 * importing the Python package or starts answering from a stale C++ copy.
 */

#include <gtest/gtest.h>

#include <cstring>
#include <set>
#include <string>

#include "rp_ptcc.h"

TEST(PtccLookup, VersionStringsAreNeverEmpty) {
    ASSERT_NE(rp_PtccGetVersion(), nullptr);
    ASSERT_NE(rp_PtccGetProtocolRevision(), nullptr);

    EXPECT_GT(std::string(rp_PtccGetVersion()).size(), 0u);
    EXPECT_GT(std::string(rp_PtccGetProtocolRevision()).size(), 0u);
}

TEST(PtccLookup, EveryErrorCodeHasItsOwnText) {
    const rp_ptcc_error codes[] = {RP_PTCC_OK,      RP_PTCC_ENOINIT, RP_PTCC_EOPEN,   RP_PTCC_EIO,
                                   RP_PTCC_ETIMEOUT, RP_PTCC_ERESP,  RP_PTCC_EIP,     RP_PTCC_ERANGE,
                                   RP_PTCC_ENOTSUP, RP_PTCC_ENODEV,  RP_PTCC_EPYTHON};

    std::set<std::string> texts;
    for (const rp_ptcc_error code : codes) {
        const char* text = rp_PtccGetErrorText(code);
        ASSERT_NE(text, nullptr);
        EXPECT_GT(std::strlen(text), 0u) << "empty text for code " << code;
        texts.insert(text);
    }

    EXPECT_EQ(texts.size(), sizeof(codes) / sizeof(codes[0]));
}

TEST(PtccLookup, UnknownErrorCodeStillReturnsAString) {
    EXPECT_STREQ(rp_PtccGetErrorText(static_cast<rp_ptcc_error>(99)), "unknown error");
}

// Upstream defines status codes 0..3 and error codes from 128 up. The texts
// themselves belong to the Python package, so only their presence and their
// classification are asserted here.
TEST(PtccLookup, NormalStatusCodesResolveToText) {
    for (uint8_t code = 0; code <= 3; ++code) {
        const std::string text = rp_PtccGetStatusText(code);
        EXPECT_NE(text, "unknown status code") << "no text for status " << static_cast<int>(code);
        EXPECT_FALSE(rp_PtccIsErrorStatus(code)) << "status " << static_cast<int>(code) << " is not an error";
    }
}

TEST(PtccLookup, ErrorStatusCodesAreClassifiedAsErrors) {
    // 128 is the first error code in the upstream table.
    EXPECT_TRUE(rp_PtccIsErrorStatus(128));
    EXPECT_NE(std::string(rp_PtccGetStatusText(128)), "unknown status code");
}

TEST(PtccLookup, UnusedStatusCodeReportsUnknown) {
    EXPECT_STREQ(rp_PtccGetStatusText(200), "unknown status code");
}

TEST(PtccLookup, StatusTextIsStableAcrossCalls) {
    const std::string first = rp_PtccGetStatusText(0);
    const std::string second = rp_PtccGetStatusText(0);
    EXPECT_EQ(first, second);
}
