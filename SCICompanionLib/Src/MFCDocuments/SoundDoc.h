#pragma once

#include "ResourceEntityDocument.h"
#include "Sound.h"

class ResourceEntity;

class CSoundDoc : public ResourceEntityDocument
{
	DECLARE_DYNCREATE(CSoundDoc)

public:
	CSoundDoc();
	virtual ~CSoundDoc();
	virtual void Serialize(CArchive& ar);   // overridden for document i/o

    // Takes ownership:
    void SetSoundResource(std::unique_ptr<ResourceEntity> pSound, int id = -1);
    SoundComponent *GetSoundComponent() const;
    DeviceType GetDevice() const { return _device; }
    void SetDevice(DeviceType device, bool fNotify = true);

    void SetTempo(WORD wTempo);
    WORD GetTempo() const { return _wTempo; }

    void SetActiveCue(int index);
    int GetActiveCue() const { return _cueIndex; }
    void v_OnUndoRedo();

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
    void afx_msg _OnImportMidi();
    virtual ResourceType _GetType() const { return ResourceType::Sound; }

	DECLARE_MESSAGE_MAP()

private:
    // Assumes ownership of this:
    WORD _wTempo;
    int _cueIndex;
    DeviceType _device;

    static DeviceType s_defaultDevice;
};

std::unique_ptr<ResourceEntity> ImportMidi();
