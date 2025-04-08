add_library(core
    ProcessManager.cpp
   # DLLAnalyzer.cpp
   # Logger.cpp
)

target_include_directories(core PUBLIC ${PROJECT_SOURCE_DIR}/include)
