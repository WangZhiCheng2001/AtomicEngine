#pragma once
/* simply copied and modified from nvpro_cores/nvh/cameraManipulator */

#include <array>
#include <chrono>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include "boundingBox.hpp"

namespace std
{
    inline float sign(float s)
    {
        return (s < 0.f) ? -1.f : 1.f;
    }
};

class Camera
{
public:
    enum class eCameraProjectionType
    {
        CAMERA_PROJECTION_TYPE_ORTHO,
        CAMERA_PROJECTION_TYPE_PERSPECTIVE
    };

    enum class eCameraActionMode
    {
        CAMERA_ACTION_MODE_FLY,
        CAMERA_ACTION_MODE_FOCUS_ON_OBJECT
    };

    enum class eCameraActions
    {
        CAMERA_ACTION_NONE,
        CAMERA_ACTION_ORBIT,
        CAMERA_ACTION_LOOK_AROUND,
        CAMERA_ACTION_DOLLY,
        CAMERA_ACTION_PAN
    };

    auto getLookat() const { return std::array{m_eye, m_target, m_up}; }
    const glm::vec2 &getClipPlanes() const { return m_clipPlanes; }
    const eCameraProjectionType &getProjectionType() const { return m_type; }
    const float &getFov() const { return m_fov; }
    int getWidth() const { return m_width; }
    int getHeight() const { return m_height; }
    const glm::vec2 &getMousePosition() const { return m_mousePosition; }
    glm::mat4 getViewMatrix() const { return m_viewMatrix; }
    glm::mat4 getProjectionMatrix() const
    {
        auto mat = glm::perspective(m_fov, (float)m_width / m_height, m_clipPlanes.x, m_clipPlanes.y);
        mat[1][1] *= -1;
        return mat;
    }
    const eCameraActionMode &getActionMode() const { return m_mode; }

    void setClipPlanes(const glm::vec2 &clip) { m_clipPlanes = clip; }
    void setProjectionType(const eCameraProjectionType &type) { m_type = type; }
    void setFov(const float &fov) { m_fov = std::min(std::max(fov, 0.01f), 179.0f); }
    void setWindowSize(int w, int h) { m_width = w, m_height = h; }
    void setMousePosition(const glm::vec2 &pos) { m_mousePosition = pos; }
    void setActionMode(const eCameraActionMode &mode) { m_mode = mode; }

protected:
    void update() { m_viewMatrix = glm::lookAt(m_eye, m_target, m_up); }

