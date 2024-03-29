//
// Created by Raghavasimhan Sankaranarayanan on 2020-01-14.
//
#include <cstring>
#include <iostream>

#include "Hathaani.h"
#include "PitchFileParser.h"
#include "Logger.h"

const int8_t TRANSPOSE = 1;

//#define SET_HOME
 
using namespace std;

int main(int argc, char **argv) {
    Logger::init(Logger::info);
    Error_t lResult = kNoError;

    std::vector<float> pitches, amplitude;
    std::vector<size_t> bowChangeIdx;

    if (argc > 1)
    {
        PitchFileParser pitchFileParser(argv[1]);
        lResult = pitchFileParser.parseJson(pitches, bowChangeIdx, amplitude);
        if (lResult != kNoError)
        {
            LOG_ERROR("{} - {}", PitchFileParser::kName, lResult);
            CUtil::PrintError(PitchFileParser::kName, lResult);
            return 1;
        }
        cout << "Read pitches successfully!\n";
    }

    Hathaani hathaani;

    if ((lResult = hathaani.init()) != kNoError) {
        Hathaani::LogError("Main init", lResult, 0);
        return lResult;
    }
#ifdef SET_HOME
    return 0;
#endif //SET_HOME

//    lResult = hathaani.ApplyRosin(10);
//    if (lResult != kNoError) {
//        Hathaani::LogError("ApplyRosin", lResult, 0);
//        return EXIT_FAILURE;
//    }

//    pitches[0] = 0;
//    for (size_t i=1; i<pitches.size(); ++i) {
//        pitches[i] = min(pitches[i-1] + 0.025, 15.0);
//    }
//
//    bowChangeIdx.clear();
//
//    for (float & a : amplitude) {
//        a = 0.65;
//    }

//    for (unsigned int i=0; i<amplitude.size(); ++i) {
//        amplitude[i] = i * 1.f / amplitude.size();
//    }
//
    if ((lResult = hathaani.Perform(pitches, bowChangeIdx, amplitude, 0.25, TRANSPOSE)) != kNoError) {
        LOG_ERROR("Perform error");
        return EXIT_FAILURE;
    }
//    if ((lResult = hathaani.Play(0.5)) != kNoError) {
//        LOG_ERROR("Play Error");
//        return EXIT_FAILURE;
//    }

//    hathaani.Perform(Hathaani::Key::C, Hathaani::Mode::Major, 500, 0.5);

    // Use interval of 500 for Khandippu and 400 for the rest
//    if ((lResult = hathaani.Perform(Hathaani::Khandippu, 0, 500, 0.5)) != kNoError)
//    {
//        LOG_ERROR("Play error");
//        return lResult;
//    }

    return 0;
}
