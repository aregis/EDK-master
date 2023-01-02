/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#include <huestream_example_gui_win/stdafx.h>
#include <huestream_example_gui_win/StreamTest.h>
#include <huestream_example_gui_win/StreamTestDlg.h>
#include <support/logging/Log.h>
#include <huestream/config/Config.h>
#include <huestream/HueStream.h>
#include <huestream/HueEDK.h>
#include <huestream/effect/animation/animations/CurveAnimation.h>
#include <huestream/effect/animation/animations/SequenceAnimation.h>
#include <huestream/effect/animation/animations/TweenAnimation.h>
#include <huestream/common/data/Area.h>
#include <huestream/common/storage/FileStorageAccessor.h>
#include <huestream/effect/lightscript/LightScript.h>
#include <huestream/effect/lightscript/Timeline.h>
#include <huestream/stream/UdpConnector.h>
#include <huestream/stream/DtlsConnector.h>
#include <support/network/NetworkConfiguration.h>

#include <math.h>

#include <iomanip>
#include <sstream>
#include <string>
#include <memory>
#include <vector>

#include "afxdialogex.h"  // NOLINT

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define ID_TIMER WM_USER + 200

constexpr static auto PERSISTENCE_ENCRYPTION_KEY = "encryption_key";

class CAboutDlg : public CDialogEx {
 public:
    CAboutDlg();

#ifdef AFX_DESIGN_TIME
    enum { IDD = IDD_ABOUTBOX };
#endif

 protected:
    void DoDataExchange(CDataExchange *pDX) override;

    BOOL OnInitDialog() override;

    afx_msg void OnBnClickedNewbridge();
    afx_msg void OnComboChangedBridge();

    afx_msg void OnBnClickedButtonUDP();
    afx_msg void OnBnClickedButtonDTLS();

    afx_msg void OnBnClickedButtonON();
    afx_msg void OnBnClickedButtonOFF();

    DECLARE_MESSAGE_MAP()
    CStreamTestDlg* m_mainDialog;
    BridgeListPtr m_bridges;

 public:
    void SetMainDialog(CStreamTestDlg* dialog) {
        m_mainDialog = dialog;
    }
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX) {
}

void CAboutDlg::DoDataExchange(CDataExchange *pDX) {
    CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx
)
ON_BN_CLICKED(IDC_UDP, &CAboutDlg::OnBnClickedButtonUDP)
ON_BN_CLICKED(IDC_DTLS, &CAboutDlg::OnBnClickedButtonDTLS)
ON_BN_CLICKED(IDC_ON, &CAboutDlg::OnBnClickedButtonON)
ON_BN_CLICKED(IDC_OFF, &CAboutDlg::OnBnClickedButtonOFF)
ON_BN_CLICKED(IDC_NEWBRIDGE, &CAboutDlg::OnBnClickedNewbridge)
ON_CBN_SELENDOK(IDC_COMBO_BRIDGES, &CAboutDlg::OnComboChangedBridge)
END_MESSAGE_MAP()

BOOL CAboutDlg::OnInitDialog() {
    CDialogEx::OnInitDialog();

    auto huestream = m_mainDialog->GetHueStream();

    CWnd *buildfield = GetDlgItem(IDC_BUILD);
    auto buildString = "Build: " + std::string(BUILD_INFO);
    buildfield->SetWindowText(buildString.c_str());

    CWnd *framecounterField = GetDlgItem(IDC_FRAMECOUNTER);
    auto framesString =
        "Frames sent this session: " + std::to_string(huestream->GetStreamCounter());
    framecounterField->SetWindowText(framesString.c_str());

    CWnd *connectResultField = GetDlgItem(IDC_CONNECTRESULT);
    auto result = huestream->GetConnectionResult();
    std::string resultString = "Connection result: ";
    switch (result) {
        case huestream::Uninitialized:
            resultString += "Uninitialized";
            break;
        case huestream::Busy:
            resultString += "Busy";
            break;
        case huestream::NoBridgeFound:
            resultString += "NoBridgeFound";
            break;
        case huestream::BridgeFound:
            resultString += "BridgeFound";
            break;
        case huestream::ActionRequired:
            resultString += "ActionRequired";
            break;
        case huestream::ReadyToStart:
            resultString += "ReadyToStart";
            break;
        case huestream::Streaming:
            resultString += "Streaming";
            break;
        default:
            break;
    }
    connectResultField->SetWindowText(resultString.c_str());

    m_bridges = huestream->GetKnownBridges();
    auto combo_bridges = (CComboBox*)GetDlgItem(IDC_COMBO_BRIDGES);
    combo_bridges->ResetContent();
    for (auto b = m_bridges->begin(); b != m_bridges->end(); ++b) {
        auto idx = combo_bridges->AddString((*b)->GetName().c_str());
        if ((*b)->GetId() == huestream->GetLoadedBridge()->GetId()) {
            combo_bridges->SetCurSel(idx);
        }
    }

    return TRUE;
}

void CAboutDlg::OnBnClickedButtonUDP() {
    m_mainDialog->ChangeMode(true);
}

void CAboutDlg::OnBnClickedButtonDTLS() {
    m_mainDialog->ChangeMode(false);
}

void CAboutDlg::OnBnClickedButtonON() {
    m_mainDialog->GetHueStream()->SetGroupOn(true);
}

void CAboutDlg::OnBnClickedButtonOFF() {
    m_mainDialog->GetHueStream()->SetGroupOn(false);
}

void CAboutDlg::OnBnClickedNewbridge() {
    m_mainDialog->GetHueStream()->ConnectNewBridgeAsync();
}

