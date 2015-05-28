#include "../fixtures/util.hpp"
#include "mock_file_source.hpp"

#include <mbgl/storage/request.hpp>
#include <mbgl/util/io.hpp>
#include <mbgl/util/thread.hpp>

namespace {

const uint64_t timeout = 1000;

}

namespace mbgl {

class MockFileSource::Impl {
public:
    Impl(uv_loop_t* loop, Type type, const std::string& match)
        : type_(type), match_(match), timer_(loop) {
        timer_.start(timeout, timeout, [this] { dispatchPendingRequests(); });
        timer_.unref();
    }

    ~Impl() {
        timer_.stop();
    }

    void handleRequest(Request* req);

private:
    void replyWithSuccess(Request* req) const;
    void replyWithSuccessWithDelay(Request* req);
    void replyWithFailure(Request* req) const;
    void replyWithFailureWithDelay(Request* req);
    void replyWithCorruptedData(Request* req) const;

    void dispatchPendingRequests();

    Type type_;
    std::string match_;
    std::vector<Request*> pendingRequests_;
    uv::timer timer_;
};

void MockFileSource::Impl::dispatchPendingRequests() {
    for (auto req : pendingRequests_) {
        replyWithSuccess(req);
    }

    pendingRequests_.clear();
}


void MockFileSource::Impl::replyWithFailure(Request* req) const {
    if (req->resource.url.find(match_) == std::string::npos) {
        replyWithSuccess(req);
        return;
    }

    std::shared_ptr<Response> res = std::make_shared<Response>();
    res->status = Response::Status::Error;
    res->message = "Failed by the test case";

    req->notify(res);
}

void MockFileSource::Impl::replyWithFailureWithDelay(Request* req) {
    if (req->resource.url.find(match_) == std::string::npos) {
        pendingRequests_.push_back(req);
        return;
    }

    std::shared_ptr<Response> res = std::make_shared<Response>();
    res->status = Response::Status::Error;
    res->message = "Failed by the test case";

    req->notify(res);
}

void MockFileSource::Impl::replyWithCorruptedData(Request* req) const {
    if (req->resource.url.find(match_) == std::string::npos) {
        replyWithSuccess(req);
        return;
    }

    std::shared_ptr<Response> res = std::make_shared<Response>();
    res->status = Response::Status::Successful;
    res->data = util::read_file(req->resource.url);
    res->data.insert(0, "CORRUPTED");

    req->notify(res);
}

void MockFileSource::Impl::replyWithSuccess(Request* req) const {
    std::shared_ptr<Response> res = std::make_shared<Response>();
    res->status = Response::Status::Successful;
    res->data = util::read_file(req->resource.url);

    req->notify(res);
}

void MockFileSource::Impl::replyWithSuccessWithDelay(Request* req) {
    pendingRequests_.push_back(req);
}

void MockFileSource::Impl::handleRequest(Request* req) {
    switch (type_) {
    case Type::Success:
        replyWithSuccess(req);
        break;
    case Type::SuccessWithDelay:
        replyWithSuccessWithDelay(req);
        break;
    case Type::RequestFail:
        replyWithFailure(req);
        break;
    case Type::RequestFailWithDelay:
        replyWithFailureWithDelay(req);
        break;
    case Type::RequestWithCorruptedData:
        replyWithCorruptedData(req);
        break;
    default:
        EXPECT_TRUE(false) << "Should never be reached.";
    }
}

MockFileSource::MockFileSource(Type type, const std::string& match)
    : thread_(std::make_unique<util::Thread<Impl>>("FileSource", util::ThreadPriority::Low, type, match)) {
}

Request* MockFileSource::request(const Resource& resource, uv_loop_t* loop, Callback callback) {
    Request* req = new Request(resource, loop, std::move(callback));
    thread_->invoke(&Impl::handleRequest, req);

    return req;
}

void MockFileSource::cancel(Request*) {
}

}
