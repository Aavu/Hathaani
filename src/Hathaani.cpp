#include "Hathaani.h"
#include "Util.h"

Hathaani* Hathaani::pInstance = nullptr;

Hathaani::Hathaani() : RPDOTimer(TIMER_CH1),
m_portHandler(DXL_DEVICE_NAME) {
    m_pBowController = BowController::create(m_portHandler);
    m_pFingerController = FingerController::create(m_portHandler);
}

Hathaani* Hathaani::create() {
    pInstance = new Hathaani();
    return pInstance;
}

void Hathaani::destroy(Hathaani* pInstance) {
    delete pInstance;
}

int Hathaani::init(bool shouldHome) {
    pInstance = this;
    // Comm
    if (!CanBus.begin(CAN_BAUD_1000K, CAN_STD_FORMAT)) {
        LOG_ERROR("CAN open failed");
        return 1;
    }

    CanBus.attachRxInterrupt(canRxHandle);

    if (m_portHandler.openPort() != 0) {
        LOG_ERROR("Open Port");
        return 1;
    }

    if (m_portHandler.setBaudRate(DXL_BAUDRATE) != 0) {
        LOG_ERROR("Set BaudRate");
        return 1;
    }

    // Bow
    int err = m_pBowController->init(shouldHome);
    if (err != 0) {
        LOG_ERROR("Bow Controller Init failed");
        return 1;
    }

    // Finger
    RPDOTimer.stop();
    RPDOTimer.setPeriod(PDO_RATE * 1000);
    RPDOTimer.attachInterrupt(RPDOTimerIRQHandler);

    // test dummy
    return 0;

    return m_pFingerController->init(shouldHome);
}

int Hathaani::reset() {
    int err;
    err = m_pBowController->reset();
    BowController::destroy(m_pBowController);

    err = m_pFingerController->reset();
    FingerController::destroy(m_pFingerController);

    CanBus.end();

    return err;
}

int Hathaani::enableEncoderTransmission(bool bEnable) {
    return m_pFingerController->enablePDO(bEnable);
}

int Hathaani::perform(const performParam_t& param, float lpf_alpha, bool shouldBow) {
    m_bShouldBow = shouldBow;
    int err;

    if (shouldBow)
        err = m_pBowController->prepareToPlay();

    // Shallow copy
    m_performParam = param;

    m_performParam.positions = new int32_t[m_performParam.length];
    Util::cleanUpData(m_performParam.pitches, m_performParam.length, lpf_alpha);
    Util::convertPitchToInc(m_performParam.positions, m_performParam.pitches, m_performParam.length, 0);
    // m_performParam.length += kOffset;

    // for (int i = 0; i < m_performParam.length; ++i) {
    //     Serial.println(m_performParam.positions[i]);
    //     // Serial.print(",");
    // }
    // Serial.println();

    if (!Util::checkFollowError(m_performParam.positions, m_performParam.length)) {
        LOG_ERROR("Playing this phrase will violate max follow window... Aborting");
        err = 1;
        goto cleanup;
    }

    err = m_pFingerController->fingerOff();
    if (err != 0) {
        LOG_ERROR("fingerOff");
        goto cleanup;
    }
    m_bFingerOn = false;

    err = m_pFingerController->prepareToPlay(m_performParam.pitches[0]);
    delay(10);

    /*********** Switch to Cyclic Position to use PDO ***********/
    err = m_pFingerController->getReadyToPlay();
    if (err != 0) {
        LOG_ERROR("getReadyToPlay");
        goto cleanup;
    }

    m_bPlaying = true;
    RPDOTimer.start();

    // Wait for the performance to complete
    while (m_bPlaying) {
        delay(PDO_RATE);
    }

    RPDOTimer.stop();
    delay(10);

    err = m_pFingerController->fingerOff();
    if (err != 0)
        LOG_ERROR("fingerOff");
    m_bFingerOn = false;

    if (shouldBow)
        err = m_pBowController->reset();

cleanup:
    delete[] m_performParam.positions;
    return err;
}

