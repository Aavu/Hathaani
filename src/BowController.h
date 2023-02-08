//
// Created by Raghavasimhan Sankaranarayanan on 01/20/22.
// 

#pragma once

#include "def.h"
#include "logger.h"
#include "epos4/epos4.h"
#include "dxl/dxl.h"
#include "Util.h"

class BowController {
public:
    enum class BowDirection {
        None = -1,
        Up = 0,
        Down = 1
    };

    enum class BowState {
        Stopped = 0,
        Bowing = 1
    };

    // Not thread safe. It is ok here since this is guaranteed to be called only once - inside setup()
    static BowController* create(PortHandler& portHandler);
    static void destroy(BowController* pInstance);

    // Delete copy and assignment constructor
    BowController(FingerController&) = delete;
    void operator=(const BowController&) = delete;

    int init(bool shouldHome = true);
    int reset();

    int setHome();

    int enable(bool bEnable = true);
    int prepareToPlay(const int bowChanges[], int numChanges, int nPitches);

    int startBowing(float amplitude = 0.5, BowDirection direction = BowDirection::None);
    int stopBowing();

    int changeDirection();

    int enablePDO(bool bEnable = true);
    int setNMTState(NMTState nmtState);
    int setPosition(int32_t iPosition, bool bDirChange = false, bool bPDO = true);
    int updatePosition(int i);
    int setVelocity(int32_t iVelocity, bool bPDO = true);

    int setRxMsg(can_message_t& msg);
    int PDO_processMsg(can_message_t& msg);
    void handleEMCYMsg(can_message_t& msg);

private:
    bool m_bInitialized = false;
    PortHandler* m_pPortHandler;
    BowState m_bowingState;
    BowDirection m_CurrentDirection = BowDirection::Down;

    float* m_afBowTrajectory = nullptr;
    int m_iNPitches = 0;

    Epos4 m_epos;
    volatile int32_t m_iCurrentPosition = 0;

    Dynamixel m_pitchDxl, m_rollDxl, m_yawDxl;
    float m_fPitch = PITCH_AVG;
    HardwareTimer m_dxlTimer;

    BowController(PortHandler& portHandler);
    ~BowController();

    static BowController* pInstance;

    int computeBowTrajectory(const int bowChanges[], int numChanges, int nPitches);
    void encoderPositionCallback(int32_t encPos);
    static void dxlUpdateCallback();
};
