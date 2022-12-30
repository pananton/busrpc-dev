#pragma once

#include "entities/project.h"

/// \dir generators Documentation generators.
/// \file generator.h Base class for documentation generators.

namespace busrpc {

/// Documentation generator base class.
class DocGenerator {
public:
    /// Generate documentation for \a project.
    virtual void generate(const Project& project) const = 0;

    /// Default virtual destructor.
    virtual ~DocGenerator() = default;
};
} // namespace busrpc
