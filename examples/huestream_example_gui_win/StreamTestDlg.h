/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#pragma once

#include <huestream/HueStream.h>
#include <huestream_example_gui_win/EffectPlayer.h>
#include <huestream/common/data/Bridge.h>

#include <list>
#include <string>
#include <vector>

#include "afxwin.h"  // NOLINT

using huestream::HueStreamPtr;
using huestream::BridgePtr;
using huestream::GroupPtr;
using huestream::TimelinePtr;
using huestream::LightScriptPtr;

class CStreamTestDlg : public CDialogEx {
public:
    explicit CStreamTestDlg(CWnd *pParent = NULL);

    void ChangeMode(bool useUDP);

    HueStreamPtr GetHueStream();

#ifdef AFX_DESIGN_TIME
    enum { IDD = IDD_STREAMTEST_DIALOG };
#endif

protected:
    void DoDataExchange(CDataExchange *pDX) override;

    HICON m_hIcon;
    EffectPlayerPtr m_effectPlayer;

    std::list<std::string> m_msgHistory;

    BOOL OnInitDialog() override;

    afx_msg void OnSysCommand(UINT nID, LPARAM lParam);

    afx_msg void OnPaint();

    afx_msg HCURSOR

    OnQueryDragIcon();

    DECLARE_MESSAGE_MAP()

    bool IsUDPForcedByCommandLine();

    const std::string GetDeviceName() const;

    void InitializeUserInterface();

    void InitializeHueStream(bool useUDP = false, const std::string language = "en");

    void InitializeEffectPlayerAndTimeline();

    void PostDebugMsg(const std::string &msg);

    void AddDebugMsg(const std::string &msg);

    void SetUserMessage(const std::string &msg) const;

    void SetIpAndCredentials(LPCSTR ip, LPCSTR uname, LPCSTR key);

    void SetGroups(BridgePtr bridge);

    void StartRenderLightsOnUI();

    void StopRenderLightsOnUI();

    void DrawLights();

    void ClearLights();

    void SetConnectButtonText() const;

    double GetSpeed() const;

    void SetTimeBoxPosition(int64_t pos, int64_t length) const;

    bool CanPlayEffect();

    static double TransparencyToAlpha(CString transpararency);

    std::vector<std::string> split(const std::string &s, char delim) const;

    void EnableSaw();

    void EnableSquare();

    void EnableSine();

    void EnableColorloop();

    void EnableMarquee();

    void EnableFireballs();

    void EnablePolice();

    void DisableSaw();

    void DisableSquare();

    void DisableSine();

    void DisableColorloop();

    void DisableMarquee();

    void DisableFireballs();

    void DisablePolice();

    void DisableAll();

    LightScriptPtr CreateExampleScript();

    UINT_PTR m_timerVal = 0;
    CEdit m_edit_ip;
    CEdit m_edit_username;
    CEdit m_edit_key;
    CEdit m_edit_log;
    CComboBox m_combo_group;
    CComboBox m_combo_lang;
    CSliderCtrl m_slider_speed;
    CSliderCtrl m_slider_time;
    bool m_saw_toggle = false;
    bool m_square_toggle = false;
    bool m_sine_toggle = false;
    bool m_marquee_toggle = false;
    bool m_colorloop_toggle = false;
    bool m_fireballs_toggle = false;
    bool m_police_toggle = false;
    TimelinePtr m_timeline;
    LightScriptPtr m_lightscript;
    HueStreamPtr m_huestream;
    CFont *m_pFont;

public:
    afx_msg void OnBnClickedButtonConnect();

    afx_msg void OnBnClickedExplosion();

    afx_msg void OnBnClickedRedWhiteBlue();

    afx_msg void OnBnClickedCombi();

    afx_msg void OnBnClickedButtonManual();

    afx_msg void OnBnClickedButtonReset();

    afx_msg void OnBnClickedButtonSequenceRound();

    afx_msg void OnBnClickedSeqeffectltor();

    afx_msg void OnBnClickedSequencerandom();

    afx_msg void OnComboChangedGroup();

    afx_msg void OnComboChangedLanguage();

    afx_msg void OnBnClickedSaw();

    afx_msg void OnBnClickedSquare();

    afx_msg void OnBnClickedSine();

    afx_msg void OnBnClickedMarquee();

    afx_msg void OnBnClickedColorloop();

    afx_msg void OnBnClickedFireballs();

    afx_msg void OnBnClickedPolice();

    afx_msg void OnChangeAlphaSaw();

    afx_msg void OnChangeAlphaSquare();

    afx_msg void OnChangeAlphaSine();

    afx_msg void OnChangeAlphaColorloop();

    afx_msg void OnChangeAlphaMarquee();

    afx_msg void OnChangeAlphaFireballs();

    afx_msg void OnChangeAlphaPolice();

    afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar *pSlider);

    afx_msg void OnTimer(UINT_PTR TimerVal);

    afx_msg void OnBnClickedSave();

    afx_msg void OnBnClickedLoad();

    afx_msg void OnBnClickedPlay();

    afx_msg void OnBnClickedStop();

    afx_msg LRESULT OnHuestreamFeedback(WPARAM wParam, LPARAM lParam);

    afx_msg LRESULT OnPostDebugMsg(WPARAM wParam, LPARAM lParam);

    afx_msg LRESULT OnTimelineEvent(WPARAM wParam, LPARAM lParam);

    std::string GetLanguage() const;

    void ShutDown();
};

