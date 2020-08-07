#include <NovusTypes.h>
#include <fstream>

#include <ShaderCooker/ShaderCooker.h>
#include <Utils/DebugHandler.h>

i32 main(int argc, char* argv[])
{
    if (argc != 4)
    {
        NC_LOG_ERROR("Expected three parameters, got %i. Usage: <input> <output> <includeDir>", argc)
    }

    std::string inputFile = argv[1];
    std::string outputFile = argv[2];
    std::string includeDir = argv[3];

    ShaderCooker::ShaderCooker shaderCooker;
    
    shaderCooker.AddIncludeDir(includeDir);

    char* blob;
    size_t size;
    shaderCooker.CompileFile(inputFile, blob, size);

    std::ofstream ofstream(outputFile, std::ios::trunc | std::ofstream::binary);
    ofstream.write(blob, size);
    ofstream.close();

    return 0;
}