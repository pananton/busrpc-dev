#pragma once

#include "exception.h"

#define EXPECT_COMMAND_EXCEPTION(OP, CODE)     \
    try {                                      \
        (OP);                                  \
        ADD_FAILURE() << "No exception";       \
    } catch (const busrpc::command_error& e) { \
        EXPECT_EQ(e.code(), (CODE));           \
    }