void CAboutDlg::OnComboChangedBridge() {
    auto huestream = m_mainDialog->GetHueStream();
    auto combo_bridges = (CComboBox*)GetDlgItem(IDC_COMBO_BRIDGES);
    auto sel = combo_bridges->GetCurSel();
    if (sel >= 0 && sel < m_bridges->size()) {
        auto newBridge = m_bridges->at(sel);
        if (newBridge->GetId() != huestream->GetLoadedBridge()->GetId()) {
            huestream->ConnectManualBridgeInfoAsync(newBridge);
        }
    }
}

CStreamTestDlg::CStreamTestDlg(CWnd *pParent /*=NULL*/)
    : CDialogEx(IDD_STREAMTEST_DIALOG, pParent) {
    m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CStreamTestDlg::DoDataExchange(CDataExchange *pDX) {
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_EDIT_IP, m_edit_ip);
    DDX_Control(pDX, IDC_EDIT_UNAME, m_edit_username);
    DDX_Control(pDX, IDC_EDIT_KEY, m_edit_key);
    DDX_Control(pDX, IDC_EDIT_LOG, m_edit_log);
    DDX_Control(pDX, IDC_COMBO_GROUP, m_combo_group);
    DDX_Control(pDX, IDC_COMBO_LANG, m_combo_lang);
    DDX_Control(pDX, IDC_SLIDER_SPEED, m_slider_speed);
    DDX_Control(pDX, IDC_SLIDER_TIME, m_slider_time);
}

BEGIN_MESSAGE_MAP(CStreamTestDlg, CDialogEx
)

ON_WM_SYSCOMMAND()

ON_WM_PAINT()

ON_WM_QUERYDRAGICON()

ON_MESSAGE(UWM_HUESTREAM_FEEDBACK, OnHuestreamFeedback)
ON_MESSAGE(UWM_POST_DEBUGMSG, OnPostDebugMsg)
ON_MESSAGE(UWM_TIMELINE_EVENT, OnTimelineEvent)

ON_BN_CLICKED(IDC_BUTTON_CONNECT, &CStreamTestDlg::OnBnClickedButtonConnect)
ON_BN_CLICKED(IDC_BUTTON_MANUAL, &CStreamTestDlg::OnBnClickedButtonManual)
ON_BN_CLICKED(IDC_BUTTON_RESET, &CStreamTestDlg::OnBnClickedButtonReset)
ON_BN_CLICKED(IDC_EXPLOSION, &CStreamTestDlg::OnBnClickedExplosion)
ON_BN_CLICKED(IDC_REDWHITEBLUE, &CStreamTestDlg::OnBnClickedRedWhiteBlue)
ON_BN_CLICKED(IDC_COMBI, &CStreamTestDlg::OnBnClickedCombi)
ON_BN_CLICKED(IDC_SEQUENCE_EFFECT, &CStreamTestDlg::OnBnClickedButtonSequenceRound)
ON_BN_CLICKED(IDC_SEQUENCE_LTOR, &CStreamTestDlg::OnBnClickedSeqeffectltor)
ON_BN_CLICKED(IDC_SEQUENCE_ALL, &CStreamTestDlg::OnBnClickedSequencerandom)
ON_CBN_SELENDOK(IDC_COMBO_GROUP, &CStreamTestDlg::OnComboChangedGroup)
ON_CBN_SELENDOK(IDC_COMBO_LANG, &CStreamTestDlg::OnComboChangedLanguage)
ON_BN_CLICKED(IDC_SAW, &CStreamTestDlg::OnBnClickedSaw)
ON_BN_CLICKED(IDC_SQUARE, &CStreamTestDlg::OnBnClickedSquare)
ON_BN_CLICKED(IDC_SINE, &CStreamTestDlg::OnBnClickedSine)
ON_BN_CLICKED(IDC_MARQUEE, &CStreamTestDlg::OnBnClickedMarquee)
ON_BN_CLICKED(IDC_COLORLOOP, &CStreamTestDlg::OnBnClickedColorloop)
ON_BN_CLICKED(IDC_FIREBALLS, &CStreamTestDlg::OnBnClickedFireballs)
ON_BN_CLICKED(IDC_POLICE, &CStreamTestDlg::OnBnClickedPolice)
ON_BN_CLICKED(IDC_LOAD, &CStreamTestDlg::OnBnClickedLoad)
ON_BN_CLICKED(IDC_SAVE, &CStreamTestDlg::OnBnClickedSave)
ON_BN_CLICKED(IDC_PLAY, &CStreamTestDlg::OnBnClickedPlay)
ON_BN_CLICKED(IDC_STOP, &CStreamTestDlg::OnBnClickedStop)
ON_EN_CHANGE(IDC_EDIT_ALPHA1, &CStreamTestDlg::OnChangeAlphaSaw)
ON_EN_CHANGE(IDC_EDIT_ALPHA2, &CStreamTestDlg::OnChangeAlphaSquare)
ON_EN_CHANGE(IDC_EDIT_ALPHA3, &CStreamTestDlg::OnChangeAlphaSine)
ON_EN_CHANGE(IDC_EDIT_ALPHA4, &CStreamTestDlg::OnChangeAlphaColorloop)
ON_EN_CHANGE(IDC_EDIT_ALPHA5, &CStreamTestDlg::OnChangeAlphaMarquee)
ON_EN_CHANGE(IDC_EDIT_ALPHA6, &CStreamTestDlg::OnChangeAlphaFireballs)
ON_EN_CHANGE(IDC_EDIT_ALPHA7, &CStreamTestDlg::OnChangeAlphaPolice)

ON_WM_HSCROLL()

ON_WM_TIMER()

END_MESSAGE_MAP()


