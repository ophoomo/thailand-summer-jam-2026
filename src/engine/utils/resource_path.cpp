
#include "utils/resource_path.h"
#include "SDL3/SDL_filesystem.h"
#include "utils/logger.h"

std::string ResourcePath::s_basePath;

// ============================================================
// Public Methods
// ============================================================

void ResourcePath::init()
{
    const char *sdlBase = SDL_GetBasePath();
    if (sdlBase) {
        ResourcePath::s_basePath = sdlBase;
    } else {
        ResourcePath::s_basePath = "./";
        LOG_CORE_WARN("[ResourcePath] SDL_GetBasePath() failed, using './'");
    }
    LOG_CORE_INFO("[ResourcePath] base = {}", s_basePath);
}

std::string ResourcePath::resolve(const char *rel)
{
    if (!rel || rel[0] == '\0')
        return ResourcePath::s_basePath;
    // Absolute path — return as-is.
    if (rel[0] == '/')
        return rel;
    return ResourcePath::s_basePath + rel;
}

std::string ResourcePath::resolve(const std::string &rel)
{
    return resolve(rel.c_str());
}
