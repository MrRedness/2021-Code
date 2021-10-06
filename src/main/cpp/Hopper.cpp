#include "Hopper.hpp"
#include <frc/DigitalInput.h>

inline static rev::CANSparkMax      indexer { HOPPER::INDEXER::PORT, rev::CANSparkMaxLowLevel::MotorType::kBrushless };
inline static rev::CANSparkMax      transport { HOPPER::TRANSPORT::PORT, rev::CANSparkMaxLowLevel::MotorType::kBrushless };
inline static rev::CANPIDController pidController = transport.GetPIDController();
inline static rev::CANEncoder       encoder       = transport.GetEncoder();
inline static frc::DigitalInput     limitSwitch { HOPPER::LIMIT_SWITCH };
inline static int                   numberOfBalls  = 3;
inline static double                targetDistance = HOPPER::TRANSPORT::DISTANCE;
inline static bool                  isTransporting = false;
inline static std::atomic<bool>     invalidStopFlag { false };

//private functions
// I found this method in the .hpp file but it is never defined
//    void driveDistance();

// ^Make it

/******************************************************************/
/*                      Non Static Functions                      */
/******************************************************************/

void Hopper::init()
{
    // indexer.Set(HOPPER::INDEXER::SPEED);


    indexer.SetIdleMode(HOPPER::INDEXER::IDLE_MODE);
    transport.SetIdleMode(HOPPER::TRANSPORT::IDLE_MODE);

    pidController.SetP(HOPPER::TRANSPORT::P);
    pidController.SetI(HOPPER::TRANSPORT::I);
    pidController.SetD(HOPPER::TRANSPORT::D);

    pidController.SetFeedbackDevice(encoder);
    pidController.SetOutputRange(-HOPPER::TRANSPORT::SPEED, HOPPER::TRANSPORT::SPEED);

    encoder.SetPosition(0);
}

bool Hopper::index(bool warn_if_shooting)
{
    if(invalidStopFlag)
    {
        if(warn_if_shooting)
            std::cerr << "Stop not called after shooting: Indexer Aborting\n";
        return false;
    }

    if(! limitSwitch.Get() && numberOfBalls < 3 && ! isTransporting)
    {
        pidController.SetReference(targetDistance, rev::ControlType::kPosition);
        numberOfBalls++;
        isTransporting = true;
    }

    if(isTransporting && encoder.GetPosition() > (targetDistance - HOPPER::TRANSPORT::TOLERANCE))
    {
        targetDistance += HOPPER::TRANSPORT::DISTANCE;
        isTransporting = false;
    }

    if(limitSwitch.Get() && numberOfBalls < 4)
        indexer.Set(HOPPER::INDEXER::SPEED);
    else
        indexer.Set(0);
    return true;
}

void Hopper::shoot()
{
    invalidStopFlag = true;
    // indexer.Set(HOPPER::INDEXER::SPEED);
    transport.Set(HOPPER::TRANSPORT::SHOOT_SPEED);
}

void Hopper::stop()
{
    invalidStopFlag = false;
    isTransporting  = false;
    numberOfBalls   = 0;
    targetDistance  = HOPPER::TRANSPORT::DISTANCE;
    encoder.SetPosition(0);


    transport.Set(0);
}