    // Do panning: movement parallels to the screen
    void pan(float dx, float dy)
    {
        if (m_mode == eCameraActionMode::CAMERA_ACTION_MODE_FLY)
        {
            dx *= -1, dy *= -1;
        }

        glm::vec3 z(m_eye - m_target);
        float length = static_cast<float>(glm::length(z)) / 0.785f; // 45 degrees
        z = glm::normalize(z);
        glm::vec3 x = glm::cross(m_up, z);
        x = glm::normalize(x);
        glm::vec3 y = glm::cross(z, x);
        y = glm::normalize(y);
        x *= -dx * length;
        y *= dy * length;

        m_eye += x + y;
        m_target += x + y;
    }
    // Do orbiting: rotation around the center of interest. If invert, the interest orbit around the camera position
    void orbit(float dx, float dy, bool invert = false)
    {
        if (dx == 0 && dy == 0)
            return;

        // Full width will do a full turn
        dx *= glm::two_pi<float>();
        dy *= glm::two_pi<float>();

        // Get the camera
        glm::vec3 origin(invert ? m_eye : m_target);
        glm::vec3 position(invert ? m_target : m_eye);

        // Get the length of sight
        glm::vec3 centerToEye(position - origin);
        float radius = glm::length(centerToEye);
        centerToEye = glm::normalize(centerToEye);

        glm::mat4 rot_x, rot_y;

        // Find the rotation around the UP axis (Y)
        glm::vec3 axe_z(glm::normalize(centerToEye));
        rot_y = glm::rotate(glm::mat4(1.0f), -dx, m_up);

        // Apply the (Y) rotation to the eye-center vector
        glm::vec4 vect_tmp = rot_y * glm::vec4(centerToEye.x, centerToEye.y, centerToEye.z, 0);
        centerToEye = glm::vec3(vect_tmp.x, vect_tmp.y, vect_tmp.z);

        // Find the rotation around the X vector: cross between eye-center and up (X)
        glm::vec3 axe_x = glm::cross(m_up, axe_z);
        axe_x = glm::normalize(axe_x);
        rot_x = glm::rotate(glm::mat4(1.0f), -dy, axe_x);

        // Apply the (X) rotation to the eye-center vector
        vect_tmp = rot_x * glm::vec4(centerToEye.x, centerToEye.y, centerToEye.z, 0);
        glm::vec3 vect_rot(vect_tmp.x, vect_tmp.y, vect_tmp.z);
        if (std::sign(vect_rot.x) == std::sign(centerToEye.x))
            centerToEye = vect_rot;

        // Make the vector as long as it was originally
        centerToEye *= radius;

        // Finding the new position
        glm::vec3 newPosition = centerToEye + origin;

        if (!invert)
            m_eye = newPosition; // Normal: change the position of the camera
        else
            m_target = newPosition; // Inverted: change the interest point
    }
    // Do dolly: movement toward the interest.
    void dolly(float dx, float dy)
    {
        glm::vec3 z = m_target - m_eye;
        float length = static_cast<float>(glm::length(z));

        // We are at the point of interest, and don't know any direction, so do nothing!
        if (length < 0.000001f)
            return;

        // Use the larger movement.
        float dd;
        if (m_mode == eCameraActionMode::CAMERA_ACTION_MODE_FLY)
            dd = -dy;
        else
            dd = fabs(dx) > fabs(dy) ? dx : -dy;
        float factor = m_speed * dd;

        // Adjust speed based on distance.
        if (m_mode == eCameraActionMode::CAMERA_ACTION_MODE_FOCUS_ON_OBJECT)
        {
            // Don't move over the point of interest.
            if (factor >= 1.0f)
                return;

            z *= factor;
        }
        else
            // Normalize the Z vector and make it faster
            z *= factor / length * 10.0f;

        m_eye += z;

        // In fly mode, the interest moves with us.
        if (m_mode == eCameraActionMode::CAMERA_ACTION_MODE_FLY)
            m_target += z;
    }

    double getSystemTime()
    {
        auto now(std::chrono::system_clock::now());
        auto duration = now.time_since_epoch();
        return std::chrono::duration_cast<std::chrono::microseconds>(duration).count() / 1000.0;
    }

    glm::vec3 computeBezier(float t, glm::vec3 &p0, glm::vec3 &p1, glm::vec3 &p2)
    {
        float u = 1.f - t;
        float tt = t * t;
        float uu = u * u;

        glm::vec3 p = uu * p0; // first term
        p += 2 * u * t * p1;   // second term
        p += tt * p2;          // third term

        return p;
    }
    void findBezierPoints()
    {
        glm::vec3 p0 = m_eye;
        glm::vec3 p2 = m_eyeTo;
        glm::vec3 p1, pc;

        // point of interest
        glm::vec3 pi = (m_targetTo + m_target) * 0.5f;

        glm::vec3 p02 = (p0 + p2) * 0.5f;                          // mid p0-p2
        float radius = (length(p0 - pi) + length(p2 - pi)) * 0.5f; // Radius for p1
        glm::vec3 p02pi(p02 - pi);                                 // Vector from interest to mid point
        p02pi = glm::normalize(p02pi);
        p02pi *= radius;
        pc = pi + p02pi;                       // Calculated point to go through
        p1 = 2.f * pc - p0 * 0.5f - p2 * 0.5f; // Computing p1 for t=0.5
        p1.y = p02.y;                          // Clamping the P1 to be in the same height as p0-p2

        m_bezier[0] = p0;
        m_bezier[1] = p1;
        m_bezier[2] = p2;
    }

public:
    void updateAnim()
    {
        auto elapse = static_cast<float>(getSystemTime() - m_start_time) / 1000.f;

        // Key animation
        if (m_key_vec != glm::vec3(0, 0, 0))
        {
            m_eye += m_key_vec * elapse * 1000.f;
            m_target += m_key_vec * elapse * 1000.f;
            update();
            m_start_time = getSystemTime();
            return;
        }

        // Camera moving to new position
        if (m_anim_done)
            return;

        float t = std::min(elapse / float(m_duration), 1.0f);
        // Evaluate polynomial (smoother step from Perlin)
        t = t * t * t * (t * (t * 6.0f - 15.0f) + 10.0f);
        if (t >= 1.0f)
        {
            m_eye = m_eyeTo;
            m_target = m_targetTo;
            m_up = m_upTo;
            m_fov = m_fovTo;
            m_anim_done = true;
            return;
        }

        // Interpolate camera position and interest
        // The distance of the camera between the interest is preserved to
        // create a nicer interpolation
        glm::vec3 vpos, vint, vup;
        m_target = (1 - t) * m_targetFrom + t * m_targetTo;
        m_up = (1 - t) * m_upFrom + t * m_upTo;
        m_eye = computeBezier(t, m_bezier[0], m_bezier[1], m_bezier[2]);
        m_fov = (1 - t) * m_fovFrom + t * m_fovTo;

        update();
    }

