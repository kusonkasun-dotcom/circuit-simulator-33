#pragma once
#include "core/Result.h"
#include "project/Project.h"
#include <QString>

namespace ardulab {

// Reads/writes .fal project files (structured JSON with a schema version).
// Saving is atomic (temp file + rename). A corrupt file yields a clear error
// and never overwrites the caller's in-memory project.
class ProjectSerializer {
public:
    static Status save(const Project& project, const QString& path);
    static Result<Project> load(const QString& path);
};

} // namespace ardulab
