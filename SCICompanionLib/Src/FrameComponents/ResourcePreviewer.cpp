// ResourcePreviewer.cpp : implementation file
//
// Contains implementations of all the resource previewers.
//
#include "stdafx.h"
#include "AppState.h"
#include "ResourcePreviewer.h"
#include "CompiledScript.h"
#include "Vocab99x.h"
#include "Vocab000.h"
#include "PicDrawManager.h"
#include "Components.h"
#include "RasterOperations.h"
#include "PaletteOperations.h"
#include "SoundUtil.h"
#include "MidiPlayer.h"
#include "AudioPlayback.h"
#include "ResourceEntity.h"
#include "SummarizeScript.h"
#include <vfw.h>

BOOL ResourcePreviewer::OnInitDialog()
{
    BOOL fRet = __super::OnInitDialog();
    ShowSizeGrip(FALSE);
    return fRet;
}

//
// Pic Previewer
//
void PicPreviewer::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);

    DDX_Control(pDX, IDC_STATICVISUAL, m_wndVisual);
    DDX_Control(pDX, IDC_GROUPVISUAL, m_wndVisualGroup);
    m_wndVisualGroup.SetStyle(CExtGroupBox::e_style_t::STYLE_CAPTION);
    DDX_Control(pDX, IDC_STATICPRIORITY, m_wndPriority);
    DDX_Control(pDX, IDC_GROUPPRIORITY, m_wndPriorityGroup);
    m_wndPriorityGroup.SetStyle(CExtGroupBox::e_style_t::STYLE_CAPTION);
    DDX_Control(pDX, IDC_STATICCONTROL, m_wndControl);
    DDX_Control(pDX, IDC_GROUPCONTROL, m_wndControlGroup);
    m_wndControlGroup.SetStyle(CExtGroupBox::e_style_t::STYLE_CAPTION);

    // Visuals
    DDX_Control(pDX, IDC_BUTTON1, m_wndButton1);
    DDX_Control(pDX, IDC_BUTTON2, m_wndButton2);
    DDX_Control(pDX, IDC_BUTTON3, m_wndButton3);
    DDX_Control(pDX, IDC_BUTTON4, m_wndButton4);
    DDX_Control(pDX, IDC_STATICPALETTE, m_wndStaticPalette);
}

PicPreviewer::~PicPreviewer()
{
}

BEGIN_MESSAGE_MAP(PicPreviewer, ResourcePreviewer)
    ON_COMMAND_EX(IDC_BUTTON1, OnSetPalette)
    ON_COMMAND_EX(IDC_BUTTON2, OnSetPalette)
    ON_COMMAND_EX(IDC_BUTTON3, OnSetPalette)
    ON_COMMAND_EX(IDC_BUTTON4, OnSetPalette)
    ON_WM_ERASEBKGND()
END_MESSAGE_MAP()

// CNoFlickerStatic message handlers
BOOL PicPreviewer::OnEraseBkgnd(CDC *pDC)
{
    return TRUE;
}

BOOL PicPreviewer::OnSetPalette(UINT nID)
{
    ASSERT((IDC_BUTTON4 - IDC_BUTTON1) == 3);
    _paletteNumber = (BYTE)(nID - IDC_BUTTON1);
    PicDrawManager pdm(&_pic->GetComponent<PicComponent>(), _pic->TryGetComponent<PaletteComponent>());

    pdm.SetPalette(_paletteNumber);
    _ResetVisualBitmap(pdm);

    // Update buttons states - ON_UPDATE_COMMAND_UI doesn't work in dialogs.
    for (UINT buttonId = IDC_BUTTON1; buttonId <= IDC_BUTTON4; buttonId++)
    {
        CExtButton *pButton = static_cast<CExtButton*>(GetDlgItem(buttonId));
        pButton->SetCheck(buttonId == nID);
    }
    return TRUE;
}

