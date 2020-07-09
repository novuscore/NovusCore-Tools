#include <NovusTypes.h>
#include <fstream>

#include <ShaderCooker/ShaderCooker.h>
#include <Utils/DebugHandler.h>

i32 main(int argc, char* argv[])
{
    if (argc != 3)
    {
        NC_LOG_ERROR("Expected two parameters, got %i. Usage: <input> <output>", argc)
    }

    std::string inputFile = argv[1];
    std::string outputFile = argv[2];

    ShaderCooker::ShaderCooker shaderCooker;
    
    char* blob;
    size_t size;
    shaderCooker.CompileFile(inputFile, blob, size);

    std::ofstream ofstream(outputFile, std::ios::trunc | std::ofstream::binary);
    ofstream.write(blob, size);
    ofstream.close();

    return 0;
}