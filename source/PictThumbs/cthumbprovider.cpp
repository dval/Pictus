#include "cthumbprovider.h"
#include "illa/codecmgr.h"
#include "illa/surfacemgr.h"
#include "orz/Win32/stream_windows.h"
#include "illa/swsurface.h"
#include "illa/config.h"
#include "illa/render.h"
#include "illa/filter.h"
#include "codecsetup.h"

#include <Shlwapi.h>

using namespace Img;
using namespace Geom;

CPictusThumbnailProvider::CPictusThumbnailProvider():m_cRef(1) {
	CodecManagerSetup(&m_cfs);
}

CPictusThumbnailProvider::~CPictusThumbnailProvider() {}

IFACEMETHODIMP CPictusThumbnailProvider::QueryInterface(REFIID riid, __deref_out void **ppv) {
	static const QITAB qit[] =  {
		QITABENT(CPictusThumbnailProvider, IInitializeWithStream),
		QITABENT(CPictusThumbnailProvider, IThumbnailProvider),
		{ 0 },
	};
	return QISearch(this, qit, riid, ppv);
}

IFACEMETHODIMP_(ULONG) CPictusThumbnailProvider::AddRef() {
	return InterlockedIncrement(&m_cRef);
}

IFACEMETHODIMP_(ULONG) CPictusThumbnailProvider::Release() {
	ULONG cRef = InterlockedDecrement(&m_cRef);
	if (!cRef) {
		delete this;
	}

	return cRef;
}

IFACEMETHODIMP CPictusThumbnailProvider::Initialize(_In_ IStream *pStream, _In_ DWORD grfMode) {
	if(m_reader)
		return E_UNEXPECTED;
	if(!pStream)
		return E_INVALIDARG;

	IStream* stream;
	HRESULT ret = pStream->QueryInterface(IID_PPV_ARGS(&stream));
	if(!SUCCEEDED(ret))
		return ret;

	IO::Stream::Ptr winStream(new IO::StreamWindows(stream));
	m_reader.reset(new IO::FileReader(winStream));

	return S_OK;
}

struct DimData {
	SizeInt sz;
	float scale;
};

DimData DetermineDimensions(UINT cx, Geom::SizeInt surfDims) {
	DimData d;
	Geom::SizeFloat factors = (float)cx / surfDims.StaticCast<float>();
	d.scale = std::min(factors.Width, std::min(factors.Height, 1.0f));
	d.sz = (surfDims * d.scale).StaticCast<int>();
	return d;
}

