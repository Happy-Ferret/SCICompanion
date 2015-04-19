#pragma once

#include "Resource.h"
#include "NoFlickerStatic.h"

// CGameVersionDialog dialog

class CGameVersionDialog : public CExtNCW<CExtResizableDialog>
{
public:
    CGameVersionDialog(SCIVersion &version, CWnd* pParent = NULL);   // standard constructor

    // Dialog Data
    enum { IDD = IDD_DIALOGVERSION };

    SCIVersion &_version;


private:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    virtual void OnOK() override;
    afx_msg void OnViewResourceMap();
    afx_msg void OnViewMessageMap();
    DECLARE_MESSAGE_MAP()

    void _Sync();
    SCIVersion _ReverseSync();

    int _fHasPalette;
    int _fGrayscaleCursors;
    int _fCodeSCI1;
    int _fSeparateHeapResources;
    int _fVocab900;
    int _fEarlySCI0Script;
    int _fSCI11Palettes;

    int _viewFormat;
    int _picFormat;
    int _resourceMapVersion;
    int _resourcePackVersion;
    int _compressionVersion;
    int _soundVersion;

    CExtButton m_wndOk;
    CExtButton m_wndCancel;
    CExtButton m_wndViewResMap;
    CExtButton m_wndViewMessageMap;

    CExtGroupBox m_wndGroupResourceMap;
    CExtRadioButton m_wndRadioResourceMapSCI0;
    CExtRadioButton m_wndRadioResourceMapSCI1;
    CExtRadioButton m_wndRadioResourceMapSCI11;

    CExtGroupBox m_wndGroupResourcePack;
    CExtRadioButton m_wndRadioResourcePackSCI0;
    CExtRadioButton m_wndRadioResourcePackSCI1;
    CExtRadioButton m_wndRadioResourcePackSCI11;

    CExtGroupBox m_wndGroupSound;
    CExtRadioButton m_wndRadioSoundSCI0;
    CExtRadioButton m_wndRadioSoundSCI1;

    CExtGroupBox m_wndGroupCompression;
    CExtRadioButton m_wndRadioCompressionSCI0;
    CExtRadioButton m_wndRadioCompressionSCI1;

    CExtComboBox m_wndViewCombo;
    CExtComboBox m_wndPicCombo;

    CExtNoFlickerStatic m_wndLabel4;
    CExtNoFlickerStatic m_wndLabel5;

    CExtCheckBox m_wndHasPalette;
    CExtCheckBox m_wndGrayscaleCursors;
    CExtCheckBox m_wndSCI1Code;
    CExtCheckBox m_wndSepHeap;
    CExtCheckBox m_wndParserVocab900;
    CExtCheckBox m_wndEarlySCI0Script;
    CExtCheckBox m_wndSCI11Palettes;
};