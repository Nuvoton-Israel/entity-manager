/*
// Copyright (c) 2017 Intel Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
*/

#pragma once
#include <boost/container/flat_map.hpp>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <sdbusplus/asio/connection.hpp>
#include <sdbusplus/exception.hpp>

constexpr const char* configurationOutDir = "/var/configuration/";
constexpr const char* versionHashFile = "/var/configuration/version";
constexpr const char* versionFile = "/etc/os-release";

bool findFiles(const std::filesystem::path& dirPath,
               const std::string& matchString,
               std::vector<std::filesystem::path>& foundPaths);

bool getI2cDevicePaths(
    const std::filesystem::path& dirPath,
    boost::container::flat_map<size_t, std::filesystem::path>& busPaths);

bool validateJson(const nlohmann::json& schemaFile,
                  const nlohmann::json& input);

bool isPowerOn(void);
void setupPowerMatch(const std::shared_ptr<sdbusplus::asio::connection>& conn);
struct DBusInternalError final : public sdbusplus::exception_t
{
    const char* name() const noexcept override
    {
        return "org.freedesktop.DBus.Error.Failed";
    };
    const char* description() const noexcept override
    {
        return "internal error";
    };
    const char* what() const noexcept override
    {
        return "org.freedesktop.DBus.Error.Failed: "
               "internal error";
    };
};

inline bool fwVersionIsSame(void)
{
    std::ifstream version(versionFile);
    if (!version.good())
    {
        std::cerr << "Can't read " << versionFile << "\n";
        return false;
    }

    std::string versionData;
    std::string line;
    while (std::getline(version, line))
    {
        versionData += line;
    }

    std::string expectedHash =
        std::to_string(std::hash<std::string>{}(versionData));

    std::filesystem::create_directory(configurationOutDir);
    std::ifstream hashFile(versionHashFile);
    if (hashFile.good())
    {

        std::string hashString;
        hashFile >> hashString;

        if (expectedHash == hashString)
        {
            return true;
        }
        hashFile.close();
    }

    std::ofstream output(versionHashFile);
    output << expectedHash;
    return false;
}