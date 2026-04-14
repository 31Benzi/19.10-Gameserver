#pragma once
#include <string>

namespace UI {
    void Create();
    void AddLog(const std::wstring& msg);
    void SetStatus(const wchar_t* status);
    void SetRestartCallback(void(*callback)());
    void TriggerRestart();
}
