#pragma once

#include "entities/project.h"
#include "exception.h"
#include "types.h"

#include <optional>
#include <string>

#define EXPECT_COMMAND_EXCEPTION(OP, CODE)     \
    try {                                      \
        (OP);                                  \
        ADD_FAILURE() << "No exception";       \
    } catch (const busrpc::command_error& e) { \
        EXPECT_EQ(e.code(), (CODE));           \
    }

#define EXPECT_ENTITY_EXCEPTION(OP, ETYPE, DNAME)     \
    try {                                             \
        (OP);                                         \
        ADD_FAILURE() << "No exception";              \
    } catch (const busrpc::entity_error& e) {         \
        EXPECT_EQ(e.type(), (ETYPE));                 \
        EXPECT_EQ(e.dname(), (DNAME));                \
    } catch (...) {                                   \
        ADD_FAILURE() << "Unexpected exception type"; \
    }

#define EXPECT_NAME_CONFLICT_EXCEPTION(OP, ETYPE, DNAME, CONFLICTING_NAME) \
    try {                                                                  \
        (OP);                                                              \
        ADD_FAILURE() << "No exception";                                   \
    } catch (const busrpc::name_conflict_error& e) {                       \
        EXPECT_EQ(e.type(), (ETYPE));                                      \
        EXPECT_EQ(e.dname(), (DNAME));                                     \
        EXPECT_EQ(e.conflictingName(), (CONFLICTING_NAME));                \
    } catch (...) {                                                        \
        ADD_FAILURE() << "Unexpected exception type";                      \
    }

namespace busrpc { namespace test {

bool IsHelpMessage(const std::string& msg, std::optional<CommandId> commandId = std::nullopt);
}} // namespace busrpc::test
