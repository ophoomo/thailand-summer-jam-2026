
#include "game/scenes/scene_credit.h"
#include "game/scenes/scene_game.h"
#include "game/scenes/scene_menu.h"
#include "game/scenes/scene_setting.h"
#include "game/scenes/scene_splash.h"
#include "game/scenes/scene_language.h"
#include <engine/core/application.h>
#include <engine/utils/logger.h>
#include <exception>
#include <memory>

int main()
{
    Logger::init();
    LOG_INFO("Initializing engine and application");

    try {
        // Application Initializing
        std::unique_ptr<Application> app = std::make_unique<Application>(1280, 720, "Heliora");

        // Scene Initializing
        std::shared_ptr<SceneSplash> scene_splash = std::make_shared<SceneSplash>(
            app->getDispatcher(), app->getRenderer(), app->getAssets(), app->getAudio());
        std::shared_ptr<SceneMenu> scene_menu = std::make_shared<SceneMenu>(
            app->getDispatcher(), app->getRenderer(), app->getAssets(), app->getAudio());
        std::shared_ptr<SceneSetting> scene_setting = std::make_shared<SceneSetting>(
            app->getDispatcher(), app->getRenderer(), app->getAssets(), app->getAudio());
        std::shared_ptr<SceneCredit> scene_credit = std::make_shared<SceneCredit>(
            app->getDispatcher(), app->getRenderer(), app->getAssets(), app->getAudio());
        std::shared_ptr<SceneGame> scene_game = std::make_shared<SceneGame>(
            app->getDispatcher(), app->getRenderer(), app->getAssets(), app->getAudio());
        std::shared_ptr<SceneLanguage> scene_lang = std::make_shared<SceneLanguage>(
            app->getDispatcher(), app->getRenderer(), app->getAssets(), app->getAudio());

        // Add Scene into Scene Manager
        app->getScene()->addScene(scene_splash, "splash");
        app->getScene()->addScene(scene_menu, "menu");
        app->getScene()->addScene(scene_setting, "setting");
        app->getScene()->addScene(scene_credit, "credit");
        app->getScene()->addScene(scene_game, "gameplay");
        app->getScene()->addScene(scene_lang, "lang");

        // Set First Scene
        app->getScene()->change("menu");

        app->run();
    } catch (const std::exception &e) {
        LOG_FATAL("Unhandled exception: {}", e.what());
        Logger::shutdown();
        return EXIT_FAILURE;
    } catch (...) {
        LOG_FATAL("Unknown exception caught");
        Logger::shutdown();
        return EXIT_FAILURE;
    }

    Logger::shutdown();
    return EXIT_SUCCESS;
}
