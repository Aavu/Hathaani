//
// Created by Raghavasimhan Sankaranarayanan on 01/22/22.
// 

#pragma once

// CANbus
#define PDO_RATE 6 // ms
#define CAN_MSG_BUF_LEN 16

// Epos4
#define FINGER_NODE_ID 1
#define BOW_NODE_ID 2

#define SCALE_LENGTH 320
#define P2P_MULT 100.f // 10000.f/100.f // pulse per mm
#define NUT_POS 400 // Abs value. No Negative numbers

// As fret number increases if the encoder reading is more negative, set it to -1
#define ENCODER_DIR -1
#define CALLBACK_ENC_THRESHOLD 5
#define MAX_ENCODER_INC 26000
#define MAX_FOLLOW_ERROR 10000

// Dxl
// To make your ENUM Scoped, you need to declare it enum class
enum class DxlID {
    Finger = 1,
    String = 2,
    Pitch = 3,
    Roll = 4,
    Yaw = 5
};

// string

enum class StringPos {
    E = 72,
    A = 90,
    D = 110,
    G = 120
};

#define BOW_ENCODER_MIN 10000
#define BOW_ENCODER_MAX 40000
#define PROFILE_VELOCITY_PITCH 200
#define BOW_LIFT_MULTIPLIER 1.5 // Increase this number to increase bow lift towards the edges

#define PITCH_MAX 178 //176   // Away from strings
#define PITCH_MIN 188   // Pressing the strings
#define PITCH_AVG 183.5f

#define ROLL_MAX 100    // Near bridge
#define ROLL_MIN 142    // Near Fingerboard
#define ROLL_AVG 120

enum class YawPos {  // Angles in degrees
    Mid = 2048,
    E = 2048 - 24,
    A = 2048 - 8,
    D = 2048 + 8,
    G = 2048 + 24
};

#define FINGER_ON 252.f         // deg
#define FINGER_OFF 246.f        // deg
#define FINGER_RISE_LEVEL 7    // The amount of change in angle as the finger move along the fret

#define DXL_UPDATE_RATE 50

#define DXL_MAX_CURRENT 50