int Hathaani::bowTest(const performParam_t& param) {
    int err;
    err = m_pBowController->prepareToPlay();
    m_performParam = param;
    m_bPlaying = true;
    RPDOTimer.start();

    // Wait for the performance to complete
    while (m_bPlaying) {
        delay(PDO_RATE);
    }

    m_pBowController->enablePDO(false);
    pInstance->RPDOTimer.stop();

    return err;
}
void Hathaani::RPDOTimerIRQHandler() {
    static int i = 0;
    static int32_t lastPos = 0;
    performParam_t& param = pInstance->m_performParam;
    int kTotalLength = param.length;
    int32_t pos = BOW_ENCODER_MIN + param.bowTraj[i] * (BOW_ENCODER_MAX - BOW_ENCODER_MIN);
    ++i;
    pInstance->m_pBowController->setPosition(pos, false, true);
    if (i < kTotalLength) {
        lastPos = pos;
    } else {
        pInstance->m_bPlaying = false;
        i = kTotalLength - 1;
    }
}

// void Hathaani::RPDOTimerIRQHandler() {
//     static int32_t iBowingLength = 0;
//     static int32_t lastPos = 0;
//     static bool dir = 0;    // start with down bow
//     static bool bBowDirChange = false;

//     float p0 = (dir == 0) ? BOW_ENCODER_MIN : BOW_ENCODER_MAX;
//     float pf = (dir == 0) ? BOW_ENCODER_MAX : BOW_ENCODER_MIN;

//     performParam_t& param = pInstance->m_performParam;
//     int kTotalLength = param.length;
//     uint32_t len = pInstance->m_fBowTestLen_s * 1000.0 / PDO_RATE;
//     int nBows = pInstance->m_iNBows;
//     float a[3] = { p0, 3.f * (pf - p0), -2.f * (pf - p0) };

//     int32_t i = iBowingLength % len;
//     double t = i / (double) len;

//     int32_t iPos = a[0] + a[1] * pow(t, 2) + a[2] * pow(t, 3);
//     iBowingLength++;

//     if (iBowingLength < len) {
//         lastPos = iPos;
//         bBowDirChange = false;
//     } else if (iBowingLength == len) {
//         bBowDirChange = true;
//     } else {
//         if (bBowDirChange) {
//             dir ^= 1;
//             bBowDirChange = false;
//         }
//     }

//     if (iBowingLength >= 2 * len) {
//         pInstance->m_bPlaying = false;
//         pInstance->m_pBowController->setPosition(lastPos, bBowDirChange, true);
//     } else {
//         pInstance->m_pBowController->setPosition(iPos, bBowDirChange, true);
//         lastPos = iPos;
//     }
// }

void Hathaani::canRxHandle(can_message_t* arg) {
    auto id = arg->id - COB_ID_SDO_SC;
    // TODO: Improve Implementation
    // Bow
    if (id == BOW_NODE_ID) {
        pInstance->m_pBowController->setRxMsg(*arg);
    }

    if (arg->id == COB_ID_TPDO3 + BOW_NODE_ID) {
        pInstance->m_pBowController->PDO_processMsg(*arg);
    }

    if (arg->id == COB_ID_EMCY + BOW_NODE_ID) {
        pInstance->m_pBowController->handleEMCYMsg(*arg);
    }

    // Finger
    if (id == FINGER_NODE_ID) {
        pInstance->m_pFingerController->setRxMsg(*arg);
    }

    if (arg->id == COB_ID_TPDO3 + FINGER_NODE_ID) {
        pInstance->m_pFingerController->PDO_processMsg(*arg);

        // Serial.print("ID : ");
        // Serial.print(arg->id, HEX);
        // Serial.print(", Length : ");
        // Serial.print(arg->length);
        // Serial.print(", Data : ");
        // for (int i = 0; i < arg->length; i++) {
        //     Serial.print(arg->data[i], HEX);
        //     Serial.print(" ");
        // }
        // Serial.println();

    }

    if (arg->id == COB_ID_EMCY + FINGER_NODE_ID) {
        pInstance->m_pFingerController->handleEMCYMsg(*arg);
    }
}
