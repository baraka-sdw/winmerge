#include "IListCtrl.h"
#include <commctrl.h>
#include <atlimage.h>

class IListCtrlImpl : public IListCtrl
{
public:
	IListCtrlImpl(HWND hwndListCtrl) : m_hwndListCtrl(hwndListCtrl)
	{
	}

	int GetColumnCount() const
	{
		return Header_GetItemCount(ListView_GetHeader(m_hwndListCtrl));
	}

	int GetRowCount() const
	{
		return ListView_GetItemCount(m_hwndListCtrl);
	}

	String GetColumnName(int col) const
	{
		LVCOLUMN lvc;
		TCHAR columnName[256]; // Assuming max col header will never be > 256
		lvc.mask = LVCF_TEXT;
		lvc.pszText = &columnName[0];
		lvc.cchTextMax = sizeof(columnName) / sizeof(TCHAR);
		ListView_GetColumn(m_hwndListCtrl, col, &lvc);
		return lvc.pszText;
	}

	String GetItemText(int row, int col) const
	{
		TCHAR text[512]; // Assuming max col header will never be > 512
		ListView_GetItemText(m_hwndListCtrl, row, col, text, sizeof(text)/sizeof(text[0]));
		return text;
	}

	void *GetItemData(int row) const
	{
		LVITEM lvi = {0};
		lvi.iItem = row;
		lvi.mask = LVIF_PARAM;
		::SendMessage(m_hwndListCtrl, LVM_GETITEM, 0, (LPARAM)&lvi);
		return (void *)lvi.lParam;
	}

	int GetBackColor(int row) const
	{
		NMLVCUSTOMDRAW nmlvcd = {0};
		nmlvcd.nmcd.hdr.code = NM_CUSTOMDRAW;
		nmlvcd.nmcd.hdr.idFrom = GetDlgCtrlID(m_hwndListCtrl);
		nmlvcd.nmcd.hdr.hwndFrom = m_hwndListCtrl;
		nmlvcd.nmcd.dwDrawStage = CDDS_ITEMPREPAINT | CDDS_SUBITEM;
		nmlvcd.nmcd.dwItemSpec = row;
		SendMessage(GetParent(m_hwndListCtrl), WM_NOTIFY, (WPARAM)m_hwndListCtrl, (LPARAM)&nmlvcd);
		return nmlvcd.clrTextBk;
	}

	bool IsSelectedItem(int sel) const
	{
		return !!ListView_GetItemState(m_hwndListCtrl, sel, LVIS_SELECTED);
	}

	int GetNextItem(int sel, bool selected = false, bool reverse = false) const
	{
		return ListView_GetNextItem(m_hwndListCtrl, sel, (selected ? LVNI_SELECTED : 0) | (reverse ? LVNI_ABOVE : 0));		
	}

	int GetNextSelectedItem(int sel, bool reverse = false) const
	{
		return ListView_GetNextItem(m_hwndListCtrl, sel, LVNI_SELECTED | (reverse ? LVNI_ABOVE : 0));		
	}

	unsigned GetSelectedCount() const
	{
		return ListView_GetSelectedCount(m_hwndListCtrl);
	}

	int GetIndent(int row) const
	{
		LVITEM item = {0};
		item.mask = LVIF_INDENT;
		item.iItem = row;
		ListView_GetItem(m_hwndListCtrl, &item);
		return item.iIndent;
	}

	int GetIconIndex(int row) const
	{
		LVITEM item = {0};
		item.mask = LVIF_IMAGE;
		item.iItem = row;
		ListView_GetItem(m_hwndListCtrl, &item);
		return item.iImage;
	}

	int GetIconCount() const
	{
		HIMAGELIST hImageList = ListView_GetImageList(m_hwndListCtrl, LVSIL_SMALL);
		return ImageList_GetImageCount(hImageList);
	}

	std::string GetIconPNGData(int iconIndex) const
	{
		HIMAGELIST hImageList = ListView_GetImageList(m_hwndListCtrl, LVSIL_SMALL);
		IMAGEINFO imageInfo = {0};
		LARGE_INTEGER li = {0};
		IStream *pStream = NULL;
		CImage image;

		ImageList_GetImageInfo(hImageList, iconIndex, &imageInfo);
		int w = imageInfo.rcImage.right - imageInfo.rcImage.left;
		int h = imageInfo.rcImage.bottom - imageInfo.rcImage.top;
		CreateStreamOnHGlobal(NULL, TRUE, &pStream);
		HDC hdcMem = CreateCompatibleDC(NULL);
		BITMAPINFO bmpinfo = {0};
		bmpinfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		bmpinfo.bmiHeader.biWidth = w;
		bmpinfo.bmiHeader.biHeight = h;
		bmpinfo.bmiHeader.biPlanes = 1;
		bmpinfo.bmiHeader.biBitCount = 32;
		bmpinfo.bmiHeader.biCompression = BI_RGB;
		HBITMAP hbmpImage = CreateDIBSection( NULL, &bmpinfo, DIB_RGB_COLORS, NULL, NULL, NULL);
		HBITMAP hbmpOld = (HBITMAP)SelectObject(hdcMem, hbmpImage);
		ImageList_Draw(hImageList, iconIndex, hdcMem, 0, 0, ILD_TRANSPARENT);
		image.Attach(hbmpImage);
#if _MSC_VER >= 1400
		image.SetHasAlphaChannel(true);
#endif
		image.Save(pStream, Gdiplus::ImageFormatPNG);
		STATSTG stat;
		pStream->Stat(&stat, STATFLAG_NONAME);
		std::string ret(stat.cbSize.LowPart, 0);
		pStream->Seek(li, STREAM_SEEK_SET, NULL);
		pStream->Read(&ret[0], stat.cbSize.LowPart, NULL);
		pStream->Release();

		SelectObject(hdcMem, hbmpOld);
		DeleteObject(hdcMem);
		DeleteObject(hbmpImage);
		return ret;
	}

protected:
	HWND m_hwndListCtrl;
};