    void setLookat(const glm::vec3 &eye, const glm::vec3 &center, const glm::vec3 &up, bool instantSet = true)
    {
        m_anim_done = true;

        if (instantSet)
        {
            m_eye = eye;
            m_target = center;
            m_up = up;
            update();
        }
        else if (m_eye != eye || m_target != center || m_up != up)
        {
            m_eyeFrom = m_eye;
            m_targetFrom = m_target;
            m_upFrom = m_up;
            m_fovFrom = m_fov;
            m_eyeTo = eye;
            m_targetTo = center;
            m_upTo = up;
            m_fovTo = m_fov;
            m_anim_done = false;
            m_start_time = getSystemTime();
            findBezierPoints();
        }
    }

    void fit(const BoundingBox &box, const glm::mat4 &modelMatrix = glm::mat4(1), bool instantFit = true, bool tight = false, float aspect = 1.0f)
    {
        glm::vec4 temp = modelMatrix * glm::vec4{box.minPoint, 1.f};
        const glm::vec3 minPoint = {temp.x, temp.y, temp.z};
        temp = modelMatrix * glm::vec4{box.maxPoint, 1.f};
        const glm::vec3 maxPoint = {temp.x, temp.y, temp.z};
        const glm::vec3 boxHalfSize = (maxPoint - minPoint) * .5f;
        const glm::vec3 boxCenter = minPoint + boxHalfSize;

        float offset = 0;
        float yfov = m_fov;
        float xfov = m_fov * aspect;

        if (!tight)
        {
            // Using the bounding sphere
            float radius = glm::length(boxHalfSize);
            if (aspect > 1.f)
                offset = radius / sin(glm::radians(yfov * 0.5f));
            else
                offset = radius / sin(glm::radians(xfov * 0.5f));
        }
        else
        {
            glm::mat4 mView = glm::lookAt(m_eye, boxCenter, m_up);
            mView[3][0] = mView[3][1] = mView[3][2] = 0; // Keep only rotation

            for (int i = 0; i < 8; i++)
            {
                glm::vec3 vct(i & 1 ? boxHalfSize.x : -boxHalfSize.x, i & 2 ? boxHalfSize.y : -boxHalfSize.y,
                              i & 4 ? boxHalfSize.z : -boxHalfSize.z);
                vct = glm::vec3(mView * glm::vec4(vct, 1.f));

                if (vct.z < 0) // Take only points in front of the center
                {
                    // Keep the largest offset to see that vertex
                    offset = std::max(float(fabs(vct.y) / tan(glm::radians(yfov * 0.5f)) + fabs(vct.z)), offset);
                    offset = std::max(float(fabs(vct.x) / tan(glm::radians(xfov * 0.5f)) + fabs(vct.z)), offset);
                }
            }
        }

        // Re-position the camera
        auto viewDir = glm::normalize(m_eye - m_target);
        auto veye = boxCenter + viewDir * offset;
        setLookat(veye, boxCenter, m_up, instantFit);
    }

