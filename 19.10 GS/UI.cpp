#include "pch.h"
#include "UI.h"
#include "Options.h"
#include <commctrl.h>
#include <richedit.h>
#include <ctime>
#pragma comment(lib, "comctl32.lib")

#ifndef GET_X_LPARAM
#define GET_X_LPARAM(lp) ((int)(short)LOWORD(lp))
#define GET_Y_LPARAM(lp) ((int)(short)HIWORD(lp))
#endif

namespace UI {

static const COLORREF C_BG      = RGB(9,  13, 22);
static const COLORREF C_SURFACE = RGB(14, 20, 34);
static const COLORREF C_PANEL   = RGB(18, 26, 44);
static const COLORREF C_BORDER  = RGB(35, 50, 78);
static const COLORREF C_ACCENT  = RGB(77, 166, 255);
static const COLORREF C_GREEN   = RGB(46, 204, 113);
static const COLORREF C_TEXT    = RGB(220, 232, 248);
static const COLORREF C_SUBTEXT = RGB(107, 127, 163);
static const COLORREF C_TOGOFF  = RGB(30, 44, 70);
static const COLORREF C_INPUT   = RGB(22, 32, 54);

static const int WND_W    = 920;
static const int WND_H    = 700;
static const int TOP_H    = 70;
static const int DIV_X    = 460;
static const int CONFIG_H = 280;
static const int ACT_Y    = TOP_H + CONFIG_H;
static const int LOG_Y    = ACT_Y + 56;

#define WM_UI_LOG    (WM_USER + 1)
#define WM_UI_STATUS (WM_USER + 2)

static HWND g_hwnd   = nullptr;
static HWND g_hLog   = nullptr;
static HWND g_hIP    = nullptr;
static HWND g_hReg   = nullptr;
static HWND g_hEvt   = nullptr;
static HWND g_hCnt   = nullptr;

static bool g_launched     = false;
static bool g_btnHover     = false;
static bool g_restartHover = false;
static void(*g_restartCb)() = nullptr;
static std::wstring g_statusStr = L"\u25cf  STANDBY";
static RECT g_rcRestart;

static HFONT hFontTitle   = nullptr;
static HFONT hFontSection = nullptr;
static HFONT hFontLabel   = nullptr;
static HFONT hFontMono    = nullptr;

static HBRUSH hBrBg      = nullptr;
static HBRUSH hBrPanel   = nullptr;
static HBRUSH hBrSurface = nullptr;
static HBRUSH hBrInput   = nullptr;

static RECT g_rcLaunch;
static RECT g_rcIPBox, g_rcRegBox, g_rcCntBox, g_rcEvtBox;

static std::wstring GetSettingsPath() {
    wchar_t exe[MAX_PATH];
    GetModuleFileNameW(nullptr, exe, MAX_PATH);
    std::wstring ws(exe);
    auto p = ws.rfind(L'\\');
    if (p != std::wstring::npos) ws = ws.substr(0, p + 1);
    return ws + L"gameserver_settings.ini";
}

static void SaveSettings() {
    FILE* f = nullptr;
    _wfopen_s(&f, GetSettingsPath().c_str(), L"w");
    if (!f) return;
    auto bv = [](bool b){ return b ? "1" : "0"; };
    fprintf(f, "bDuos=%s\n",         bv(bDuos));
    fprintf(f, "bLateGame=%s\n",     bv(bLateGame));
    fprintf(f, "bTournament=%s\n",   bv(bTournament));
    fprintf(f, "bCreative=%s\n",     bv(bCreative));
    fprintf(f, "bGameSessions=%s\n", bv(bGameSessions));
    fprintf(f, "bDev=%s\n",         bv(bDev));
    fprintf(f, "IP=%s\n",            std::string(IP.begin(), IP.end()).c_str());
    fprintf(f, "Region=%s\n",        std::string(bRegion.begin(), bRegion.end()).c_str());
    fprintf(f, "EventId=%s\n",       std::string(eventId.begin(), eventId.end()).c_str());
    fprintf(f, "StartingCount=%d\n", StartingCount);
    fclose(f);
}

static void LoadSettings() {
    FILE* f = nullptr;
    _wfopen_s(&f, GetSettingsPath().c_str(), L"r");
    if (!f) return;
    char line[512];
    while (fgets(line, sizeof(line), f)) {
        std::string s(line);
        auto eq = s.find('=');
        if (eq == std::string::npos) continue;
        std::string key = s.substr(0, eq);
        std::string val = s.substr(eq + 1);
        while (!val.empty() && (val.back() == '\n' || val.back() == '\r')) val.pop_back();
        if      (key == "bDuos")         bDuos         = val == "1";
        else if (key == "bLateGame")     bLateGame     = val == "1";
        else if (key == "bTournament")   bTournament   = val == "1";
        else if (key == "bCreative")     bCreative     = val == "1";
        else if (key == "bGameSessions") bGameSessions = val == "1";
        else if (key == "bDev")          bDev          = val == "1";
        else if (key == "IP")            IP            = val;
        else if (key == "Region")        bRegion       = val;
        else if (key == "EventId")       eventId       = val;
        else if (key == "StartingCount") StartingCount = val.empty() ? 0 : std::stoi(val);
    }
    fclose(f);
}

struct Toggle {
    const wchar_t* label;
    bool*          value;
    RECT           rcTrack;
    bool           hov;
};
static std::vector<Toggle> g_toggles;

static void CreateRes() {
    hBrBg      = CreateSolidBrush(C_BG);
    hBrPanel   = CreateSolidBrush(C_PANEL);
    hBrSurface = CreateSolidBrush(C_SURFACE);
    hBrInput   = CreateSolidBrush(C_INPUT);
    hFontTitle   = CreateFontW(-32, 0, 0, 0, FW_LIGHT,    0, 0, 0, DEFAULT_CHARSET, 0, 0, CLEARTYPE_QUALITY, DEFAULT_PITCH, L"Segoe UI");
    hFontSection = CreateFontW(-11, 0, 0, 0, FW_SEMIBOLD,  0, 0, 0, DEFAULT_CHARSET, 0, 0, CLEARTYPE_QUALITY, DEFAULT_PITCH, L"Segoe UI");
    hFontLabel   = CreateFontW(-14, 0, 0, 0, FW_NORMAL,    0, 0, 0, DEFAULT_CHARSET, 0, 0, CLEARTYPE_QUALITY, DEFAULT_PITCH, L"Segoe UI");
    hFontMono    = CreateFontW(-12, 0, 0, 0, FW_NORMAL,    0, 0, 0, DEFAULT_CHARSET, 0, 0, CLEARTYPE_QUALITY, FIXED_PITCH,   L"Consolas");
}

static void DestroyRes() {
    DeleteObject(hBrBg);   DeleteObject(hBrPanel);
    DeleteObject(hBrSurface); DeleteObject(hBrInput);
    DeleteObject(hFontTitle); DeleteObject(hFontSection);
    DeleteObject(hFontLabel); DeleteObject(hFontMono);
}

static void FillRR(HDC hdc, RECT rc, int r, COLORREF clr) {
    HBRUSH br = CreateSolidBrush(clr);
    HPEN   pn = CreatePen(PS_SOLID, 0, clr);
    HBRUSH ob = (HBRUSH)SelectObject(hdc, br);
    HPEN   op = (HPEN)SelectObject(hdc, pn);
    RoundRect(hdc, rc.left, rc.top, rc.right, rc.bottom, r, r);
    SelectObject(hdc, ob); SelectObject(hdc, op);
    DeleteObject(br); DeleteObject(pn);
}

static void Txt(HDC hdc, const wchar_t* s, RECT rc, COLORREF clr, HFONT font,
                UINT fmt = DT_LEFT | DT_VCENTER | DT_SINGLELINE) {
    SetTextColor(hdc, clr);
    SetBkMode(hdc, TRANSPARENT);
    HFONT old = (HFONT)SelectObject(hdc, font);
    DrawTextW(hdc, s, -1, &rc, fmt);
    SelectObject(hdc, old);
}

static void HLine(HDC hdc, int x1, int x2, int y, COLORREF clr = 0) {
    HPEN pn = CreatePen(PS_SOLID, 1, clr ? clr : C_BORDER);
    HPEN op = (HPEN)SelectObject(hdc, pn);
    MoveToEx(hdc, x1, y, nullptr); LineTo(hdc, x2, y);
    SelectObject(hdc, op); DeleteObject(pn);
}

static void VLine(HDC hdc, int x, int y1, int y2) {
    HPEN pn = CreatePen(PS_SOLID, 1, C_BORDER);
    HPEN op = (HPEN)SelectObject(hdc, pn);
    MoveToEx(hdc, x, y1, nullptr); LineTo(hdc, x, y2);
    SelectObject(hdc, op); DeleteObject(pn);
}

static void SecHeader(HDC hdc, const wchar_t* title, int x, int y, int w) {
    RECT rc = { x, y, x + w, y + 18 };
    Txt(hdc, title, rc, C_ACCENT, hFontSection);
    RECT sz = rc;
    DrawTextW(hdc, title, -1, &sz, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_CALCRECT);
    HLine(hdc, sz.right + 10, x + w - 2, y + 9);
}

static void DrawTog(HDC hdc, const Toggle& t) {
    COLORREF bg = *t.value ? C_GREEN : (t.hov ? RGB(48, 68, 108) : C_TOGOFF);
    FillRR(hdc, t.rcTrack, 13, bg);
    int cx = *t.value ? t.rcTrack.right - 13 : t.rcTrack.left + 13;
    int cy = (t.rcTrack.top + t.rcTrack.bottom) / 2;
    HBRUSH br = CreateSolidBrush(RGB(245, 248, 255));
    HPEN   pn = CreatePen(PS_SOLID, 0, RGB(245, 248, 255));
    HBRUSH ob = (HBRUSH)SelectObject(hdc, br);
    HPEN   op = (HPEN)SelectObject(hdc, pn);
    Ellipse(hdc, cx - 9, cy - 9, cx + 9, cy + 9);
    SelectObject(hdc, ob); SelectObject(hdc, op);
    DeleteObject(br); DeleteObject(pn);
}

static void DrawInputBox(HDC hdc, RECT rc) {
    RECT outer = { rc.left - 1, rc.top - 1, rc.right + 1, rc.bottom + 1 };
    FillRR(hdc, outer, 6, C_BORDER);
    FillRR(hdc, rc, 5, C_INPUT);
}

static void AppendLog(const std::wstring& text) {
    if (!g_hLog) return;
    std::time_t now = std::time(nullptr);
    struct tm tm = {};
    localtime_s(&tm, &now);
    wchar_t ts[24];
    swprintf_s(ts, L"[%02d:%02d:%02d] ", tm.tm_hour, tm.tm_min, tm.tm_sec);
    std::wstring line = ts + text;
    if (line.back() != L'\n') line += L"\r\n";
    int len = GetWindowTextLengthW(g_hLog);
    SendMessageW(g_hLog, EM_SETSEL, len, len);
    SendMessageW(g_hLog, EM_REPLACESEL, FALSE, (LPARAM)line.c_str());
    SendMessageW(g_hLog, WM_VSCROLL, SB_BOTTOM, 0);
}

static HWND MakeEdit(HWND parent, const RECT& box, const wchar_t* def) {
    HWND h = CreateWindowExW(0, L"EDIT", def,
        WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | WS_TABSTOP,
        box.left + 8, box.top + 5,
        (box.right - box.left) - 16,
        (box.bottom - box.top) - 10,
        parent, nullptr, GetModuleHandleW(nullptr), nullptr);
    SendMessageW(h, WM_SETFONT, (WPARAM)hFontLabel, TRUE);
    return h;
}

static void BuildControls(HWND hwnd) {
    const int TX = 396, TW = 48, TH = 26;
    const int TY0 = TOP_H + 44, TS = 38;
    struct { const wchar_t* lbl; bool* val; } rows[] = {
        { L"Duos",          &bDuos         },
        { L"Late Game",     &bLateGame     },
        { L"Tournament",    &bTournament   },
        { L"Creative",      &bCreative     },
        { L"Game Sessions", &bGameSessions },
        { L"Dev Mode",      &bDev          },
    };
    g_toggles.clear();
    for (int i = 0; i < 6; ++i) {
        int y = TY0 + i * TS;
        Toggle t;
        t.label   = rows[i].lbl;
        t.value   = rows[i].val;
        t.rcTrack = { TX, y, TX + TW, y + TH };
        t.hov     = false;
        g_toggles.push_back(t);
    }

    g_rcLaunch  = { DIV_X + 18, ACT_Y + 12, WND_W - 18, ACT_Y + 46 };
    g_rcRestart = { 18,         ACT_Y + 12, DIV_X - 18, ACT_Y + 46 };

    g_rcIPBox  = { DIV_X + 18, TOP_H + 96,  WND_W - 18,   TOP_H + 132 };
    g_rcRegBox = { DIV_X + 18, TOP_H + 162, DIV_X + 208,  TOP_H + 196 };
    g_rcCntBox = { DIV_X + 216, TOP_H + 162, WND_W - 18,  TOP_H + 196 };
    g_rcEvtBox = { DIV_X + 18, TOP_H + 228, WND_W - 18,   TOP_H + 262 };

    std::wstring wsIP (IP.begin(),      IP.end());
    std::wstring wsReg(bRegion.begin(), bRegion.end());
    std::wstring wsEvt(eventId.begin(), eventId.end());
    g_hIP  = MakeEdit(hwnd, g_rcIPBox,  wsIP.c_str());
    g_hReg = MakeEdit(hwnd, g_rcRegBox, wsReg.c_str());
    wchar_t cntBuf[32];
    swprintf_s(cntBuf, L"%d", StartingCount);
    g_hCnt = MakeEdit(hwnd, g_rcCntBox, cntBuf);
    g_hEvt = MakeEdit(hwnd, g_rcEvtBox, wsEvt.c_str());

    LoadLibraryW(L"Msftedit.dll");
    g_hLog = CreateWindowExW(0, MSFTEDIT_CLASS, L"",
        WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_READONLY | ES_AUTOVSCROLL,
        10, LOG_Y + 44, WND_W - 20, WND_H - LOG_Y - 54,
        hwnd, nullptr, GetModuleHandleW(nullptr), nullptr);
    SendMessageW(g_hLog, WM_SETFONT, (WPARAM)hFontMono, TRUE);
    SendMessageW(g_hLog, EM_SETBKGNDCOLOR, 0, (LPARAM)C_SURFACE);
    CHARFORMAT2W cf = {};
    cf.cbSize      = sizeof(cf);
    cf.dwMask      = CFM_COLOR;
    cf.crTextColor = C_TEXT;
    SendMessageW(g_hLog, EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&cf);
}

static void PaintAll(HDC hdc) {
    RECT all = { 0, 0, WND_W, WND_H };
    FillRect(hdc, &all, hBrBg);

    RECT topRc = { 0, 0, WND_W, TOP_H };
    FillRect(hdc, &topRc, hBrSurface);
    HLine(hdc, 0, WND_W, TOP_H);

    Txt(hdc, L"19.10",      { 24, 6,  300, 44 }, C_TEXT,   hFontTitle);
    Txt(hdc, L"GAMESERVER", { 26, 46, 280, 64 }, C_ACCENT, hFontSection);

    COLORREF statClr = g_launched ? C_GREEN : RGB(255, 196, 0);
    Txt(hdc, g_statusStr.c_str(), { 580, 20, WND_W - 24, 50 }, statClr, hFontSection,
        DT_RIGHT | DT_VCENTER | DT_SINGLELINE);

    RECT lpRc = { 0, TOP_H, DIV_X, ACT_Y };
    FillRect(hdc, &lpRc, hBrPanel);

    VLine(hdc, DIV_X, TOP_H, ACT_Y);

    RECT actRc = { 0, ACT_Y, WND_W, LOG_Y };
    FillRect(hdc, &actRc, hBrSurface);
    HLine(hdc, 0, WND_W, ACT_Y);
    HLine(hdc, 0, WND_W, LOG_Y);

    RECT logBg = { 0, LOG_Y, WND_W, WND_H };
    FillRect(hdc, &logBg, hBrBg);

    SecHeader(hdc, L"GAME MODE",      24,         TOP_H + 14, DIV_X - 32);
    SecHeader(hdc, L"NETWORK CONFIG", DIV_X + 20, TOP_H + 14, WND_W - DIV_X - 38);
    SecHeader(hdc, L"SERVER LOG",     18,         LOG_Y + 14, WND_W - 36);

    for (auto& t : g_toggles) {
        int row = (int)(&t - g_toggles.data());
        int y   = TOP_H + 44 + row * 38;
        Txt(hdc, t.label, { 36, y + 4, 370, y + 30 }, C_TEXT, hFontLabel);
        DrawTog(hdc, t);
    }

    Txt(hdc, L"IP ADDRESS",  { DIV_X + 20, TOP_H + 76,  WND_W - 20, TOP_H + 96  }, C_SUBTEXT, hFontSection);
    Txt(hdc, L"REGION",      { DIV_X + 20, TOP_H + 142, DIV_X + 210, TOP_H + 162 }, C_SUBTEXT, hFontSection);
    Txt(hdc, L"START COUNT", { DIV_X + 218, TOP_H + 142, WND_W - 20, TOP_H + 162 }, C_SUBTEXT, hFontSection);
    Txt(hdc, L"EVENT ID",    { DIV_X + 20, TOP_H + 208, WND_W - 20, TOP_H + 228 }, C_SUBTEXT, hFontSection);

    DrawInputBox(hdc, g_rcIPBox);
    DrawInputBox(hdc, g_rcRegBox);
    DrawInputBox(hdc, g_rcCntBox);
    DrawInputBox(hdc, g_rcEvtBox);

    COLORREF btnClr = g_launched
        ? C_GREEN
        : (g_btnHover ? RGB(70, 148, 255) : RGB(46, 115, 210));
    FillRR(hdc, g_rcLaunch, 7, btnClr);
    const wchar_t* btnTxt = g_launched ? L"\u25cf  SERVER RUNNING" : L"LAUNCH SERVER";
    Txt(hdc, btnTxt, g_rcLaunch, RGB(255, 255, 255), hFontSection,
        DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    COLORREF rstClr = !g_launched
        ? RGB(30, 42, 66)
        : (g_restartHover ? RGB(220, 110, 30) : RGB(185, 82, 18));
    FillRR(hdc, g_rcRestart, 7, rstClr);
    COLORREF rstTxt = g_launched ? RGB(255, 255, 255) : C_SUBTEXT;
    Txt(hdc, L"\u21ba  RESTART SERVER", g_rcRestart, rstTxt, hFontSection,
        DT_CENTER | DT_VCENTER | DT_SINGLELINE);
}

static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
    case WM_CREATE:
        g_hwnd = hwnd;
        CreateRes();
        BuildControls(hwnd);
        g_launched  = true;
        g_statusStr = L"\u25cf  STARTING...";
        AppendLog(L"Auto-starting server with saved settings...");
        return 0;

    case WM_ERASEBKGND:
        return 1;

    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        HDC mem = CreateCompatibleDC(hdc);
        HBITMAP bm  = CreateCompatibleBitmap(hdc, WND_W, WND_H);
        HBITMAP obm = (HBITMAP)SelectObject(mem, bm);
        PaintAll(mem);
        BitBlt(hdc, 0, 0, WND_W, WND_H, mem, 0, 0, SRCCOPY);
        SelectObject(mem, obm);
        DeleteObject(bm);
        DeleteDC(mem);
        EndPaint(hwnd, &ps);
        return 0;
    }

