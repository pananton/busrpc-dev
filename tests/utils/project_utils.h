#pragma once

#include "entities/project.h"
#include "utils/file_utils.h"

#include <memory>
#include <string>
#include <vector>

namespace busrpc { namespace test {

std::string GetFileHeader(const std::string& packageName,
                          const std::vector<std::string>& imports = {},
                          bool doNotImportMain = false);
std::string GetTestEnum();
std::string GetTestStruct();
std::string GetMainFile();
std::string GetNamespaceDescriptor();
std::string GetClassDescriptor();
std::string GetStaticClassDescriptor();
std::string GetMethodDescriptor();
std::string GetOnewayMethodDescriptor();
std::string GetStaticMethodDescriptor();
std::string GetOnewayStaticMethodDescriptor();
std::string GetServiceDescriptor();

void CreateMinimalProject(TmpDir& projectDir);
void CreateTestProject(TmpDir& projectDir);
}} // namespace busrpc::test