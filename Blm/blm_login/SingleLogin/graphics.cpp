#include "graphics.h"
Gdiplus::Bitmap* BitmapFromResource(HINSTANCE hInstance,
									LPCTSTR szResName, LPCTSTR szResType)
{
	Gdiplus::Bitmap* m_pBitmap = NULL; 
	HRSRC hResource = ::FindResource(hInstance, szResName, szResType);
	if (!hResource)
		return 0;

	DWORD imageSize = ::SizeofResource(hInstance, hResource);
	if (!imageSize)
		return 0;

	const void* pResourceData = ::LockResource(::LoadResource(hInstance, 
		hResource));
	if (!pResourceData)
		return 0;

	HGLOBAL m_hBuffer  = ::GlobalAlloc(GMEM_MOVEABLE, imageSize);
	if (m_hBuffer)
	{
		void* pBuffer = ::GlobalLock(m_hBuffer);
		if (pBuffer)
		{
			CopyMemory(pBuffer, pResourceData, imageSize);

			IStream* pStream = NULL;
			if (::CreateStreamOnHGlobal(m_hBuffer, FALSE, &pStream) == S_OK)
			{
				Gdiplus::Bitmap * tmpBitMap = Gdiplus::Bitmap::FromStream(pStream); 
				m_pBitmap = tmpBitMap->Clone(0,0,tmpBitMap->GetWidth(),tmpBitMap->GetHeight(),PixelFormatDontCare);
				delete tmpBitMap;
				pStream->Release();
				if (m_pBitmap)
				{ 
					if (m_pBitmap->GetLastStatus() == Gdiplus::Ok)
						return m_pBitmap;

					m_pBitmap = NULL;
				}
			}
			m_pBitmap = NULL;
			::GlobalUnlock(m_hBuffer);
		}
		::GlobalFree(m_hBuffer);
		m_hBuffer = NULL;
	}
	return m_pBitmap;
}