    case WM_CTLCOLOREDIT:
    case WM_CTLCOLORSTATIC: {
        HDC hdc = (HDC)wp;
        SetTextColor(hdc, C_TEXT);
        SetBkColor(hdc, C_INPUT);
        return (LRESULT)hBrInput;
    }

    case WM_MOUSEMOVE: {
        int mx = GET_X_LPARAM(lp), my = GET_Y_LPARAM(lp);
        bool wb = g_btnHover;
        g_btnHover = !g_launched
            && mx >= g_rcLaunch.left && mx < g_rcLaunch.right
            && my >= g_rcLaunch.top  && my < g_rcLaunch.bottom;
        bool wr = g_restartHover;
        g_restartHover = g_launched
            && mx >= g_rcRestart.left && mx < g_rcRestart.right
            && my >= g_rcRestart.top  && my < g_rcRestart.bottom;
        bool changed = wb != g_btnHover || wr != g_restartHover;
        for (auto& t : g_toggles) {
            bool wh = t.hov;
            t.hov = mx >= t.rcTrack.left && mx < t.rcTrack.right
                 && my >= t.rcTrack.top  && my < t.rcTrack.bottom;
            if (wh != t.hov) changed = true;
        }
        if (changed) InvalidateRect(hwnd, nullptr, FALSE);
        return 0;
    }

