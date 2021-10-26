#pragma once

#include "Constants.hpp"

namespace ShooterWheel
{
    bool shooting = false;

    void init();

    void   bangbang();
    void   stop();
    double get_speed();

    double get_temp();

    void setShooting(bool boolean);
} // namespace ShooterWheel