void PicPreviewer::OnUpdatePaletteButton(CCmdUI *pCmdUI)
{
    BYTE paletteNumber = (BYTE)(pCmdUI->m_nID - IDC_BUTTON1);
    pCmdUI->Enable(TRUE);
    pCmdUI->SetCheck(paletteNumber == _paletteNumber);
}

void PicPreviewer::_ResetVisualBitmap(PicDrawManager &pdm)
{
    CRect rc;
    m_wndVisual.GetClientRect(&rc);
    CBitmap bitmap;
    bitmap.Attach(pdm.CreateBitmap(PicScreen::Visual, PicPosition::Final, rc.Width(), rc.Height()));
    m_wndVisual.FromBitmap((HBITMAP)bitmap, rc.Width(), rc.Height());
}

void PicPreviewer::SetResource(const ResourceBlob &blob)
{
    _pic = CreateResourceFromResourceData(blob);

    PicDrawManager pdm(&_pic->GetComponent<PicComponent>(), _pic->TryGetComponent<PaletteComponent>());
    pdm.SetPalette(_paletteNumber);
    pdm.RefreshAllScreens(PicScreenFlags::All, PicPositionFlags::Final); // Be efficient - we're going to get all 3 screens.
    _ResetVisualBitmap(pdm);

    // Do the priority and controls too.
    CRect rc;
    m_wndVisual.GetClientRect(&rc);
    CBitmap bitmapP;
    bitmapP.Attach(pdm.CreateBitmap(PicScreen::Priority, PicPosition::Final, rc.Width(), rc.Height()));
    m_wndPriority.FromBitmap((HBITMAP)bitmapP, rc.Width(), rc.Height());
    CBitmap bitmapC;
    bitmapC.Attach(pdm.CreateBitmap(PicScreen::Control, PicPosition::Final, rc.Width(), rc.Height()));
    m_wndControl.FromBitmap((HBITMAP)bitmapC, rc.Width(), rc.Height());
}

//
// View previewer
//
BEGIN_MESSAGE_MAP(ViewPreviewer, ResourcePreviewer)
END_MESSAGE_MAP()

void ViewPreviewer::SetResource(const ResourceBlob &blob)
{
    //
    // Generate a large bitmap containing all the views.
    //
    _view = CreateResourceFromResourceData(blob);
    CBitmap bitmap;
    SCIBitmapInfo bmi;
    BYTE *pBitsDest;

    std::unique_ptr<PaletteComponent> optionalPalette;
    if (_view->GetComponent<RasterComponent>().Traits.PaletteType == PaletteType::VGA_256)
    {
        optionalPalette = appState->GetResourceMap().GetMergedPalette(*_view, 999);
    }
    bitmap.Attach(CreateBitmapFromResource(*_view, optionalPalette.get(), &bmi, &pBitsDest));
    m_wndView.FromBitmap((HBITMAP)bitmap, bmi.bmiHeader.biWidth, abs(bmi.bmiHeader.biHeight));

    // Don't use the fallback here:
    if (_view->TryGetComponent<PaletteComponent>())
    {
        CBitmap bitmapPalette;
        SCIBitmapInfo bmiPalette;
        COLORREF background = g_PaintManager->GetColor(COLOR_3DFACE);
        std::vector<const Cel*> cels;
        RasterComponent &raster = _view->GetComponent<RasterComponent>();
        for (Loop &loop : raster.Loops)
        {
            for (Cel &cel : loop.Cels)
            {
                cels.push_back(&cel);
            }
        }

        bitmapPalette.Attach(CreateBitmapFromPaletteResource(_view.get(), &bmiPalette, &pBitsDest, &background, &cels));
        m_wndPalette.ShowWindow(SW_SHOW);
        m_wndPalette.FromBitmap((HBITMAP)bitmapPalette, bmiPalette.bmiHeader.biWidth, abs(bmiPalette.bmiHeader.biHeight));
    }
    else
    {
        m_wndPalette.ShowWindow(SW_HIDE);
        //m_wndPalette.FromBitmap(nullptr, 0, 0);
    }
}