    case WM_LBUTTONDOWN: {
        int mx = GET_X_LPARAM(lp), my = GET_Y_LPARAM(lp);
        for (auto& t : g_toggles) {
            if (mx >= t.rcTrack.left && mx < t.rcTrack.right
             && my >= t.rcTrack.top  && my < t.rcTrack.bottom) {
                *t.value = !*t.value;
                InvalidateRect(hwnd, nullptr, FALSE);
                wchar_t buf[80];
                swprintf_s(buf, L"Toggled  %s  \u2192  %s", t.label, *t.value ? L"ON" : L"OFF");
                AppendLog(buf);
                SaveSettings();
                return 0;
            }
        }
        if (!g_launched
         && mx >= g_rcLaunch.left && mx < g_rcLaunch.right
         && my >= g_rcLaunch.top  && my < g_rcLaunch.bottom) {
            wchar_t buf[512];
            GetWindowTextW(g_hIP,  buf, 512); IP      = std::string(buf, buf + wcslen(buf));
            GetWindowTextW(g_hReg, buf, 512); bRegion = std::string(buf, buf + wcslen(buf));
            GetWindowTextW(g_hEvt, buf, 512); eventId = std::string(buf, buf + wcslen(buf));
            GetWindowTextW(g_hCnt, buf, 512); StartingCount = _wtoi(buf);
            g_launched = true;
            g_statusStr = L"\u25cf  STARTING...";
            InvalidateRect(hwnd, nullptr, FALSE);
            SaveSettings();
            AppendLog(L"Options saved & locked.  Server initialising...");
            return 0;
        }
        if (g_launched
         && mx >= g_rcRestart.left && mx < g_rcRestart.right
         && my >= g_rcRestart.top  && my < g_rcRestart.bottom) {
            bool expected = false;
            if (!bServerRestarting.compare_exchange_strong(expected, true)) {
                AppendLog(L"Restart already in progress...");
                return 0;
            }
            TriggerRestart();
        }
        return 0;
    }