BOOL CStreamTestDlg::OnInitDialog() {
    CDialogEx::OnInitDialog();

    // Add "About..." menu item to system menu.

    // IDM_ABOUTBOX must be in the system command range.
    ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
    ASSERT(IDM_ABOUTBOX < 0xF000);

    CMenu *pSysMenu = GetSystemMenu(FALSE);
    if (pSysMenu != NULL) {
        BOOL bNameValid;
        CString strAboutMenu;
        bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
        ASSERT(bNameValid);
        if (!strAboutMenu.IsEmpty()) {
            pSysMenu->AppendMenu(MF_SEPARATOR);
            pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
        }
    }

    // Set the icon for this dialog.  The framework does this automatically
    //  when the application's main window is not a dialog
    SetIcon(m_hIcon, TRUE);            // Set big icon
    SetIcon(m_hIcon, FALSE);        // Set small icon

    ShowWindow(SW_SHOWNORMAL);

    bool useUDP = IsUDPForcedByCommandLine();
    InitializeHueStream(useUDP);
    InitializeEffectPlayerAndTimeline();
    InitializeUserInterface();

    if (!useUDP) {
        m_huestream->ConnectBridgeBackgroundAsync();
    }

    return TRUE;
}

void CStreamTestDlg::OnSysCommand(UINT nID, LPARAM lParam) {
    if ((nID & 0xFFF0) == IDM_ABOUTBOX) {
        CAboutDlg dlgAbout;
        dlgAbout.SetMainDialog(this);
        dlgAbout.DoModal();
    } else {
        CDialogEx::OnSysCommand(nID, lParam);
    }
}

void CStreamTestDlg::OnPaint() {
    if (IsIconic()) {
        CPaintDC dc(this);

        SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

        // Center icon in client rectangle
        int cxIcon = GetSystemMetrics(SM_CXICON);
        int cyIcon = GetSystemMetrics(SM_CYICON);
        CRect rect;
        GetClientRect(&rect);
        int x = (rect.Width() - cxIcon + 1) / 2;
        int y = (rect.Height() - cyIcon + 1) / 2;

        dc.DrawIcon(x, y, m_hIcon);
    } else {
        CDialogEx::OnPaint();
    }
}

HCURSOR CStreamTestDlg::OnQueryDragIcon() {
    return static_cast<HCURSOR>(m_hIcon);
}

void CStreamTestDlg::InitializeUserInterface() {
    m_pFont = new CFont();
    m_pFont->CreatePointFont(62, _T("Tahoma"));
    m_edit_log.SetFont(m_pFont, FALSE);

    m_slider_speed.SetRange(-30, 30, true);
    m_slider_speed.SetPos(0);
    m_slider_time.SetRange(0, 0, true);
    m_slider_time.SetPos(0);

    GetDlgItem(IDC_EDIT_ALPHA1)->SetWindowText("0");
    GetDlgItem(IDC_EDIT_ALPHA2)->SetWindowText("0");
    GetDlgItem(IDC_EDIT_ALPHA3)->SetWindowText("0");
    GetDlgItem(IDC_EDIT_ALPHA4)->SetWindowText("0");
    GetDlgItem(IDC_EDIT_ALPHA5)->SetWindowText("0");
    GetDlgItem(IDC_EDIT_ALPHA6)->SetWindowText("0");
    GetDlgItem(IDC_EDIT_ALPHA7)->SetWindowText("0");

    m_combo_lang.AddString("en");
    m_combo_lang.AddString("nl");
    m_combo_lang.AddString("de");
    m_combo_lang.SetCurSel(m_combo_lang.FindStringExact(0, "en"));
}

bool CStreamTestDlg::IsUDPForcedByCommandLine() {
    std::string command(GetCommandLine());
    auto args = split(command, ' ');
    return (args.size() == 2 && args.at(1) == "udp");
}

const std::string CStreamTestDlg::GetDeviceName() const {
    TCHAR computerName[MAX_COMPUTERNAME_LENGTH + 1];
    DWORD size = sizeof(computerName) / sizeof(computerName[0]);
    GetComputerName(computerName, &size);
    return std::string(computerName);
}

void CStreamTestDlg::InitializeHueStream(bool useUDP, const std::string language) {
    const std::string applicationName = "HueStreamGUI Windows";
    auto config = std::make_shared<huestream::Config>(applicationName, GetDeviceName(), huestream::PersistenceEncryptionKey(PERSISTENCE_ENCRYPTION_KEY), language);
    if (useUDP) {
        AddDebugMsg("Initialized with UDP");
        config->SetStreamingMode(huestream::STREAMING_MODE_UDP);
		support::NetworkConfiguration::set_use_http2(false);
    } else {
        AddDebugMsg("Initialized with default configuration");
        config->SetStreamingMode(huestream::STREAMING_MODE_DTLS);
		support::NetworkConfiguration::set_use_http2(true);
    }

    //Choose whether to start streaming immediately at bridge connection (default true)
    //config->GetAppSettings()->SetAutoStartAtConnection(false);

    //Choose whether lights retain their color after effects are finished/disabled (default false)
    //config->GetAppSettings()->SetLightsRetainColor(true);

    //  Create HueStream instance ...
    m_huestream = std::make_shared<huestream::HueStream>(config);

    //  Register feedback callback
    m_huestream->RegisterFeedbackCallback([this](const huestream::FeedbackMessage &message) {
        auto feedbackMessage = new huestream::FeedbackMessage(message);
        PostMessage(UWM_HUESTREAM_FEEDBACK, 0, reinterpret_cast< LPARAM >(feedbackMessage));
    });
}

void CStreamTestDlg::InitializeEffectPlayerAndTimeline()
{
    m_effectPlayer = std::make_shared<EffectPlayer>(m_huestream);
    m_timeline = std::make_shared<huestream::Timeline>(m_huestream->GetTimeManager());
    m_timeline->SetLength(0);
    m_timeline->RegisterTimelineStateChangedCallback([this](huestream::TimelineState state, huestream::ITimeline* timeline) {
        auto event = new huestream::TimelineState(state);
        PostMessage(UWM_TIMELINE_EVENT, 0, reinterpret_cast< LPARAM >(event));
    });
    m_lightscript = std::make_shared<huestream::LightScript>();
}

