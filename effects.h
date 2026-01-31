#pragma once

#include <array>
#include <string>
#include <string_view>
#include <vector>

enum Effect {
    BOUNCE,
    COMET,
    FADE,
    MARQUEE,
    PARTICLES,
    STARS
};

inline const char* toString(const Effect effect) noexcept {
    switch (effect) {
        case BOUNCE:    return "bounce";
        case COMET:     return "comet";
        case FADE:      return "fade";
        case MARQUEE:   return "marquee";
        case PARTICLES: return "particles";
        case STARS:     return "stars";
        default:        return "unknown";
    }
}

inline bool tryParse(const std::string_view str, Effect &effect) noexcept {
    if (str == "bounce")    effect = BOUNCE;
    else if (str == "comet")     effect = COMET;
    else if (str == "fade")      effect = FADE;
    else if (str == "marquee")   effect = MARQUEE;
    else if (str == "particles") effect = PARTICLES;
    else if (str == "stars")     effect = STARS;
    else return false;

    return true;
}

inline const char *effectNames[] = {"bounce", "comet", "fade", "marquee", "particles", "stars"};

inline std::string buildEffectList() {
    std::string result = "<";

    for (const Effect effect : {
        BOUNCE, COMET, FADE, MARQUEE, PARTICLES, STARS
    }) {
        result += toString(effect);
        result += "|";
    }

    result.back() = '>';
    return result;
}
