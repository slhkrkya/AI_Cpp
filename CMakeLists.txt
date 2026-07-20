cmake_minimum_required(VERSION 3.25)
project(aicpp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Threads REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(CURL REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(replxx CONFIG REQUIRED)
find_package(reproc++ CONFIG REQUIRED)
find_package(re2 CONFIG REQUIRED)

add_executable(aicpp
  src/main.cpp
  src/core/net/HttpClient.cpp
  src/core/agent/AgentSession.cpp
  src/providers/ollama/OllamaProvider.cpp
  src/cli/terminal.cpp
  src/cli/line_editor.cpp
  src/cli/dispatcher.cpp
  src/commands/command_registry.cpp
  src/commands/cmd_help.cpp
  src/commands/cmd_exit.cpp
  src/commands/cmd_clear.cpp
  src/core/tools/ToolRegistry.cpp
  src/core/tools/FileTracker.cpp
  src/core/tools/ReadFileTool.cpp
  src/core/tools/WriteFileTool.cpp
  src/core/tools/EditFileTool.cpp
  src/core/tools/GlobTool.cpp
  src/core/tools/GrepTool.cpp
  src/core/tools/BashTool.cpp
  src/core/permissions/DefaultPermissionManager.cpp
  src/providers/openai/OpenAIProvider.cpp
  src/core/config/ProviderFactory.cpp
  src/commands/cmd_model.cpp
  src/commands/cmd_tools.cpp
  src/core/config/Paths.cpp
  src/core/config/AppConfig.cpp
  src/core/llm/Serialization.cpp
  src/core/session/SessionStore.cpp
  src/commands/cmd_config.cpp
  src/commands/cmd_history.cpp
  src/commands/cmd_resume.cpp
  src/commands/cmd_plan.cpp
  src/core/workflow/WorkflowSpec.cpp
  src/core/workflow/WorkflowStore.cpp
  src/core/workflow/WorkflowEngine.cpp
  src/commands/cmd_workflow.cpp
)

target_include_directories(aicpp PRIVATE ${CMAKE_SOURCE_DIR}/src)

target_link_libraries(aicpp PRIVATE
  fmt::fmt
  CURL::libcurl
  nlohmann_json::nlohmann_json
  replxx::replxx
  reproc++
  re2::re2
)

if (MSVC)
  target_compile_options(aicpp PRIVATE /W4)
else()
  target_compile_options(aicpp PRIVATE -Wall -Wextra)
endif()
