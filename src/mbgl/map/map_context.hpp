#ifndef MBGL_MAP_MAP_CONTEXT
#define MBGL_MAP_MAP_CONTEXT

#include <mbgl/map/tile_id.hpp>
#include <mbgl/map/update.hpp>
#include <mbgl/map/environment.hpp>
#include <mbgl/map/transform_state.hpp>
#include <mbgl/style/style.hpp>
#include <mbgl/util/ptr.hpp>

#include <vector>

typedef struct uv_loop_s uv_loop_t;

namespace uv {
class async;
}

namespace mbgl {

class View;
class MapData;
class TexturePool;
class Painter;
class Sprite;
class Worker;
class StillImage;
struct LatLng;
struct LatLngBounds;

class MapContext : public Style::Observer {
public:
    MapContext(uv_loop_t*, View&, FileSource&, MapData&);
    ~MapContext();

    void pause();
    void render();

    void resize(uint16_t width, uint16_t height, float ratio);

    using StillImageCallback = std::function<void(std::exception_ptr, std::unique_ptr<const StillImage>)>;
    void renderStill(StillImageCallback callback);

    void triggerUpdate(Update = Update::Nothing);

    void setStyleURL(const std::string&);
    void setStyleJSON(const std::string& json, const std::string& base);
    std::string getStyleURL() const { return styleURL; }
    std::string getStyleJSON() const { return styleJSON; }

    double getTopOffsetPixelsForAnnotationSymbol(const std::string& symbol);
    void updateAnnotationTiles(const std::vector<TileID>&);

    void setSourceTileCacheSize(size_t size);
    void onLowMemory();

    // ResourceLoader::Observer implementation.
    void onTileDataChanged() override;
    void onResourceLoadingFailed(std::exception_ptr error) override;

private:
    void updateTiles();

    // Update the state indicated by the accumulated Update flags, then render.
    void update();

    // Loads the actual JSON object an creates a new Style object.
    void loadStyleJSON(const std::string& json, const std::string& base);

    View& view;
    MapData& data;

    Environment env;
    EnvironmentScope envScope;

    UpdateType updated { static_cast<UpdateType>(Update::Nothing) };
    std::unique_ptr<uv::async> asyncUpdate;

    std::unique_ptr<TexturePool> texturePool;
    std::unique_ptr<Painter> painter;
    std::unique_ptr<Style> style;

    std::string styleURL;
    std::string styleJSON;

    StillImageCallback callback;
    size_t sourceCacheSize;
    TransformState transformState;
};

}

#endif