    case WM_SETCURSOR: {
        POINT pt; GetCursorPos(&pt); ScreenToClient(hwnd, &pt);
        bool hand = (!g_launched
            && pt.x >= g_rcLaunch.left && pt.x < g_rcLaunch.right
            && pt.y >= g_rcLaunch.top  && pt.y < g_rcLaunch.bottom)
            || (g_launched
            && pt.x >= g_rcRestart.left && pt.x < g_rcRestart.right
            && pt.y >= g_rcRestart.top  && pt.y < g_rcRestart.bottom);
        for (auto& t : g_toggles)
            if (pt.x >= t.rcTrack.left && pt.x < t.rcTrack.right
             && pt.y >= t.rcTrack.top  && pt.y < t.rcTrack.bottom) hand = true;
        SetCursor(LoadCursor(nullptr, hand ? IDC_HAND : IDC_ARROW));
        return 1;
    }

    case WM_UI_LOG: {
        auto* s = reinterpret_cast<std::wstring*>(lp);
        if (s) { AppendLog(*s); delete s; }
        return 0;
    }

    case WM_UI_STATUS: {
        auto* s = reinterpret_cast<std::wstring*>(lp);
        if (s) { g_statusStr = *s; delete s; InvalidateRect(hwnd, nullptr, FALSE); }
        return 0;
    }