LRESULT CStreamTestDlg::OnHuestreamFeedback(WPARAM wParam, LPARAM lParam)
{
    auto message = reinterpret_cast< huestream::FeedbackMessage * >(lParam);

    AddDebugMsg(message->GetDebugMessage());
    if (message->GetMessageType() == huestream::FeedbackMessage::FEEDBACK_TYPE_USER) {
        SetUserMessage(message->GetUserMessage());
    }

    if (message->GetId() == huestream::FeedbackMessage::ID_DONE_BRIDGE_FOUND) {
        SetUserMessage(
            "It looks like you've got a Hue bridge and guess what: this app works with Hue! Press connect if you want to connect.");
    }

    if (message->GetId() == huestream::FeedbackMessage::ID_DONE_COMPLETED) {
        SetUserMessage("Connected to Hue bridge!");
    }

    if (message->GetId() == huestream::FeedbackMessage::ID_BRIDGE_CHANGED) {
        SetIpAndCredentials(
            message->GetBridge()->GetIpAddress().c_str(),
            message->GetBridge()->GetUser().c_str(),
            message->GetBridge()->GetClientKey().c_str()
        );
    }

    if (message->GetId() == huestream::FeedbackMessage::ID_USERPROCEDURE_STARTED ||
        message->GetId() == huestream::FeedbackMessage::ID_USERPROCEDURE_FINISHED ||
        message->GetId() == huestream::FeedbackMessage::ID_STREAMING_DISCONNECTED) {
        SetConnectButtonText();
    }

    if (message->GetId() == huestream::FeedbackMessage::ID_GROUPLIST_UPDATED) {
        SetGroups(message->GetBridge());
    }

    if (message->GetId() == huestream::FeedbackMessage::ID_LIGHTS_UPDATED) {
        ClearLights();
        DrawLights();
    }

    if (message->GetId() == huestream::FeedbackMessage::ID_STREAMING_CONNECTED) {
        StartRenderLightsOnUI();
        m_effectPlayer->Start();
        if (message->GetBridge()->IsProxyNodeUnreachable()) {
            AddDebugMsg("(Warning: communicator unreachable)");
        }
    }

    if (message->GetId() == huestream::FeedbackMessage::ID_STREAMING_DISCONNECTED) {
        m_effectPlayer->Stop();
        StopRenderLightsOnUI();
    }

    delete message;
    return LRESULT();
}

LRESULT CStreamTestDlg::OnPostDebugMsg(WPARAM wParam, LPARAM lParam)
{
    auto message = reinterpret_cast<std::string *>(lParam);
    AddDebugMsg(*message);
    delete message;
    return LRESULT();
}

LRESULT CStreamTestDlg::OnTimelineEvent(WPARAM wParam, LPARAM lParam)
{
    auto event = reinterpret_cast<huestream::TimelineState *>(lParam);

    switch (*event) {
    case huestream::TIMELINE_STATE_STARTED:
        GetDlgItem(IDC_PLAY)->SetWindowText("Pause");
        AddDebugMsg("TIMELINESTATE_START");
        break;
    case huestream::TIMELINE_STATE_PAUSED:
        GetDlgItem(IDC_PLAY)->SetWindowText("Play");
        AddDebugMsg("TIMELINESTATE_PAUSE");
        break;
    case huestream::TIMELINE_STATE_RESUMED:
        GetDlgItem(IDC_PLAY)->SetWindowText("Pause");
        AddDebugMsg("TIMELINESTATE_RESUME");
        break;
    case huestream::TIMELINE_STATE_STOPPED:
        GetDlgItem(IDC_PLAY)->SetWindowText("Play");
        AddDebugMsg("TIMELINESTATE_STOP");
        break;
    case huestream::TIMELINE_STATE_ENDED:
        GetDlgItem(IDC_PLAY)->SetWindowText("Play");
        AddDebugMsg("TIMELINESTATE_END");
        break;
    }

    delete event;
    return LRESULT();
}


void CStreamTestDlg::ChangeMode(bool useUDP) {
    if (m_huestream == nullptr)
        return;

    m_effectPlayer->Stop();
    m_huestream->ShutDown();

    InitializeHueStream(useUDP, GetLanguage());
    InitializeEffectPlayerAndTimeline();
}

HueStreamPtr CStreamTestDlg::GetHueStream() {
    return m_huestream;
}

void CStreamTestDlg::OnBnClickedButtonConnect() {
    switch (m_huestream->GetConnectionResult()) {
    case huestream::Busy:
        m_huestream->AbortConnectingAsync();
        ClearLights();
        break;
    case huestream::Uninitialized:
    case huestream::NoBridgeFound:
    case huestream::BridgeFound:
        m_huestream->ConnectBridgeAsync();
        break;
    case huestream::ActionRequired:
    case huestream::ReadyToStart:
        m_huestream->StartAsync();
        break;
    case huestream::Streaming:
        m_huestream->StopAsync();
        break;
    default:
        break;
    }
}

void CStreamTestDlg::PostDebugMsg(const std::string &msg) {
    auto postMsg = new string(msg);
    PostMessage(UWM_POST_DEBUGMSG, 0, reinterpret_cast<LPARAM>(postMsg));
}

void CStreamTestDlg::AddDebugMsg(const std::string &msg) {
    m_msgHistory.push_back(msg);
    if (m_msgHistory.size() > 10) {
        m_msgHistory.pop_front();
    }

    CString s;
    for (auto i = m_msgHistory.begin(); i != m_msgHistory.end(); ++i) {
        s += (*i).c_str();
        s += "\r\n";
    }

    m_edit_log.SetWindowText(s);
}

void CStreamTestDlg::SetUserMessage(const std::string &msg) const {
    CWnd *userMsg = GetDlgItem(IDC_USER_MSG);
    userMsg->SetWindowText(msg.c_str());
}