    void motion(int x, int y, eCameraActions action = eCameraActions::CAMERA_ACTION_NONE)
    {
        float dx = float(x - m_mousePosition[0]) / float(m_width);
        float dy = float(y - m_mousePosition[1]) / float(m_height);

        switch (action)
        {
        case eCameraActions::CAMERA_ACTION_ORBIT:
            orbit(dx, dy, false);
            break;
        case eCameraActions::CAMERA_ACTION_LOOK_AROUND:
            orbit(dx, -dy, true);
            break;
        case eCameraActions::CAMERA_ACTION_DOLLY:
            dolly(dx, dy);
            break;
        case eCameraActions::CAMERA_ACTION_PAN:
            pan(dx, dy);
            break;
        }

        // Resetting animation
        m_anim_done = true;

        update();

        m_mousePosition[0] = static_cast<float>(x);
        m_mousePosition[1] = static_cast<float>(y);
    }
    void keyMotion(float dx, float dy, eCameraActions action)
    {
        if (action == eCameraActions::CAMERA_ACTION_NONE)
        {
            m_key_vec = {0, 0, 0};
            return;
        }

        auto d = glm::normalize(m_target - m_eye);
        dx *= m_speed * 2.f;
        dy *= m_speed * 2.f;

        glm::vec3 key_vec;
        if (action == eCameraActions::CAMERA_ACTION_DOLLY)
            key_vec = d * dx;
        else if (action == eCameraActions::CAMERA_ACTION_PAN)
        {
            auto r = glm::cross(d, m_up);
            key_vec = r * dx + m_up * dy;
        }

        m_key_vec += key_vec;

        // Resetting animation
        m_start_time = getSystemTime();
    }

    eCameraActions mouseMove(int x, int y, bool lmb, bool mmb, bool rmb)
    {
        eCameraActions curAction = eCameraActions::CAMERA_ACTION_NONE;
        if (lmb)
        {
            curAction = m_mode == eCameraActionMode::CAMERA_ACTION_MODE_FOCUS_ON_OBJECT ? eCameraActions::CAMERA_ACTION_ORBIT : eCameraActions::CAMERA_ACTION_LOOK_AROUND;
        }
        else if (mmb)
            curAction = eCameraActions::CAMERA_ACTION_PAN;
        else if (rmb)
            curAction = eCameraActions::CAMERA_ACTION_DOLLY;

        if (curAction != eCameraActions::CAMERA_ACTION_NONE)
            motion(x, y, curAction);

        return curAction;
    }
    void wheel(int value)
    {
        float fval(static_cast<float>(value));
        float dx = (fval * fabsf(fval)) / static_cast<float>(m_width);

        dolly(dx * m_speed, dx * m_speed);
        update();
    }

protected:
    // basic
    glm::vec3 m_eye{10, 10, 10};
    glm::vec3 m_target{0, 0, 0};
    glm::vec3 m_up{0, 1, 0};
    glm::vec2 m_clipPlanes{0.001f, 100000000.f};
    eCameraProjectionType m_type{eCameraProjectionType::CAMERA_PROJECTION_TYPE_PERSPECTIVE};
    float m_fov{60.0f};

    glm::mat4 m_viewMatrix = glm::mat4(1);

    // Animation
    glm::vec3 m_eyeFrom{};
    glm::vec3 m_targetFrom{};
    glm::vec3 m_upFrom{};
    float m_fovFrom{};
    glm::vec3 m_eyeTo{};
    glm::vec3 m_targetTo{};
    glm::vec3 m_upTo{};
    float m_fovTo{};
    std::array<glm::vec3, 3> m_bezier;
    double m_start_time = 0;
    double m_duration = 0.5;
    bool m_anim_done{true};
    glm::vec3 m_key_vec{0, 0, 0};

    // Screen
    int m_width = 1;
    int m_height = 1;

    // Other
    float m_speed = 3.f;
    glm::vec2 m_mousePosition{0.f, 0.f};

    bool m_button = false; // Button pressed
    bool m_moving = false; // Mouse is moving
    float m_tbsize = 0.8f; // Trackball size;

    eCameraActionMode m_mode{eCameraActionMode::CAMERA_ACTION_MODE_FLY};
};