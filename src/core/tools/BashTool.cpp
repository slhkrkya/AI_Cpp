#include "core/tools/BashTool.h"

#include <tuple>
#include <vector>

#include <fmt/format.h>
#include <reproc++/drain.hpp>
#include <reproc++/reproc.hpp>

namespace aicpp::tools {

nlohmann::json BashTool::parametersSchema() const {
    return {
        {"type", "object"},
        {"properties",
         {
             {"command", {{"type", "string"}, {"description", "Calistirilacak kabuk komutu"}}},
             {"timeout_ms", {{"type", "integer"}, {"description", "Zaman asimi (ms), varsayilan 120000"}}},
         }},
        {"required", nlohmann::json::array({"command"})},
    };
}

std::string BashTool::summarize(const nlohmann::json& args) const {
    return fmt::format("Run: {}", args.value("command", std::string()));
}

ToolExecResult BashTool::execute(const nlohmann::json& args, ToolExecutionContext& ctx) {
    const std::string command = args.at("command").get<std::string>();
    const int timeoutMs = args.value("timeout_ms", 120000);

#ifdef _WIN32
    std::vector<std::string> argv = {"powershell.exe", "-NoProfile", "-NonInteractive", "-Command", command};
#else
    std::vector<std::string> argv = {"/bin/bash", "-lc", command};
#endif

    std::string workingDir = ctx.working_directory.string();

    reproc::options options;
    options.redirect.out.type = reproc::redirect::pipe;
    options.redirect.err.type = reproc::redirect::pipe;
    options.working_directory = workingDir.c_str();
    options.deadline = reproc::milliseconds(timeoutMs);
    options.stop = reproc::stop_actions{
        {reproc::stop::terminate, reproc::milliseconds(5000)},
        {reproc::stop::kill, reproc::milliseconds(2000)},
        {reproc::stop::noop, reproc::milliseconds(0)},
    };

    reproc::process process;
    std::error_code ec = process.start(argv, options);
    if (ec) {
        return {false, fmt::format("Komut baslatilamadi: {}", ec.message()), true};
    }

    std::string out, err;
    ec = reproc::drain(process, reproc::sink::string(out), reproc::sink::string(err));
    bool timedOut = ec == std::errc::timed_out;

    int status = 0;
    std::error_code stopEc;
    std::tie(status, stopEc) = process.stop(options.stop);

    std::string combined = out;
    if (!err.empty()) {
        combined += "\n[stderr]\n";
        combined += err;
    }

    constexpr size_t kMaxChars = 20000;
    if (combined.size() > kMaxChars) {
        combined.resize(kMaxChars);
        combined += "\n[... cikti kirpildi ...]";
    }

    combined += fmt::format("\n[exit code: {}]", status);
    if (timedOut) {
        combined += "\n[komut zaman asimina ugradi ve sonlandirildi]";
        return {false, combined, true};
    }

    return {true, combined, status != 0};
}

}  // namespace aicpp::tools
