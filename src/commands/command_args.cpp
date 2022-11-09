#include "commands/command_args.h"

CommandArgs::CommandArgs(): cmd_("help"), root_(std::filesystem::current_path()) { }

CommandArgs::CommandArgs(const int argc, const char* argv[])
{
    root_ = std::filesystem::current_path();

    size_t currentArg = 0;
    auto isNamedArgKey = [argv](size_t idx) { return argv[idx][0] == '-'; };

    if (argc != 0) {
        cmd_ = argv[currentArg++];
    } else {
        cmd_ = "help";
    }

    // handling named arguments

    while (currentArg < argc && isNamedArgKey(currentArg)) {
        std::string key(&argv[currentArg++][1]);
        std::optional<std::string> value;

        if (key != "-") {
            if (currentArg < argc && !isNamedArgKey(currentArg)) {
                value.emplace(argv[currentArg++]);
            }
        } else {
            break;
        }

        if (key != "r") {
            namedArgs_[std::move(key)] = std::move(value);
        } else {
            if (value) {
                try {
                    root_ = std::filesystem::path(*value);
                } catch (const std::filesystem::filesystem_error&) {
                    throw args_error("invalid busrpc root directory");
                }
            } else {
                throw args_error("busrpc root directory is not specified");
            }
        }
    }

    // handling positional arguments

    while (currentArg < argc) {
        posArgs_.emplace_back(argv[currentArg++]);
    }
}

const std::string& CommandArgs::cmd() const noexcept
{
    return cmd_;
}

const std::filesystem::path& CommandArgs::root() const noexcept
{
    return root_;
}

const NamedArgs& CommandArgs::namedArgs() const noexcept
{
    return namedArgs_;
}

const PosArgs& CommandArgs::posArgs() const noexcept
{
    return posArgs_;
}