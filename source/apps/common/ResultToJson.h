#pragma once

#include "lib/vcaLib.h"
#include <iostream>
#include <nlohmann/json.hpp>

[[maybe_unused]] void to_json(nlohmann::json &j, const vca_frame_results &frameResults)
{
    j = nlohmann::json{
        {"averageBrightness", frameResults.averageBrightness},
        {"averageEnergy", frameResults.averageEnergy},
        {"sad", frameResults.sad},
        {"averageU", frameResults.averageU},
        {"averageV", frameResults.averageV},
        {"energyU", frameResults.energyU},
        {"energyV", frameResults.energyV},
        {"epsilon", frameResults.epsilon},
        {"poc", frameResults.poc},
        {"isNewShot", frameResults.isNewShot}};
}

[[maybe_unused]] void from_json(const nlohmann::json &j, vca_frame_results &frameResults)
{
    j.at("averageBrightness").get_to(frameResults.averageBrightness);
    j.at("averageEnergy").get_to(frameResults.averageEnergy);
    j.at("sad").get_to(frameResults.sad);
    j.at("averageU").get_to(frameResults.averageU);
    j.at("averageV").get_to(frameResults.averageV);
    j.at("energyU").get_to(frameResults.energyU);
    j.at("energyV").get_to(frameResults.energyV);
    j.at("epsilon").get_to(frameResults.epsilon);
    j.at("poc").get_to(frameResults.poc);
    j.at("isNewShot").get_to(frameResults.isNewShot);
}