void ViewPreviewer::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_STATICVIEW, m_wndView);
    AddAnchor(IDC_STATICVIEW, CPoint(0, 0), CPoint(100, 100));
    DDX_Control(pDX, IDC_STATIC2, m_wndPalette);
    AddAnchor(IDC_STATIC2, CPoint(50, 100), CPoint(50, 100));
}

//
// Palette previewer
//
BEGIN_MESSAGE_MAP(PalettePreviewer, ResourcePreviewer)
END_MESSAGE_MAP()

void PalettePreviewer::SetResource(const ResourceBlob &blob)
{
    //
    // Generate a large bitmap containing all the views.
    //
    _palette = CreateResourceFromResourceData(blob);
    CBitmap bitmap;
    SCIBitmapInfo bmi;
    BYTE *pBitsDest;
    COLORREF background = g_PaintManager->GetColor(COLOR_3DFACE);
    bitmap.Attach(CreateBitmapFromPaletteResource(_palette.get(), &bmi, &pBitsDest, &background));
    m_wndView.FromBitmap((HBITMAP)bitmap, bmi.bmiHeader.biWidth, abs(bmi.bmiHeader.biHeight));
}

void PalettePreviewer::DoDataExchange(CDataExchange* pDX)
{
    __super::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_STATICVIEW, m_wndView);
    AddAnchor(IDC_STATICVIEW, CPoint(0, 0), CPoint(100, 100));
    DDX_Control(pDX, IDC_STATIC2, m_wndPaletteNOT);
    AddAnchor(IDC_STATIC2, CPoint(100, 100), CPoint(100, 100));
}

//
// Script previewer
//
BEGIN_MESSAGE_MAP(ScriptPreviewer, ResourcePreviewer)
END_MESSAGE_MAP()

ScriptPreviewer::~ScriptPreviewer() {}

void ScriptPreviewer::SetResource(const ResourceBlob &blob)
{
    // Try to find a source file.
    std::string scriptFileName = appState->GetResourceMap().GetScriptFileName(blob.GetName());

    std::ifstream scriptFile(scriptFileName.c_str());
    if (scriptFile.is_open())
    {
        ScriptId scriptId(scriptFileName);
        if (scriptId.Language() == LangSyntaxSCIStudio)
        {
            m_wndHeader.SetWindowText("Language: SCI Studio");
        }
        else
        {
            m_wndHeader.SetWindowText("Language: C syntax");
        }

        std::string scriptText;
        std::string line;
        while (std::getline(scriptFile, line))
        {
            scriptText += line;
            scriptText += "\r\n";

        }
        m_wndEdit.SetWindowText(scriptText.c_str());
    }
    else
    {
        m_wndHeader.SetWindowText("");

        // If that wasn't possible, spew info from the compiled script resource:
        CompiledScript compiledScript(0);
        if (compiledScript.Load(appState->GetVersion(), blob.GetNumber(), blob.GetReadStream()))
        {
            // Write some crap.
            std::stringstream out;
            DebugOut(compiledScript, out, true);
            // Now we have a stream.  Put it in the edit box
            m_wndEdit.SetWindowText(out.str().c_str());
        }
        else
        {
            m_wndEdit.SetWindowText("Unable to load script.");
        }
    }
}

void ScriptPreviewer::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);

    DDX_Control(pDX, IDC_EDITSCRIPT, m_wndEdit);
    DDX_Control(pDX, IDC_STATICHEADER, m_wndHeader);
    AddAnchor(IDC_EDITSCRIPT, CPoint(0, 0), CPoint(100, 100));
}



//
// Text previewer
//
BEGIN_MESSAGE_MAP(TextPreviewer, ResourcePreviewer)
END_MESSAGE_MAP()

