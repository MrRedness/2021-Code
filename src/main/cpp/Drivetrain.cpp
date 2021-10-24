#include "drivetrain.hpp"
#include <frc/kinematics/ChassisSpeeds.h>
#include <frc/kinematics/SwerveModuleState.h>

#include "Twist.hpp"
#include "Wheel.hpp"
#include "RobotState.hpp"
#include <AHRS.h>
#include <array>
#include <frc/geometry/Translation2d.h>
#include <thread>
#include <iostream>
#include <memory>

// #include <cmath>

inline static std::array<std::unique_ptr<Wheel>, 4> wheels {
    std::make_unique<Wheel>(WHEELS::WHEEL_1),
    std::make_unique<Wheel>(WHEELS::WHEEL_2),
    std::make_unique<Wheel>(WHEELS::WHEEL_3),
    std::make_unique<Wheel>(WHEELS::WHEEL_4)
};

inline static frc::SwerveDriveKinematics<4> const m_kinematics {
    *wheels[0],
    *wheels[1],
    *wheels[2],
    *wheels[3]
};

inline static std::unique_ptr<AHRS> navx { std::make_unique<AHRS>(frc::SPI::Port::kMXP) };


/******************************************************************/
/*                      Non Static Functions                      */
/******************************************************************/

void Drivetrain::init()
{
    reset_gyro();
}

void Drivetrain::reset_gyro()
{
    navx->ZeroYaw();
}

double Drivetrain::get_angle()
{
    return -navx->GetAngle() + 90;
}

void Drivetrain::drive(frc::ChassisSpeeds const& feild_speeds)
{
    auto const speeds = frc::ChassisSpeeds::FromFieldRelativeSpeeds(
        feild_speeds.vx,
        feild_speeds.vy,
        feild_speeds.omega,
        frc::Rotation2d { units::degree_t { get_angle() } });
    auto const module_states = m_kinematics.ToSwerveModuleStates(speeds);
    drive(module_states);
}


void Drivetrain::drive(wpi::array<frc::SwerveModuleState, 4> const& module_states)
{
    std::vector<std::thread> t;
    unsigned                 wheel_idx = 0;
    for(auto&& state : module_states)
        t.emplace_back(wheels[wheel_idx++]->drive(state));
    for(auto&& ts : t)
        ts.join();
}


void Drivetrain::face_direction(units::meters_per_second_t dx, units::meters_per_second_t dy, units::degree_t theta)
{
    auto const currentRotation = units::degree_t { get_angle() };

    int const errorTheta      = ((currentRotation - theta).to<int>()%360-180-90)%360;
    auto const rotateP         = 1.25;
    auto       pRotation       = errorTheta * rotateP;
    if(ngr::fabs(pRotation) > 40)
         pRotation = 40 * ((pRotation > 0)?1:-1);
    drive(frc::ChassisSpeeds{ dx, dy, units::degrees_per_second_t{pRotation} });
}

void Drivetrain::face_closest(units::meters_per_second_t dx, units::meters_per_second_t dy)
{
    auto const currentRotation = get_angle();
    auto const errorTheta      = (ngr::fabs(currentRotation) <= 90)
                                ? currentRotation
                                : currentRotation - 180;
    face_direction(dx, dy, units::degree_t {errorTheta});
}

inline static std::thread drive_thread;

bool auton_stop_flag = false;
void Drivetrain::auton_drive(units::meters_per_second_t dx, units::meters_per_second_t dy, units::degree_t direction)
{
    auton_stop_flag = true;     // stop previous thread
    if(drive_thread.joinable()) // verify drive thread was running, should only happen on first run
        drive_thread.join();    // wait for drivethread to finish
    auton_stop_flag = false;    // reset stop flag
    drive_thread    = std::thread { [dx, dy, direction]() {
        while(! auton_stop_flag && RobotState::IsAutonomousEnabled())
        {
            face_direction(dx, dy, direction);
            std::this_thread::sleep_for(20ms); // don't hog the cpu
        }
    } };
}

void Drivetrain::stop()
{
    auton_stop_flag = true;
        // note this takes advantage of the fact that
        // the thread starts with the wheel instructions
        // and therefore we dont need to wait for it to
        // finish the loop as long as it won't tell the
        // wheels to do anything
    std::this_thread::sleep_for(1ms); // make sure it has time to stop the thread
    for(auto&& wheel : wheels)
        wheel->stop();
}

void Drivetrain::goto180()
{
    auton_stop_flag = true;
    wpi::array<frc::SwerveModuleState, 4> states {
        frc::SwerveModuleState { 0_mps, frc::Rotation2d { units::degree_t { 180 } } },
        frc::SwerveModuleState { 0_mps, frc::Rotation2d { units::degree_t { 180 } } },
        frc::SwerveModuleState { 0_mps, frc::Rotation2d { units::degree_t { 180 } } },
        frc::SwerveModuleState { 0_mps, frc::Rotation2d { units::degree_t { 180 } } }
    };
    drive(states);
}

void Drivetrain::gotoZero()
{
    auton_stop_flag = true;
    wpi::array<frc::SwerveModuleState, 4> states {
        frc::SwerveModuleState { 0_mps, frc::Rotation2d { units::degree_t { 0 } } },
        frc::SwerveModuleState { 0_mps, frc::Rotation2d { units::degree_t { 0 } } },
        frc::SwerveModuleState { 0_mps, frc::Rotation2d { units::degree_t { 0 } } },
        frc::SwerveModuleState { 0_mps, frc::Rotation2d { units::degree_t { 0 } } }
    };
    drive(states);
}

void Drivetrain::PrintWheelAngle(int wheelid)
{
    wheels[wheelid]->printAngle();
}