void CStreamTestDlg::SetIpAndCredentials(LPCSTR ip, LPCSTR uname, LPCSTR key) {
    m_edit_ip.SetWindowText(ip);
    m_edit_username.SetWindowText(uname);
    m_edit_key.SetWindowText(key);
}

void CStreamTestDlg::SetGroups(BridgePtr bridge) {
    m_combo_group.ResetContent();
    for (auto g = bridge->GetGroups()->begin(); g != bridge->GetGroups()->end(); ++g) {
        m_combo_group.AddString((*g)->GetName().c_str());
    }
    if (bridge->IsValidGroupSelected()) {
        m_combo_group.SetCurSel(m_combo_group.FindStringExact(0, bridge->GetGroup()->GetName().c_str()));
    }
}

void CStreamTestDlg::DrawLights() {
    if (m_huestream == nullptr) {
        return;
    }

    CBrush *pBrushBlank;
    CDC *dc = this->GetDC();
    CRect rect;
    CWnd *pWnd = GetDlgItem(IDC_LOCATIONS);
    pWnd->GetWindowRect(&rect);
    ScreenToClient(&rect);

    const auto circleSize = 10;
    const auto topAdjustment = 3;
    auto centerX = rect.CenterPoint().x;
    auto centerY = rect.CenterPoint().y + topAdjustment;
    auto verticalAdjust = -1 * ((rect.Height() / 2) - topAdjustment - circleSize);
    auto horizontalAdjust = (rect.Width() / 2) - circleSize;

    auto lights = m_huestream->GetLoadedBridge()->GetGroupLights();

    for (auto l = lights->begin(); l != lights->end(); ++l) {
        auto color = (*l)->GetColor();
        color.Clamp();
        auto r = color.GetR();
        auto g = color.GetG();
        auto b = color.GetB();
        r = r <= 0.0031308 ? 12.92 * r : (1.0 + 0.055) * pow(r, (1.0 / 2.4)) - 0.055;
        g = g <= 0.0031308 ? 12.92 * g : (1.0 + 0.055) * pow(g, (1.0 / 2.4)) - 0.055;
        b = b <= 0.0031308 ? 12.92 * b : (1.0 + 0.055) * pow(b, (1.0 / 2.4)) - 0.055;

        pBrushBlank = new CBrush(RGB(r * 255, g * 255, b * 255));
        dc->SelectObject(pBrushBlank);

        auto x = (*l)->GetPosition().GetX();
        auto y = (*l)->GetPosition().GetY();

        auto xDraw = static_cast<int>((centerX + x * horizontalAdjust));
        auto yDraw = static_cast<int>((centerY + y * verticalAdjust));

        dc->Ellipse(xDraw - circleSize, yDraw - circleSize, xDraw + circleSize, yDraw + circleSize);
        delete pBrushBlank;
    }

    this->ReleaseDC(dc);

    auto pos = m_timeline->Now();
    m_slider_time.SetPos(static_cast<int>(pos));
    SetTimeBoxPosition(pos, m_timeline->GetLength());
}

void CStreamTestDlg::ClearLights() {
    CRect rect;
    CWnd *pWnd = GetDlgItem(IDC_LOCATIONS);
    pWnd->GetWindowRect(&rect);
    ScreenToClient(&rect);
    RedrawWindow(&rect);
}

void CStreamTestDlg::SetConnectButtonText() const {
    switch (m_huestream->GetConnectionResult()) {
    case huestream::Busy:
        GetDlgItem(IDC_BUTTON_CONNECT)->SetWindowText("Abort");
        break;
    case huestream::Uninitialized:
    case huestream::NoBridgeFound:
    case huestream::BridgeFound:
        GetDlgItem(IDC_BUTTON_CONNECT)->SetWindowText("Connect");
        break;
    case huestream::ActionRequired:
    case huestream::ReadyToStart:
        GetDlgItem(IDC_BUTTON_CONNECT)->SetWindowText("Start");
        break;
    case huestream::Streaming:
        GetDlgItem(IDC_BUTTON_CONNECT)->SetWindowText("Stop");
        break;
    default:
        break;
    }
}

double CStreamTestDlg::GetSpeed() const {
    auto pos = m_slider_speed.GetPos();
    auto adjustedPosition = 1.0 + pow((pos / 10.0), 2.0);
    double speed;
    if (pos >= 0) {
        speed = adjustedPosition;
    } else {
        speed = 1.0 / adjustedPosition;
    }
    return speed;
}

void CStreamTestDlg::SetTimeBoxPosition(int64_t pos, int64_t length) const {
    auto time = static_cast<double>(pos) / 1000.0;
    auto len = static_cast<double>(length) / 1000.0;
    CWnd *timeBox = GetDlgItem(IDC_TIME_BOX);
    std::stringstream stream;
    stream << std::fixed << std::setprecision(2) << time << " / " << std::fixed << std::setprecision(2) << len << " s";
    auto timeString = stream.str();
    timeBox->SetWindowText(timeString.c_str());
}

void CStreamTestDlg::OnBnClickedButtonManual() {
    m_effectPlayer->Stop();

    CString ip;
    m_edit_ip.GetWindowText(ip);
    auto ipAddress = std::string(ip);

    CString uname;
    m_edit_username.GetWindowText(uname);
    auto username = std::string(uname);

    CString key;
    m_edit_key.GetWindowText(key);
    auto clientkey = std::string(key);

    if (ipAddress.empty() || username.empty() || clientkey.empty()) {
        if (ipAddress.empty()) {
            ipAddress = "127.0.0.1";
        }
        m_huestream->ConnectBridgeManualIpAsync(ipAddress);
    } else {
        auto bridge = std::make_shared<huestream::Bridge>(m_huestream->GetConfig()->GetBridgeSettings());
        bridge->SetIpAddress(ipAddress);
        bridge->SetUser(username);
        bridge->SetClientKey(clientkey);
        m_huestream->ConnectManualBridgeInfoAsync(bridge);
    }
}

