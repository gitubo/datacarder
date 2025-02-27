#include <chrono>
#include "opencmd.hpp"

int main() {
    using namespace opencmd;
    Logger& logger = Logger::getInstance();
    logger.setSeverity(Logger::Level::DEBUG);

    const std::string fileName = "../catalog/can_extended.json";
    nlohmann::json jsonData;
    std::ifstream inputFile(fileName);
    if (!inputFile.is_open()) {
        logger.log("Errore: Impossibile aprire il file " + fileName, Logger::Level::ERROR);
        return 1;
    }
    try {
        inputFile >> jsonData;
        inputFile.close();
        Logger::getInstance().log("JSON file loaded", Logger::Level::INFO);
    } catch (const nlohmann::json::parse_error& e) {
        logger.log("Catalog file not correctly parsed: " + std::string(e.what()), Logger::Level::ERROR);
        return 2;
    }

    std::string schemaName = "can_extended";
    if(SchemaCatalog::getInstance().parseSchema(schemaName, jsonData) < 0){
        Logger::getInstance().log("Error in parsing the schema", Logger::Level::ERROR);
    }
    
    nlohmann::ordered_json result;
//    BitStream bs_b64 = BitStream(std::string("AUBAIGhgL/A="));
    BitStream bs_b64 = BitStream(std::string("ABwAAYX/VAAz/4A="));

    logger.info("bitstream_to_json");
    auto start = std::chrono::high_resolution_clock::now();
    auto rn_b2j = SchemaCatalog::getInstance().getAbstractChain(schemaName);
    if(!rn_b2j){
        Logger::getInstance().log("Error in evaluating schema", Logger::Level::ERROR);
        return 1;
    }
    int retVal = rn_b2j->bitstream_to_json(bs_b64, result);
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
    logger.log("bitstream_to_json returned <" + std::to_string(retVal) + ">, evaluation time " + std::to_string(duration.count()) + " ns", Logger::Level::INFO);

    logger.log("JSON: " + result.unflatten().dump(), Logger::Level::INFO);

    BitStream output_bs = BitStream();
    logger.log("json_to_bitstream", Logger::Level::INFO);
    start = std::chrono::high_resolution_clock::now();
    auto rn_j2b = SchemaCatalog::getInstance().getAbstractChain(schemaName);
    if(!rn_j2b){
        Logger::getInstance().log("Error in evaluating schema", Logger::Level::ERROR);
        return 1;
    }
    retVal = rn_j2b->json_to_bitstream(result, output_bs);
    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
    logger.log("json_to_bitstream returned <" + std::to_string(retVal) + ">, evaluation time " + std::to_string(duration.count()) + " ns", Logger::Level::INFO);

    logger.log("BITSTREAM: " + output_bs.to_base64(), Logger::Level::INFO);

    return 0;
}
