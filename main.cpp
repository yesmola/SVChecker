#include <iostream>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>

#include "processor/common.h"
#include "processor/processor_manager.h"

int main(int argc, char* argv[]) {
#ifdef DEBUG
    spdlog::set_level(spdlog::level::debug);
#endif

    if (argc == 2) {
        svc::ProcessorManager::GetInstance()->Init();

        if (std::strcmp(argv[1], "1") == 0) {
            svc::ProcessorManager::GetInstance()->SetDataPath(svc::kDataSet5, svc::kDstDatasetPath, "5");
            svc::ProcessorManager::GetInstance()->PreCompile();
            svc::ProcessorManager::GetInstance()->Clear();
            svc::ProcessorManager::GetInstance()->SetDataPath(svc::kDataSetWild, svc::kDstDatasetPath, "4");
            svc::ProcessorManager::GetInstance()->PreCompile();
            svc::ProcessorManager::GetInstance()->Clear();
            svc::ProcessorManager::GetInstance()->SetDataPath(svc::kDataSet6, svc::kDstDatasetPath, "6");
            svc::ProcessorManager::GetInstance()->PreCompile();
            svc::ProcessorManager::GetInstance()->Clear();
        }
        if (std::strcmp(argv[1], "2") == 0) {
            // svc::ProcessorManager::GetInstance()->SetDataPath(svc::kDataSet5, svc::kDstDatasetPath, "5");
            // svc::ProcessorManager::GetInstance()->InjectBug(true);
            // svc::ProcessorManager::GetInstance()->Clear();
            // svc::ProcessorManager::GetInstance()->SetDataPath(svc::kDataSet4, svc::kDstDatasetPath, "4");
            // svc::ProcessorManager::GetInstance()->InjectBug(true);
            // svc::ProcessorManager::GetInstance()->Clear();
            svc::ProcessorManager::GetInstance()->SetDataPath(svc::kDataSet6, svc::kDstDatasetPath, "6");
            svc::ProcessorManager::GetInstance()->InjectBug(true);
            svc::ProcessorManager::GetInstance()->Clear();
            svc::ProcessorManager::GetInstance()->SetDataPath(svc::kDataSetWild, svc::kDstDatasetPath, "4");
            svc::ProcessorManager::GetInstance()->InjectBug(true);
            svc::ProcessorManager::GetInstance()->Clear();
        }
        if (std::strcmp(argv[1], "3") == 0) {
            svc::ProcessorManager::GetInstance()->SetDataPath(svc::kTmpDataset, svc::kDstDatasetPath, "5");
            svc::ProcessorManager::GetInstance()->InjectBug(true);
            svc::ProcessorManager::GetInstance()->Clear();
        }
    } else {
        spdlog::error("Usage: [app_name] 1/2/3");
    }

    // svc::ProcessorManager::GetInstance()->SetDataPath(svc::kTmpDataset, svc::kDstDatasetPath, "4");
    // // svc::ProcessorManager::GetInstance()->PreCompile();
    // svc::ProcessorManager::GetInstance()->InjectBug(true);
    return 0;
}