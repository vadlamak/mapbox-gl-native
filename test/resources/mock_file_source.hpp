#ifndef TEST_RESOURCES_MOCK_FILE_SOURCE
#define TEST_RESOURCES_MOCK_FILE_SOURCE

#include <mbgl/storage/file_source.hpp>
#include <mbgl/util/thread.hpp>

#include <string>
#include <memory>

namespace mbgl {

// The MockFileSource is a FileSource that can simulate different
// types of failures and it will work completely offline.
class MockFileSource : public FileSource {
public:
    // Success:
    //     Will reply to every request correctly with valid data.
    //
    // SuccessWithDelay:
    //     Will reply to every request correctly with valid data,
    //     but after a delay. This can be useful for testing request
    //     cancellation.
    //
    // RequestFail:
    //     Will reply with an error to requests that contains
    //     the "match" string on the URL.
    //
    // RequestFailWithDelay:
    //     Will reply with an error to requests that contains
    //     the "match" string on the URL. All the other requests are
    //     going to succeed but will be answered with an delay. This
    //     can be useful for testing request cancellation.
    //
    // RequestWithCorruptedData:
    //     Will answer every request successfully but will return
    //     corrupt data on the requests that contains the "match"
    //     string on the URL.
    enum Type {
        Success,
        SuccessWithDelay,
        RequestFail,
        RequestFailWithDelay,
        RequestWithCorruptedData
    };

    class Impl;

    MockFileSource(Type type, const std::string& match);
    ~MockFileSource() override = default;

    // FileSource implementation.
    Request* request(const Resource&, uv_loop_t*, Callback) override;
    void cancel(Request*) override;

private:
    const std::unique_ptr<util::Thread<Impl>> thread_;
};

}

#endif