// TODO: Figure out why XYZ doesn't work. Doesn't SEEM to be registry related, so that leaves GetThumbnail.
// IThumbnailProvider
_Use_decl_annotations_ IFACEMETHODIMP CPictusThumbnailProvider::GetThumbnail(UINT cx, HBITMAP *phbmp, WTS_ALPHATYPE *pdwAlpha) {
	try {
		if (cx == 0) {
			Log << "(Thumb) Argument cx invalid.\n";
			return E_INVALIDARG;
		}

		if (phbmp == 0) {
			Log << "(Thumb) Argument phbmp invalid.\n";
			return E_POINTER;
		}

		if (pdwAlpha != 0) {
			*pdwAlpha = WTSAT_UNKNOWN;
		}

		if (!m_reader) {
			Log << "(Thumb) Invalid state, stream not yet initialized.";
			return E_ILLEGAL_METHOD_CALL;
		}

		Img::SurfaceFactory(new FactorySurfaceSoftware);
		Img::CodecFactoryStore cfs;
		CodecManagerSetup(&cfs);

		Img::Surface::Ptr s = LoadSurface(cx);
		if(!s) {
			Log << "(Thumb) Failed to load image.\n";
			return E_NOTIMPL;
		}

		DimData outDims = DetermineDimensions(cx, s->GetSize());
		if(!IsPositive(s->GetSize()) || !IsPositive(outDims.sz)) {
			Log << "(Thumb) Image or thumbnail had non-positive image dimensions.\n";
			return E_NOTIMPL;
		}
		Log << "(Thumb) Loaded image, dims:" << s->GetSize() << "\n";

		BITMAPINFO bmi = {};
		bmi.bmiHeader.biSize = sizeof(bmi.bmiHeader);
		bmi.bmiHeader.biWidth = outDims.sz.Width;
		bmi.bmiHeader.biHeight = -static_cast<LONG>(outDims.sz.Height);
		bmi.bmiHeader.biPlanes = 1;
		bmi.bmiHeader.biBitCount = 32;
		bmi.bmiHeader.biCompression = BI_RGB;

		uint8_t* pBits;
		*phbmp = CreateDIBSection(0, &bmi, DIB_RGB_COLORS, reinterpret_cast<void **>(&pBits), NULL, 0);
		if(*phbmp == 0) {
			Log << "PictThumb: Failed to create DIB section.";
			return E_OUTOFMEMORY;
		}
		if (*phbmp == reinterpret_cast<HBITMAP>(ERROR_INVALID_PARAMETER)) {
			*phbmp = 0;
			Log << "(Thumb) Failed to create DIB section.\n";
			return E_UNEXPECTED;
		}

		Filter::FilterBuffer dst(outDims.sz, 4, pBits, outDims.sz.Width * 4);

		Img::FilterBufferAndLock src = GenerateFilterBuffer(s);

		Img::Properties props;
		props.ResampleFilter = Filter::Mode::Lanczos3;
		props.Zoom = outDims.scale; 
		props.RetainAlpha = true;

		Img::RenderToBuffer(dst, src.filterBuffer, s->GetFormat(), RectInt(PointInt(0, 0), s->GetSize()), props);
		if (HasAlpha(s->GetFormat())) {
			Filter::Alpha::PremultiplyAlphaBuffer(dst);
		}
		else {
			Filter::Alpha::SetAlpha(dst, 0xff);
		}

		*pdwAlpha = (HasAlpha(s->GetFormat()))?WTSAT_ARGB:WTSAT_RGB;

		return S_OK;
	}
	catch(std::exception& e) {
		Log << "PictThumb: " << e.what() << "\n";
	}
	catch (...) { 
		Log << "Unknown exception encountered.\n";
	}
	return E_UNEXPECTED;
}

AbstractCodec* CPictusThumbnailProvider::FindCodec() {
	const Img::CodecFactoryStore::InfoVector& iv = m_cfs.CodecInfo();

	for(size_t i = 0; i < iv.size(); ++i) {
		AbstractCodec* c = m_cfs.CreateCodec(i);
		if (c == 0) {
			continue;
		}

		m_reader->Seek(0, IO::SeekMethod::Begin);
		if (!c->CanDetectFormat() || !c->LoadHeader(m_reader)) {
			delete c;
		}
		else {
			return c;
		}
	}
	Log << "(Thumb) Could not find a valid decoder.\n";
	return 0;
}

Img::Surface::Ptr CPictusThumbnailProvider::LoadSurface(UINT cx) {
	AbstractCodec* c = FindCodec();
	if (c == 0) {
		Log << "(Thumb) No codec is available. Can't load image.\n";
		return Img::Surface::Ptr();
	}
	if (c->Allocate(SizeInt(cx, cx)) == Img::AbstractCodec::AllocationStatus::NotSupported) {
		Log << "(Thumb) Specified image size not supported, will attempt full-size instead.\n";
		if (c->Allocate() != Img::AbstractCodec::AllocationStatus::Ok) {
			return Img::Surface::Ptr();
		}
	}

	if (c->LoadImageData() == AbstractCodec::LoadStatus::Failed) {
		Log << "(Thumb) LoadImageData failed.\n";
		return Img::Surface::Ptr();
	}

	Img::ImageComposer::Ptr cmp = c->RequestImageComposer();
	return cmp->RequestCurrentSurface();
}


HRESULT CPictusThumbnailProvider_CreateInstance(REFIID riid, __deref_out void **ppv) {
	try {
		auto *pNew = new CPictusThumbnailProvider();
		auto hr = pNew->QueryInterface(riid, ppv);
		pNew->Release();

		return hr;
	}
	catch (std::bad_alloc&) {
		return E_OUTOFMEMORY;
	}
}
