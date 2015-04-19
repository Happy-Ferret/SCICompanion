#pragma once

#include "ResourceDocument.h"
#include "UndoResource.h"
#include "Components.h"
#include "ResourceEntity.h"
#include "ResourceEntityDocument.h"
#include "RasterOperations.h"

// CNewRasterResourceDocument document

// A sort of workaround
CHintWithObject<CelIndex> WrapRasterChange(RasterChange change);

class CNewRasterResourceDocument : public ResourceEntityDocument
{
    DECLARE_DYNCREATE(CNewRasterResourceDocument)

public:
    CNewRasterResourceDocument();

    void SetResource(std::unique_ptr<ResourceEntity> pResource, int id = -1);
    void SetNewResource(std::unique_ptr<ResourceEntity> pResource);

    void LockResource(bool fLock) { _fLocked = fLock; }
    int GetPenWidth() { return _nPenWidth; }
    void SetPenWidth(int nPenWidth);
    void SetApplyToAllCels(BOOL fApply) { _fApplyToAllCels = !!fApply; UpdateAllViewsAndNonViews(nullptr, 0, &WrapHint(RasterChangeHint::ApplyToAll)); }
    BOOL GetApplyToAllCels() const { return _fApplyToAllCels; }

    // THESE ARE TODO
    CelIndex GetSelectedIndex() { return CelIndex(_nLoop, _nCel); }
    int GetSelectedGroup(CelIndex *rgGroups, size_t ceGroup);
    void MoveSelectedCel(CPoint point);

    uint8_t GetViewColor() { return _color; }
    uint8_t GetAlternateViewColor() { return _alternateColor; }
    void SetViewColor(uint8_t color);
    void SetAlternateViewColor(uint8_t color);
    COLORREF SCIColorToCOLORREF(uint8_t color);

    const RGBQUAD *GetPaletteVGA() const;
    const PaletteComponent *GetCurrentPaletteComponent() const;
    int GetDefaultZoom() const;
    void SetDefaultZoom(int iZoom) const;
    BOOL CanDeleteCels() const { return TRUE; } // REVIEW: Not true for fonts?

    int GetSelectedCel() const { return _nCel; }
    int GetSelectedLoop() const { return _nLoop; }
    void SetSelectedLoop(int nLoop);
    void SetSelectedCel(int nCel);
    void GetLabelString(PTSTR  pszLabel, size_t cch, int nCel) const { StringCchPrintf(pszLabel, cch, TEXT("%d"), nCel); }

    void SetPreviewLetters(std::string &previewLetters)
    {
        _previewLetters = previewLetters;
        UpdateAllViewsAndNonViews(nullptr, 0, &WrapHint(RasterChangeHint::SampleText));
    }
    std::string GetPreviewLetters() { return _previewLetters;  }
    void MakeFont();

    bool v_PreventUndos() { return _fLocked; }
    void v_OnUndoRedo();

    template<typename _T>
    _T &GetComponent()
    {
        return GetResource()->GetComponent<_T>();
    }

    std::vector<int> &GetPaletteChoices() { return _paletteChoices; }
    std::string GetPaletteChoiceName(int index);
    int GetPaletteChoice() { return _currentPaletteIndex; }
    void SetPaletteChoice(int choice);

private:
    // REVIEW TODO: Need to have this be dyanmic?
    virtual ResourceType _GetType() const
    {
        const ResourceEntity *pResource = static_cast<const ResourceEntity*>(GetResource());
        return pResource->GetType();
    }

    // Ensure the currently selected cel index is correct.
    void _SetInitialPalette();
    void _ValidateCelIndex();
    void _UpdateHelper(RasterChange change);
    void _TrimUndoStack();
    void _InsertFiles(const std::vector<std::string> &files);
    std::list<ResourceEntity*>::iterator _GetLastUndoFrame();

    template<typename _T>
    const _T &GetComponent() const
    {
        return GetResource()->GetComponent<_T>();
    }
    
    DECLARE_MESSAGE_MAP()
    afx_msg void OnAnimate();
    afx_msg void OnImportImageSequence();
    afx_msg void OnUpdateImportImage(CCmdUI *pCmdUI);
    afx_msg void OnUpdateAnimate(CCmdUI *pCmdUI);

    bool _fLocked;
    int _nPenWidth;
    bool _fApplyToAllCels;
    int _nCel;
    int _nLoop;

    uint8_t _color;
    uint8_t _alternateColor;

    RGBQUAD _currentPaletteVGA[256];
    // Keep around one of these too:
    std::unique_ptr<PaletteComponent> _currentPaletteComponent;
    std::vector<int> _paletteChoices;
    int _currentPaletteIndex;  // index into _paletteChoices

    // These should go away
    std::string _previewLetters = "!@#$%^&*()_+0123456789-=~`ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz{}|[]\\:\";'<>?,./";
};