TextPreviewer::~TextPreviewer() {}

void TextPreviewer::SetResource(const ResourceBlob &blob)
{
    std::unique_ptr<ResourceEntity> resource = CreateResourceFromResourceData(blob);
    TextComponent *pText = resource->TryGetComponent<TextComponent>();
    if (pText)
    {
        std::stringstream ss;
        for (auto &aString : pText->Texts)
        {
            ss << aString.Text << "\r\n";
        }
        m_wndEdit.SetWindowText(ss.str().c_str());
    }
    else
    {
        m_wndEdit.SetWindowText("Unable to load text resource.");
    }
}

void TextPreviewer::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_EDITSCRIPT, m_wndEdit);
    AddAnchor(IDC_EDITSCRIPT, CPoint(0, 0), CPoint(100, 100));
}



//
// Vocab previewer
//
BEGIN_MESSAGE_MAP(VocabPreviewer, ResourcePreviewer)
END_MESSAGE_MAP()

VocabPreviewer::~VocabPreviewer() {}


void VocabPreviewer::_Populate(const std::vector<std::string> &names, bool prependNumber)
{
    std::vector<std::string>::const_iterator it = names.begin();
    
    std::stringstream text;
    int index = 0;
    while (it != names.end())
    {
        if (prependNumber)
        {
            text << index << ": ";
        }

        const std::string &line = (*it);
        // Replace \n with \r\n for display in the edit control
        size_t position = 0;
        size_t carriageReturnPos = line.find('\n', position);
        while (carriageReturnPos != std::string::npos)
        {
            text << line.substr(position, carriageReturnPos - position);
            position = carriageReturnPos + 1;

            text << "\r\n";
            carriageReturnPos = line.find('\n', position);
        }

        text << line.substr(position);

        text << "\r\n";
        ++it;
        index++;
    }
    m_wndEdit.SetWindowText(text.str().c_str());
}

void VocabPreviewer::SetResource(const ResourceBlob &blob)
{
    // It appears that the different vocabs have different formats.  We would need
    // to write previers for each kind.
    // Let's use some heuristics to determine how to load them.
    int iNumber = blob.GetNumber();
    bool fSuccess = false;

    if (iNumber == appState->GetVersion().MainVocabResource)
    {
        CPrecisionTimer timer;
        timer.Start();
        double initTime = 0.0;
        double popTime = 0.0;
        std::unique_ptr<ResourceEntity> pResource = CreateResourceFromResourceData(blob);
        if (pResource)
        {
            initTime = timer.Stop();

            timer.Start();
            _Populate(pResource->GetComponent<Vocab000>().GetWords());
            popTime = timer.Stop();
            fSuccess = true;
        }
    }
    else
    {
        switch (iNumber)
        {
        case 995: // debug info
        {
            CVocabWithNames vocab;
            if (vocab.Create(&blob.GetReadStream(), true))
            {
                _Populate(vocab.GetNames());
                fSuccess = true;
            }
        }
        break;
        case 996: // species table
        {
            SpeciesTable species;
            if (species.Load())
            {
                _Populate(species.GetNames());
                fSuccess = true;
            }
        }
        break;
        case 997: // selector table
        {
            SelectorTable selectors;
            if (selectors.Load(blob.GetVersion()))
            {
                _Populate(selectors.GetNames(), true);
                fSuccess = true;
            }
        }
        break;
        case 999: // kernel functions
        {
            KernelTable kernels;
            if (kernels.Load())
            {
                _Populate(kernels.GetNames(), true);
                fSuccess = true;
            }
        }
        break;
        default:
            m_wndEdit.SetWindowText(GetBinaryDataVisualization(blob.GetData(), blob.GetLength()).c_str());
            fSuccess = true;
            break;
        }
    }

    if (!fSuccess)
    {
        m_wndEdit.SetWindowText("Unable to load vocab resource.");
    }
}

