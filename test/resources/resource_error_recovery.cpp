#include "../fixtures/fixture_log_observer.hpp"
#include "../fixtures/util.hpp"
#include "mock_file_source.hpp"

#include <mbgl/map/map.hpp>
#include <mbgl/map/still_image.hpp>
#include <mbgl/platform/default/headless_display.hpp>
#include <mbgl/platform/default/headless_view.hpp>
#include <mbgl/util/io.hpp>
#include <mbgl/util/run_loop.hpp>
#include <mbgl/util/uv_detail.hpp>

using namespace mbgl;

class ResourceErrorRecoveryTest : public ::testing::TestWithParam<std::string> {
};

TEST_P(ResourceErrorRecoveryTest, DeleteMapObjectAfterFailure) {
    util::RunLoop loop(uv_default_loop());

    auto display = std::make_shared<mbgl::HeadlessDisplay>();
    HeadlessView view(display);
    MockFileSource fileSource(MockFileSource::RequestFailWithDelay, GetParam());

    std::unique_ptr<Map> map = std::make_unique<Map>(view, fileSource, MapMode::Still);

    const std::string style = util::read_file("test/fixtures/resources/style.json");
    map->resize(1000, 1000, 1.0);
    map->setStyleJSON(style, ".");

    uv::async endTest(loop.get(), [&map, &loop] {
        map.reset();
        loop.stop();
    });
    endTest.unref();

    map->renderStill([&endTest](std::exception_ptr ex, std::unique_ptr<const StillImage>) {
        EXPECT_TRUE(ex != nullptr);
        endTest.send();
    });

    uv_run(loop.get(), UV_RUN_DEFAULT);
}

INSTANTIATE_TEST_CASE_P(ResourceLoader, ResourceErrorRecoveryTest,
    ::testing::Values("source.json", "sprite.json", "sprite.png", "vector.pbf", "glyphs.pbf"));