    case WM_DESTROY:
        DestroyRes();
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(hwnd, msg, wp, lp);
}

class CoutBuf : public std::streambuf {
    std::string m_line;
protected:
    int overflow(int c) override {
        if (c != EOF) {
            if (c == '\n') {
                if (!m_line.empty()) {
                    UI::AddLog(std::wstring(m_line.begin(), m_line.end()));
                    m_line.clear();
                }
            } else {
                m_line += (char)c;
            }
        }
        return c;
    }
};
static CoutBuf        g_coutBuf;
static std::streambuf* g_oldCout = nullptr;

void AddLog(const std::wstring& msg) {
    if (g_hwnd) {
        auto* s = new std::wstring(msg);
        PostMessageW(g_hwnd, WM_UI_LOG, 0, (LPARAM)s);
    }
}

void SetStatus(const wchar_t* status) {
    if (g_hwnd) {
        auto* s = new std::wstring(status);
        PostMessageW(g_hwnd, WM_UI_STATUS, 0, (LPARAM)s);
    }
}

void SetRestartCallback(void(*cb)()) {
    g_restartCb = cb;
}

void TriggerRestart() {
    AddLog(L"Restarting server...");
    SetStatus(L"\u25cf  RESTARTING...");
    if (g_restartCb)
        std::thread(g_restartCb).detach();
}