void VocabPreviewer::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_EDITSCRIPT, m_wndEdit);
    AddAnchor(IDC_EDITSCRIPT, CPoint(0, 0), CPoint(100, 100));
}



//
// Font previewer
//
BEGIN_MESSAGE_MAP(FontPreviewer, ResourcePreviewer)
END_MESSAGE_MAP()

FontPreviewer::~FontPreviewer() {}

void FontPreviewer::SetResource(const ResourceBlob &blob)
{
    _pFont = CreateResourceFromResourceData(blob);
    _pWndFontView->SetFontResource(_pFont.get());
}

BOOL FontPreviewer::OnInitDialog()
{
    BOOL fRet = __super::OnInitDialog();
    CRect rc;
    GetClientRect(&rc);
    CCreateContext context;
    context.m_pNewViewClass = RUNTIME_CLASS(CFontPreviewView);
    _pWndFontView = static_cast<CFontPreviewView*>(context.m_pNewViewClass->CreateObject());
    if (!_pWndFontView->Create(NULL, NULL, AFX_WS_DEFAULT_VIEW,
		rc, this, 12345, &context))
    {
        return FALSE;
    }
    else
    {
        AddAnchor(_pWndFontView->GetSafeHwnd(), CSize(0, 0), CSize(100, 100));
    }
    return fRet;
}

void FontPreviewer::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);
}




//
// Sound previewer
//
BEGIN_MESSAGE_MAP(SoundPreviewer, ResourcePreviewer)
    ON_CBN_SELCHANGE(IDC_COMBO_DEVICE, OnSynthChoiceChange)
    ON_CBN_SELCHANGE(IDC_COMBO_MIDIDEVICE, OnMIDIDeviceChange)
    ON_BN_CLICKED(IDC_BUTTON_PLAY, OnPlay)
    ON_BN_CLICKED(IDC_BUTTON_STOP, OnStop)
    ON_WM_TIMER()
END_MESSAGE_MAP()

#define SOUND_TIMER 5003

SoundPreviewer::SoundPreviewer()
{
    _fPlaying = false;
}


void SoundPreviewer::SetResource(const ResourceBlob &blob)
{
    _sound = CreateResourceFromResourceData(blob);
    g_midiPlayer.SetSound(_sound->GetComponent<SoundComponent>(), SoundComponent::StandardTempo); // We don't have a tempo control
    OnSynthChoiceChange();
    if (m_wndAutoPreview.GetCheck() == BST_CHECKED)
    {
        OnPlay();
    }
    else
    {
        OnStop();
    }
}

void SoundPreviewer::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);

    DDX_Control(pDX, IDC_COMBO_DEVICE, m_wndSynths);
    DDX_Control(pDX, IDC_EDIT_CHANNELS, m_wndChannels);
    DDX_Control(pDX, IDC_STATIC_SYNTH, m_wndStaticSynth);
    DDX_Control(pDX, IDC_BUTTON_PLAY, m_wndPlay);
    DDX_Control(pDX, IDC_BUTTON_STOP, m_wndStop);
    DDX_Control(pDX, IDC_SLIDER, m_wndSlider);
    DDX_Control(pDX, IDC_CHECK_AUTOPREV, m_wndAutoPreview);

    AddAnchor(IDC_COMBO_DEVICE, CPoint(0, 0), CPoint(100, 0));
    AddAnchor(IDC_SLIDER, CPoint(0, 0), CPoint(100, 0));
}



