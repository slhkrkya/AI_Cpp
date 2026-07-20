#pragma once

#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "commands/icommand.h"

namespace aicpp::commands {

class CommandRegistry {
public:
    void registerCommand(std::shared_ptr<ICommand> cmd);

    ICommand* find(const std::string& nameOrAlias) const;
    std::vector<ICommand*> all() const;

    // Best-effort "did you mean /x?" suggestion via edit distance; nullopt if
    // nothing is close enough to be worth suggesting.
    std::optional<std::string> suggest(const std::string& typo) const;

private:
    std::vector<std::shared_ptr<ICommand>> commands_;
    std::unordered_map<std::string, ICommand*> byName_;
};

}  // namespace aicpp::commands
