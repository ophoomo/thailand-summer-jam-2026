#ifndef C5FD351D_0746_40A4_9531_3385125A2153
#define C5FD351D_0746_40A4_9531_3385125A2153

#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_float4x4.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/ext/vector_float2.hpp"

struct Camera2D
{
    glm::vec2 position = {0.0f, 0.0f}; // world-space origin of view
    float rotation = 0.0f;             // radians, CCW positive
    float zoom = 1.0f;                 // >1 = zoom in

    // Cached — read by Renderer2D::EndFrame
    glm::mat4 view_proj{1.0f};

    void UpdateMatrices(float screen_w, float screen_h)
    {
        // Ortho: top-left = (0,0), bottom-right = (w,h), Y down.
        glm::mat4 proj = glm::ortho(0.f, screen_w, screen_h, 0.f, -1.f, 1.f);

        // View = inverse of camera transform.
        // Translation → rotate → scale (applied in reverse order to world).
        glm::mat4 view = glm::mat4(1.f);
        view = glm::translate(view, {screen_w * 0.5f, screen_h * 0.5f, 0.f});
        view = glm::rotate(view, rotation, {0.f, 0.f, 1.f});
        view = glm::scale(view, {zoom, zoom, 1.f});
        view = glm::translate(view, {-position.x, -position.y, 0.f});

        view_proj = proj * view;
    }
};

#endif /* C5FD351D_0746_40A4_9531_3385125A2153 */