void CStreamTestDlg::OnBnClickedButtonReset() {
    SetIpAndCredentials("", "", "");
    SetUserMessage("");
    m_combo_group.ResetContent();
    m_huestream->ResetBridgeInfoAsync();
}

void CStreamTestDlg::OnBnClickedExplosion() {
    if (CanPlayEffect()) {
        m_effectPlayer->PlayExplosion();
    }
}

void CStreamTestDlg::OnBnClickedRedWhiteBlue() {
    if (CanPlayEffect()) {
        m_effectPlayer->PlayRedWhiteBlue();
    }
}

void CStreamTestDlg::OnBnClickedCombi() {
    if (CanPlayEffect()) {
        m_effectPlayer->PlayCombi();
    }
}

void CStreamTestDlg::OnBnClickedButtonSequenceRound() {
    if (CanPlayEffect()) {
        m_effectPlayer->PlaySequenceRound();
    }
}

void CStreamTestDlg::OnBnClickedSeqeffectltor() {
    if (CanPlayEffect()) {
        m_effectPlayer->PlaySequenceLtoR();
    }
}

void CStreamTestDlg::OnBnClickedSequencerandom() {
    if (CanPlayEffect()) {
        m_effectPlayer->PlaySequenceFtoB();
    }
}

void CStreamTestDlg::OnBnClickedSaw() {
    if (CanPlayEffect() && !m_saw_toggle) {
        EnableSaw();
    } else {
        DisableSaw();
    }
}

void CStreamTestDlg::EnableSaw() {
    m_effectPlayer->SetSawEnabled(true);
    GetDlgItem(IDC_SAW)->SetWindowText("Stop");
    m_saw_toggle = true;
}

void CStreamTestDlg::DisableSaw() {
    m_effectPlayer->SetSawEnabled(false);
    GetDlgItem(IDC_SAW)->SetWindowText("Start");
    m_saw_toggle = false;
}

void CStreamTestDlg::OnBnClickedSquare() {
    if (CanPlayEffect() && !m_square_toggle) {
        EnableSquare();
    } else {
        DisableSquare();
    }
}

void CStreamTestDlg::EnableSquare() {
    m_effectPlayer->SetSquareEnabled(true);
    GetDlgItem(IDC_SQUARE)->SetWindowText("Stop");
    m_square_toggle = true;
}

void CStreamTestDlg::DisableSquare() {
    m_effectPlayer->SetSquareEnabled(false);
    GetDlgItem(IDC_SQUARE)->SetWindowText("Start");
    m_square_toggle = false;
}

void CStreamTestDlg::OnBnClickedSine() {
    if (CanPlayEffect() && !m_sine_toggle) {
        EnableSine();
    } else {
        DisableSine();
    }
}

void CStreamTestDlg::EnableSine() {
    m_effectPlayer->SetSineEnabled(true);
    GetDlgItem(IDC_SINE)->SetWindowText("Stop");
    m_sine_toggle = true;
}

void CStreamTestDlg::DisableSine() {
    m_effectPlayer->SetSineEnabled(false);
    GetDlgItem(IDC_SINE)->SetWindowText("Start");
    m_sine_toggle = false;
}

void CStreamTestDlg::OnBnClickedMarquee() {
    if (CanPlayEffect() && !m_marquee_toggle) {
        EnableMarquee();
    } else {
        DisableMarquee();
    }
}

void CStreamTestDlg::EnableMarquee() {
    m_effectPlayer->SetMarqueeEnabled(true);
    GetDlgItem(IDC_MARQUEE)->SetWindowText("Stop");
    m_marquee_toggle = true;
}

void CStreamTestDlg::DisableMarquee() {
    m_effectPlayer->SetMarqueeEnabled(false);
    GetDlgItem(IDC_MARQUEE)->SetWindowText("Start");
    m_marquee_toggle = false;
}

void CStreamTestDlg::OnBnClickedColorloop() {
    if (CanPlayEffect() && !m_colorloop_toggle) {
        EnableColorloop();
    } else {
        DisableColorloop();
    }
}

void CStreamTestDlg::EnableColorloop() {
    m_effectPlayer->SetColorloopEnabled(true);
    GetDlgItem(IDC_COLORLOOP)->SetWindowText("Stop");
    m_colorloop_toggle = true;
}

void CStreamTestDlg::DisableColorloop() {
    m_effectPlayer->SetColorloopEnabled(false);
    GetDlgItem(IDC_COLORLOOP)->SetWindowText("Start");
    m_colorloop_toggle = false;
}

void CStreamTestDlg::OnBnClickedFireballs() {
    if (CanPlayEffect() && !m_fireballs_toggle) {
        EnableFireballs();
    } else {
        DisableFireballs();
    }
}

void CStreamTestDlg::EnableFireballs() {
    m_effectPlayer->SetFireballsEnabled(true);
    GetDlgItem(IDC_FIREBALLS)->SetWindowText("Stop");
    m_fireballs_toggle = true;
}

void CStreamTestDlg::DisableFireballs() {
    m_effectPlayer->SetFireballsEnabled(false);
    GetDlgItem(IDC_FIREBALLS)->SetWindowText("Start");
    m_fireballs_toggle = false;
}

void CStreamTestDlg::OnBnClickedPolice() {
    if (CanPlayEffect() && !m_police_toggle) {
        EnablePolice();
    } else {
        DisablePolice();
    }
}

void CStreamTestDlg::EnablePolice() {
    m_effectPlayer->SetPoliceEnabled(true);
    GetDlgItem(IDC_POLICE)->SetWindowText("Stop");
    m_police_toggle = true;
}

void CStreamTestDlg::DisablePolice() {
    m_effectPlayer->SetPoliceEnabled(false);
    GetDlgItem(IDC_POLICE)->SetWindowText("Start");
    m_police_toggle = false;
}

