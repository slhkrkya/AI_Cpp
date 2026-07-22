#include "i18n/catalogs.h"

namespace aicpp::i18n {

const std::unordered_map<std::string, std::string>& catalog_en() {
    static const std::unordered_map<std::string, std::string> table = {
        // common / shared role labels
        {"role.user", "You"},
        {"role.assistant", "Assistant"},
        {"role.tool", "Tool"},
        {"role.system", "System"},
        {"common.untitled", "(untitled)"},

        // main.cpp
        {"app.banner", "aicpp v0.7 - type /help to see commands. Active model: {}"},
        {"app.startup_provider_failed", "Failed to set up the startup provider: {}"},
        {"app.bye", "Bye."},
        {"app.cancelled", "Cancelled."},
        {"app.turn_failed", "This turn ended with an unexpected error, the session stayed open: {}"},
        {"app.cwd_line", "Working directory: {}"},
        {"app.invalid_cwd", "Invalid directory: {}"},

        // cli/dispatcher.cpp
        {"dispatcher.unknown_command", "Unknown command: /{}"},
        {"dispatcher.suggestion", "Did you mean: /{}?"},
        {"dispatcher.help_hint", "Type /help to see the command list."},

        // core/permissions/DefaultPermissionManager.cpp
        {"permission.wants_to", "'{}' wants to do the following:"},
        {"permission.ask_yna", "Allow? (y)es / (n)o / (a)lways this session: "},

        // tool-call banners (cli/stream_printer.h via cli/theme.cpp)
        {"tool.start", "running tool: {}"},
        {"tool.end", "tool finished: {}"},
        {"tool.end_error", "tool failed: {}"},

        // command descriptions/usages
        {"cmd.help.desc", "Lists the available commands"},
        {"cmd.exit.desc", "Exits the application"},
        {"cmd.clear.desc", "Clears the screen and chat history"},
        {"cmd.model.desc", "Shows or switches the active LLM provider/model"},
        {"cmd.model.usage", "/model [provider:model]"},
        {"cmd.tools.desc", "Lists the available tools and their risk levels"},
        {"cmd.config.desc", "Shows the config file's path and contents"},
        {"cmd.history.desc", "Lists the current and past sessions"},
        {"cmd.resume.desc", "Resumes a past session where it left off"},
        {"cmd.resume.usage", "/resume [number-or-id]"},
        {"cmd.plan.desc", "Toggles plan mode (read-only tools only); apply it with /plan approve"},
        {"cmd.plan.usage", "/plan [text|approve|cancel]"},
        {"cmd.workflow.desc", "Runs a saved or free-text workflow with parallel sub-agents"},
        {"cmd.workflow.usage", "/workflow [list | <name> <input> | <free-text description>]"},
        {"cmd.language.desc", "Shows or switches the interface language"},
        {"cmd.language.usage", "/language [tr|en]"},

        // commands/cmd_help.cpp
        {"help.banner", "aicpp - terminal AI coding assistant"},
        {"help.commands_header", "Commands:"},
        {"help.cloud_intro", "If you want to use a cloud provider:"},
        {"help.cloud_usage",
         "Type /model <provider>:<model> (e.g. openai:gpt-4o-mini, groq:llama-3.3-70b-versatile)."},
        {"help.cloud_autoconfig",
         "If the provider isn't configured yet (missing base URL / API key), the app will ask\n"
         "you and save it to config.json - no restart needed. This works for any provider that\n"
         "speaks OpenAI's chat-completions API (OpenAI, Groq, Mistral, DeepSeek, Together,\n"
         "Fireworks, xAI/Grok, Azure OpenAI, Gemini's OpenAI-compatible endpoint, etc.)."},
        {"help.language_hint", "Use /language <tr|en> to switch the interface language."},
        {"help.footer",
         "Use /config for details, /history for past sessions, /plan for a read-only review."},

        // commands/cmd_clear.cpp
        {"clear.done", "Chat history cleared."},

        // commands/cmd_model.cpp
        {"model.setup.explain",
         "Do you want to set up the '{}' provider now? What you enter is saved to\n"
         "~/.aicpp/config.json, no restart needed. This works for ANY provider that speaks\n"
         "OpenAI's chat-completions API (OpenAI, Groq, Mistral, DeepSeek, Together,\n"
         "Fireworks, xAI/Grok, Azure OpenAI, Gemini's OpenAI-compatible endpoint, a local\n"
         "llama.cpp/LM Studio server, etc.)."},
        {"model.setup.ask_base_url", "Base URL (e.g. https://api.groq.com/openai/v1): "},
        {"model.setup.url_empty", "URL empty, aborted."},
        {"model.setup.ask_api_key", "Paste the API key: "},
        {"model.setup.key_empty", "Key empty, aborted."},
        {"model.setup.saved_plaintext_warning",
         "The key was saved to config.json in plain text - don't share this file."},
        {"model.current", "Current model: {}"},
        {"model.configured_providers", "Configured providers: {}"},
        {"model.switch_hint", "To switch: /model <provider>:<model>  (e.g. /model openai:gpt-4o-mini)"},
        {"model.other_provider_hint",
         "You can also type a different provider (e.g. /model groq:llama-3.3-70b-versatile) - "
         "if it isn't set up yet, you'll be asked and it'll be saved."},
        {"model.change_failed", "Failed to switch model: {}"},
        {"model.changed", "Model switched: {}"},

        // commands/cmd_tools.cpp
        {"tools.header", "Available tools:"},
        {"tools.risk.read", "Read (auto-allowed)"},
        {"tools.risk.write", "Write (needs permission)"},
        {"tools.risk.execute", "Execute (needs permission)"},
        {"tools.risk.network", "Network (needs permission)"},

        // commands/cmd_config.cpp
        {"config.path", "Config file: {}"},
        {"config.read_failed", "(couldn't read the file)"},
        {"config.invalid_json", "(file isn't valid JSON)"},
        {"config.redacted_marker", "***hidden*** (redacted for display, still plain text in the file)"},
        {"config.footer", "To change it, edit the file in an editor and restart the app."},

        // commands/cmd_history.cpp
        {"history.current", "Current session: {} ({} messages, model: {})"},
        {"history.no_other", "No other saved sessions."},
        {"history.recent_header", "Recent sessions:"},
        {"history.current_marker", "  <- this session"},
        {"history.resume_hint", "You can continue a session with /resume <id>."},

        // commands/cmd_resume.cpp
        {"resume.none_saved", "No saved sessions."},
        {"resume.list_header", "Past sessions:"},
        {"resume.hint", "To continue: /resume <number-or-id>"},
        {"resume.not_found", "Session not found: {}"},
        {"resume.provider_warning", "Warning: saved model '{}' couldn't be loaded ({}), keeping the current model."},
        {"resume.loaded", "Session loaded: {} - {} ({} messages, model: {})"},
        {"resume.recent_messages_header", "Recent messages:"},
        {"resume.grants_restored", "Previously-approved tools restored for this session: {}"},

        // commands/cmd_plan.cpp
        {"plan.not_active", "You're not in plan mode right now."},
        {"plan.approved", "Plan approved, moving to implementation..."},
        {"plan.cancelled", "Plan cancelled (it stays in the transcript but won't be applied)."},
        {"plan.closed", "Plan mode closed."},
        {"plan.enter_banner",
         "PLAN MODE - only read-only tools will run.\n  To approve: /plan approve   To cancel: /plan cancel"},

        // commands/cmd_workflow.cpp
        {"workflow.none_saved", "No saved workflows."},
        {"workflow.saved_header", "Saved workflows:"},
        {"workflow.usage_hint", "Usage: /workflow <name> <input>   or   /workflow <free-text description>"},
        {"workflow.meta.no_json",
         "The model couldn't produce a valid JSON spec. Small/local models can be unreliable at "
         "this step - try again with a stronger /model (e.g. openai:...)."},
        {"workflow.meta.parse_failed", "Couldn't parse the model output as JSON: {}"},
        {"workflow.meta.too_many_agents", "The generated spec has more than 5 sub-agents, rejected for safety."},
        {"workflow.decomposing",
         "converting the free-text description into a JSON spec (this is a meta-agent call)..."},
        {"workflow.run_failed", "Couldn't run the workflow: {}"},
        {"workflow.synthesizing", "all stages complete, synthesizing results..."},

        // commands/cmd_language.cpp
        {"language.current", "Current language: {} ({})"},
        {"language.available", "Available languages: tr (Turkish), en (English)"},
        {"language.usage_hint", "To switch: /language <tr|en>"},
        {"language.invalid", "Invalid language code: '{}'. Available: tr, en"},
        {"language.switched", "Language switched: {} ({})"},

        // core/tools/LineDiff.cpp
        {"diff.unchanged_lines", "... {} unchanged lines ..."},
        {"diff.too_large", "Diff not shown (file too large or too different)."},
    };
    return table;
}

}  // namespace aicpp::i18n
