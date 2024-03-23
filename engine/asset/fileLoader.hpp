#pragma once
/* simply copied from nvpro_cores/nvh */
// TODO: needs a totally rewrite of this file

#include <string>
#include <vector>
#include <fstream>
#include <filesystem>

#include <log.hpp>

static inline std::string findFile(const std::string &infilename, const std::vector<std::string> &directories, bool warn = false)
{
    std::ifstream stream;

    {
        stream.open(infilename.c_str());
        if (stream.is_open())
        {
            ENGINE_LOG_INFO("Found file {}.", infilename);
            return infilename;
        }
    }

    for (const auto &directory : directories)
    {
        std::string filename = directory + "/" + infilename;
        stream.open(filename.c_str());
        if (stream.is_open())
        {
            ENGINE_LOG_INFO("Found file {}.", filename);
            return filename;
        }
    }

    if (warn)
    {
        std::string all_directories{};
        for (const auto &directory : directories)
            all_directories += directory + " - ";
        ENGINE_LOG_WARN("File {} not found in directories {}.", infilename, all_directories);
    }

    return {};
}

static inline std::string loadFile(const std::string &filename, bool binary)
{
    std::string result;
    std::ifstream stream(filename, std::ios::ate | (binary ? std::ios::binary : std::ios_base::openmode(0)));

    if (!stream.is_open())
    {
        return result;
    }

    result.reserve(stream.tellg());
    stream.seekg(0, std::ios::beg);

    result.assign((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
    return result;
}

static inline std::string loadFile(const char *filename, bool binary)
{
    std::string name(filename);
    return loadFile(name, binary);
}

static inline std::string loadFile(const std::string &filename,
                                   bool binary,
                                   const std::vector<std::string> &directories,
                                   std::string &filenameFound,
                                   bool warn = false)
{
    filenameFound = findFile(filename, directories, warn);
    if (filenameFound.empty())
    {
        return {};
    }
    else
    {
        return loadFile(filenameFound, binary);
    }
}

static inline std::string loadFile(const std::string filename, bool binary, const std::vector<std::string> &directories, bool warn = false)
{
    std::string filenameFound;
    return loadFile(filename, binary, directories, filenameFound, warn);
}