void CStreamTestDlg::OnComboChangedGroup() {
    CString text;
    int sel = m_combo_group.GetCurSel();
    int len = m_combo_group.GetLBTextLen(sel);
    m_combo_group.GetLBText(sel, text.GetBuffer(len));
    std::string groupName(text);
    auto groups = m_huestream->GetLoadedBridgeGroups();
    for (auto g = groups->begin(); g != groups->end(); ++g) {
        if ((*g)->GetName() == groupName) {
            DisableAll();
            m_effectPlayer->Stop();
            m_huestream->SelectGroupAsync((*g)->GetId());
            break;
        }
    }
    text.ReleaseBuffer();
}

void CStreamTestDlg::DisableAll() {
    DisableSaw();
    DisableSquare();
    DisableSine();
    DisableColorloop();
    DisableMarquee();
    DisableFireballs();
    DisablePolice();
}

void CStreamTestDlg::OnComboChangedLanguage() {
    m_huestream->GetConfig()->GetAppSettings()->SetLanguage(GetLanguage());
}

void CStreamTestDlg::OnChangeAlphaSaw() {
    CString transString;
    GetDlgItem(IDC_EDIT_ALPHA1)->GetWindowText(transString);
    m_effectPlayer->m_alpha_saw->SetValue(TransparencyToAlpha(transString));
}

void CStreamTestDlg::OnChangeAlphaSquare() {
    CString transString;
    GetDlgItem(IDC_EDIT_ALPHA2)->GetWindowText(transString);
    m_effectPlayer->m_alpha_square->SetValue(TransparencyToAlpha(transString));
}

void CStreamTestDlg::OnChangeAlphaSine() {
    CString transString;
    GetDlgItem(IDC_EDIT_ALPHA3)->GetWindowText(transString);
    m_effectPlayer->m_alpha_sine->SetValue(TransparencyToAlpha(transString));
}

void CStreamTestDlg::OnChangeAlphaColorloop() {
    CString transString;
    GetDlgItem(IDC_EDIT_ALPHA4)->GetWindowText(transString);
    m_effectPlayer->m_alpha_colorloop->SetValue(TransparencyToAlpha(transString));
}

void CStreamTestDlg::OnChangeAlphaMarquee() {
    CString transString;
    GetDlgItem(IDC_EDIT_ALPHA5)->GetWindowText(transString);
    m_effectPlayer->m_alpha_marquee->SetValue(TransparencyToAlpha(transString));
}

void CStreamTestDlg::OnChangeAlphaFireballs() {
    CString transString;
    GetDlgItem(IDC_EDIT_ALPHA6)->GetWindowText(transString);
    m_effectPlayer->m_alpha_fireballs->SetValue(TransparencyToAlpha(transString));
}

void CStreamTestDlg::OnChangeAlphaPolice() {
    CString transString;
    GetDlgItem(IDC_EDIT_ALPHA7)->GetWindowText(transString);
    m_effectPlayer->m_alpha_police->SetValue(TransparencyToAlpha(transString));
}

double CStreamTestDlg::TransparencyToAlpha(CString transpararency) {
    return (1.0 - (atoi(transpararency) / 100.0));
}

void CStreamTestDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar *pSlider) {
    if (reinterpret_cast<CSliderCtrl *>(pSlider) == &m_slider_speed) {
        double speed = GetSpeed();
        m_effectPlayer->SetSpeed(speed);

        CWnd *speedBox = GetDlgItem(IDC_SPEED_BOX);
        std::stringstream stream;
        stream << std::fixed << std::setprecision(2) << speed << " x";
        std::string speedString = stream.str();
        speedBox->SetWindowText(speedString.c_str());
    }
    if (reinterpret_cast<CSliderCtrl *>(pSlider) == &m_slider_time) {
        auto pos = m_slider_time.GetPos();
        m_timeline->SetPosition(pos);
        SetTimeBoxPosition(pos, m_timeline->GetLength());
    }
}

void CStreamTestDlg::StartRenderLightsOnUI() {
    m_timerVal = SetTimer(ID_TIMER, 50, NULL);
}

void CStreamTestDlg::StopRenderLightsOnUI() {
    KillTimer(m_timerVal);
}

void CStreamTestDlg::OnTimer(UINT_PTR TimerVal) {
    DrawLights();
}

bool CStreamTestDlg::CanPlayEffect() {
    if (!m_huestream->IsBridgeStreaming()) {
        AddDebugMsg("Not started: " + m_huestream->GetLoadedBridge()->GetStatusTag());
        return false;
    }
    return true;
}

