#include <NovusTypes.h>
#include <fstream>
#include <filesystem>
#include <ShaderCooker/ShaderCooker.h>
#include <Utils/DebugHandler.h>

i32 main(int argc, char* argv[])
{
    if (argc != 4)
    {
        NC_LOG_ERROR("Expected three parameters, got %i. Usage: <shader_path> <shader_source_dir> <shader_bin_dir>", argc);
        return -1;
    }

    std::string inputFileName = argv[1];
    std::string sourceDir = argv[2];
    std::string binDir = argv[3];

    // Create and init shaderCooker
    ShaderCooker::ShaderCooker shaderCooker;
    shaderCooker.AddIncludeDir(sourceDir);

    // Get the correct paths
    std::filesystem::path shaderPath = std::filesystem::path(sourceDir) / inputFileName;
    shaderPath = std::filesystem::absolute(shaderPath.make_preferred());

    std::string outputFileName = inputFileName + ".spv";
    std::filesystem::path outputPath = std::filesystem::path(binDir) / outputFileName;
    outputPath = std::filesystem::absolute(outputPath.make_preferred());

    // Compile shader
    char* blob;
    size_t size;
    shaderCooker.CompileFile(shaderPath, blob, size);

    // Create output directories
    std::filesystem::create_directories(outputPath.parent_path());

    // Output file
    std::ofstream ofstream(outputPath, std::ios::trunc | std::ofstream::binary);
    ofstream.write(blob, size);
    ofstream.close();

    return 0;
}