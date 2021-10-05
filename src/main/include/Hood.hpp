#pragma once

#include "Constants.hpp"
#include "LimeLight.hpp"
#include <PID_CANSparkMax.hpp>
#include <frc/Joystick.h>

namespace Hood
{
//public function definitions
    void init();

    /// returns true if tolerance is met
    bool goToPosition(HOOD::POSITION position, double tolerance = HOOD::TOLERANCE);

    /// returns true if tolerance is met
    bool visionTrack(double tolerance = HOOD::TOLERANCE);

    /// used for tuning interpolation tables
    void manualPositionControl(double position);

    void   print_angle();
    double get_angle();
    double get_camera_Y();
};
