#pragma once

namespace Utils
{
    inline vec2 OctNormalWrap(vec2 v)
    {
        vec2 wrap;
        wrap.x = (1.0f - glm::abs(v.y)) * (v.x >= 0.0f ? 1.0f : -1.0f);
        wrap.y = (1.0f - glm::abs(v.x)) * (v.y >= 0.0f ? 1.0f : -1.0f);
        return wrap;
    }

    inline vec2 OctNormalEncode(vec3 n)
    {
        n /= (glm::abs(n.x) + glm::abs(n.y) + glm::abs(n.z));

        vec2 wrapped = OctNormalWrap(n);

        vec2 result;
        result.x = n.z >= 0.0f ? n.x : wrapped.x;
        result.y = n.z >= 0.0f ? n.y : wrapped.y;

        result.x = result.x * 0.5f + 0.5f;
        result.y = result.y * 0.5f + 0.5f;

        return result;
    }

    inline vec3 OctNormalDecode(vec2 encN)
    {
        encN = encN * 2.0f - 1.0f;

        // https://twitter.com/Stubbesaurus/status/937994790553227264
        vec3 n = vec3(encN.x, encN.y, 1.0f - abs(encN.x) - abs(encN.y));
        float t = glm::clamp(-n.z, 0.0f, 1.0f);

        n.x += n.x >= 0.0f ? -t : t;
        n.y += n.y >= 0.0f ? -t : t;

        return normalize(n);
    }
}