BOOL SoundPreviewer::OnInitDialog()
{
    BOOL fRet = __super::OnInitDialog();
    CRect rc;
    GetClientRect(&rc);
   
    CDC *pDC = GetDC();
    {
        LOGFONT logFont = { 0 };
        StringCchCopy(logFont.lfFaceName, ARRAYSIZE(logFont.lfFaceName), "Marlett");
        logFont.lfHeight = -MulDiv(10, GetDeviceCaps((HDC)*pDC, LOGPIXELSY), 72);
	    logFont.lfWeight = FW_NORMAL;
	    logFont.lfItalic = FALSE;
	    logFont.lfCharSet = DEFAULT_CHARSET;
	    logFont.lfOutPrecision = OUT_DEFAULT_PRECIS;
	    logFont.lfClipPrecision = CLIP_DEFAULT_PRECIS;
	    logFont.lfQuality = DEFAULT_QUALITY;
	    logFont.lfPitchAndFamily = FIXED_PITCH;
        _marlettFont.CreateFontIndirect(&logFont);
        m_wndPlay.SetFont(&_marlettFont);
        m_wndStop.SetFont(&_marlettFont);
    }

    // Add the items to the combobox.
    PopulateComboWithDevicesHelper(m_wndSynths);
    m_wndSynths.SetCurSel(0);
    OnSynthChoiceChange();

    m_wndSlider.SetRange(0, 100);

    _UpdatePlayState();

    ReleaseDC(pDC);
    return fRet;
}

std::string SoundPreviewer::_FillChannelString(BYTE bChannel, bool fHeader)
{
    std::string channelString;
    ASSERT(((0x1 << bChannel) & _wChannelMask) || (bChannel == 0));
    WORD wMask = _wChannelMask;
    for (int i = 0; i < 15; i++)
    {
        if (wMask & 0x1)
        {
            // This channel is used.
            if (fHeader)
            {
                char sz[3];
                StringCchPrintf(sz, ARRAYSIZE(sz), "%1x", i);
                channelString += sz;
            }
            else
            {
                if (bChannel == i)
                {
                    // It's used by this event.
                    channelString += "X";
                }
                else
                {
                    channelString += " ";
                }
            }
        }
        wMask >>= 1;
    }
    return channelString;
}

void SoundPreviewer::OnSynthChoiceChange()
{
    // Recalculate the mask.
    _device = GetDeviceFromComboHelper(m_wndSynths);
    if (_sound)
    {
        std::string channelText;
        _wChannelMask = _sound->GetComponent<SoundComponent>().GetChannelMask(_device);
        for (int i = 0; i < 16; i++)
        {
            channelText += ((_wChannelMask >> i) & 0x1)? "1" : "0";
        }
        m_wndChannels.SetWindowText(channelText.c_str());
       
    }
}

void SoundPreviewer::OnMIDIDeviceChange()
{
    // TODO
}
void SoundPreviewer::_UpdatePlayState()
{
    if (_fPlaying)
    {
        SetTimer(SOUND_TIMER, 100, NULL);
    }
    else
    {
        KillTimer(SOUND_TIMER);
    }
    m_wndPlay.EnableWindow(_sound.get() && !_fPlaying);
    m_wndStop.EnableWindow(_sound.get() && _fPlaying);
}
void SoundPreviewer::OnPlay()
{
#if SOUND_IMPLEMENTED
    if (_sound->GetComponent<SoundComponent>().Frequency != 0)
    {
        g_audioPlayback.Play(_sound->GetComponent<SoundComponent>());
    }
    else
#endif
    {

        g_midiPlayer.Play();
        _fPlaying = true;
        _UpdatePlayState();
    }
}
void SoundPreviewer::OnStop()
{
    g_midiPlayer.Stop();
    _fPlaying = false;
    _UpdatePlayState();
}
void SoundPreviewer::OnTimer(UINT_PTR nIDEvent)
{
    if (nIDEvent == SOUND_TIMER)
    {
        if (_fPlaying)
        {
            m_wndSlider.SetPos(g_midiPlayer.QueryPosition(100));
        }
    }
    else
    {
        __super::OnTimer(nIDEvent);
    }
}

//
// Blank previewer
//
BEGIN_MESSAGE_MAP(BlankPreviewer, ResourcePreviewer)
END_MESSAGE_MAP()