std::vector<std::string> CStreamTestDlg::split(const std::string &s, char delim) const {
    std::vector<std::string> elems;
    std::stringstream ss;
    ss.str(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}

void CStreamTestDlg::OnBnClickedSave() {
    auto lightscript = CreateExampleScript();

    TCHAR szFilters[] = _T("JSON Files (*.json)|*.json|All Files (*.*)|*.*||");

    CFileDialog fileDlg(FALSE, _T("json"), _T("*.json"),
                        OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY, szFilters);

    if (fileDlg.DoModal() == IDOK) {
        CString pathName = fileDlg.GetPathName();

        auto storageAccessor = std::make_shared<huestream::FileStorageAccessor>(std::string(pathName));
        storageAccessor->Save(lightscript, [this](huestream::OperationResult r) {
            PostDebugMsg("Saved an example lightscript");
        });
    }
}

void CStreamTestDlg::OnBnClickedLoad() {
    TCHAR szFilters[] = _T("JSON Files (*.json)|*.json|All Files (*.*)|*.*||");

    CFileDialog fileDlg(TRUE, _T("json"), _T("*.json"),
                        OFN_FILEMUSTEXIST | OFN_HIDEREADONLY, szFilters);

    if (fileDlg.DoModal() == IDOK) {
        CString pathName = fileDlg.GetPathName();

        auto storageAccessor = std::make_shared<huestream::FileStorageAccessor>(std::string(pathName));
        storageAccessor->Load([this](huestream::OperationResult r, huestream::SerializablePtr e) {
            m_huestream->LockMixer();

            m_lightscript->Finish();
            m_lightscript = std::static_pointer_cast<huestream::LightScript>(e);
            m_timeline->Stop();
            m_timeline->SetLength(m_lightscript->GetLength());
            m_lightscript->BindTimeline(m_timeline);
            m_huestream->AddLightScript(m_lightscript);

            m_huestream->UnlockMixer();

            m_slider_time.SetRange(0, static_cast<int>(m_lightscript->GetLength()), true);
            m_slider_time.SetPos(0);
            PostDebugMsg("Loaded " + m_lightscript->GetName());
        });
    }
}

void CStreamTestDlg::OnBnClickedPlay() {
    if (m_timeline->IsRunning()) {
        m_timeline->Pause();
    } else if (CanPlayEffect()) {
        m_timeline->Resume();
    }
}

void CStreamTestDlg::OnBnClickedStop() {
    m_timeline->Stop();
}

LightScriptPtr CStreamTestDlg::CreateExampleScript() {
    auto zero = std::make_shared<huestream::ConstantAnimation>(0);
    auto points = std::make_shared<huestream::PointList>();
    points->push_back(std::make_shared<huestream::Point>(0, 0));
    points->push_back(std::make_shared<huestream::Point>(1000, 1));
    points->push_back(std::make_shared<huestream::Point>(2000, 0));
    auto sawAnimationTriple = std::make_shared<huestream::CurveAnimation>(2, points);
    auto sawAnimationInf = std::make_shared<huestream::CurveAnimation>(huestream::INF, points);

    auto redEffect = std::make_shared<huestream::AreaEffect>("redLeft", 0);
    redEffect->SetArea(huestream::Area::Left);
    redEffect->SetColorAnimation(sawAnimationTriple, zero, zero);
    auto redAction = std::make_shared<huestream::Action>("redat0", 0, redEffect, 0);

    auto blueEffect = std::make_shared<huestream::AreaEffect>("blueRight", 0);
    blueEffect->SetArea(huestream::Area::RightHalf);
    blueEffect->SetColorAnimation(zero, zero, sawAnimationInf);
    auto blueAction = std::make_shared<huestream::Action>("blue3to12", 0, blueEffect, 3000, 16000);

    auto explosionEffect = std::make_shared<huestream::LightSourceEffect>("explosion", 1);
    huestream::Color color(1, 0.7, 0.3);
    explosionEffect->SetFixedColor(color);
    explosionEffect->SetPositionAnimation(std::make_shared<huestream::ConstantAnimation>(0), std::make_shared<huestream::ConstantAnimation>(0));
    auto radiusAnimation = std::make_shared<huestream::SequenceAnimation>(0);
    auto fastUp = std::make_shared<huestream::TweenAnimation>(0, 2.2, 500, huestream::TweenType::Linear);
    auto slowDown = std::make_shared<huestream::TweenAnimation>(2.2, 0, 2000, huestream::TweenType::EaseInOutQuad);
    radiusAnimation->Append(fastUp, "FastUp");
    radiusAnimation->Append(slowDown, "SlowDown");
    explosionEffect->SetRadiusAnimation(radiusAnimation);
    auto alphaAnimation = std::make_shared<huestream::SequenceAnimation>(0);
    alphaAnimation->Append(std::make_shared<huestream::TweenAnimation>(0.5, 1, 200, huestream::TweenType::Linear));
    alphaAnimation->Append(std::make_shared<huestream::TweenAnimation>(1, 1, 1500, huestream::TweenType::Linear));
    alphaAnimation->Append(std::make_shared<huestream::TweenAnimation>(1, 0, 800, huestream::TweenType::EaseInOutQuad));
    explosionEffect->SetOpacityAnimation(alphaAnimation);
    auto explosionAction = std::make_shared<huestream::Action>("explosionAt5", 1, explosionEffect, 5000);

    auto marqueeEffect = std::make_shared<huestream::LightpointMarqueeEffect>("marquee", 1);
    marqueeEffect->PrepareEffect(std::make_shared<huestream::ConstantAnimation>(1), std::make_shared<huestream::ConstantAnimation>(1));
    marqueeEffect->SetOpacityBoundToIntensity(true);
    auto marqueeAction = std::make_shared<huestream::Action>("marqueeAt10", 1, marqueeEffect, 10000, 19000);

    auto lightscript = std::make_shared<huestream::LightScript>("testscript", 20000);
    auto locations = std::make_shared<huestream::LocationList>();
    locations->push_back(std::make_shared<huestream::Location>(-1, 0));
    locations->push_back(std::make_shared<huestream::Location>(0, 0));
    locations->push_back(std::make_shared<huestream::Location>(1, 0));
    lightscript->SetIdealSetup(locations);
    lightscript->AddAction(redAction);
    lightscript->AddAction(blueAction);
    lightscript->AddAction(explosionAction);
    lightscript->AddAction(marqueeAction);

    return lightscript;
}

std::string CStreamTestDlg::GetLanguage() const {
    CString text;
    int sel = m_combo_lang.GetCurSel();
    int len = m_combo_lang.GetLBTextLen(sel);
    m_combo_lang.GetLBText(sel, text.GetBuffer(len));
    std::string lang(text);
    text.ReleaseBuffer();
    return lang;
}

void CStreamTestDlg::ShutDown() {
    m_huestream->RegisterFeedbackCallback(nullptr);
    m_effectPlayer->Stop();
    m_effectPlayer = nullptr;
    m_huestream->ShutDown();
    m_huestream = nullptr;
    delete m_pFont;
    huestream::HueEDK::deinit();
}