void Create() {
    LoadSettings();
    g_oldCout = std::cout.rdbuf(&g_coutBuf);

    std::thread([]() {
        WNDCLASSEXW wc   = {};
        wc.cbSize        = sizeof(wc);
        wc.lpfnWndProc   = WndProc;
        wc.hInstance     = GetModuleHandleW(nullptr);
        wc.lpszClassName = L"GS1910_UI";
        RegisterClassExW(&wc);

        RECT wr = { 0, 0, WND_W, WND_H };
        AdjustWindowRect(&wr, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, FALSE);
        int ww = wr.right  - wr.left;
        int wh = wr.bottom - wr.top;
        int cx = (GetSystemMetrics(SM_CXSCREEN) - ww) / 2;
        int cy = (GetSystemMetrics(SM_CYSCREEN) - wh) / 2;

        HWND hwnd = CreateWindowExW(0, L"GS1910_UI", L"19.10 Gameserver",
            WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
            cx, cy, ww, wh,
            nullptr, nullptr, GetModuleHandleW(nullptr), nullptr);

        ShowWindow(hwnd, SW_SHOW);
        UpdateWindow(hwnd);

        MSG m;
        while (GetMessageW(&m, nullptr, 0, 0)) {
            if (!IsDialogMessageW(hwnd, &m)) {
                TranslateMessage(&m);
                DispatchMessageW(&m);
            }
        }
        if (g_oldCout) std::cout.rdbuf(g_oldCout);
    }).detach();

    while (!g_launched) Sleep(50);
}

}
