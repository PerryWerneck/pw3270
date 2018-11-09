/* IShellLinkDataList, missing from mingw's <shlobj.h>. */

LIB3270_INTERNAL const GUID IID_IShellLinkDataList;

#define INTERFACE IShellLinkDataList
DECLARE_INTERFACE_(IShellLinkDataList, IUnknown)
{
	STDMETHOD(QueryInterface)(THIS_ REFIID,PVOID*) PURE;
	STDMETHOD_(ULONG,AddRef)(THIS) PURE;
	STDMETHOD_(ULONG,Release)(THIS) PURE;
	STDMETHOD(AddDataBlock)(THIS_ VOID*) PURE;
	STDMETHOD(CopyDataBlock)(THIS_ DWORD,VOID**) PURE;
	STDMETHOD(RemoveDataBlock)(THIS_ DWORD) PURE;
	STDMETHOD(GetFlags)(THIS_ DWORD*) PURE;
	STDMETHOD(SetFlags)(THIS_ DWORD) PURE;
};
#undef INTERFACE

typedef struct tagDATABLOCKHEADER {
	DWORD	cbSize;
	DWORD	dwSignature;
} DATABLOCK_HEADER, *LPDATABLOCK_HEADER, *LPDBLIST;

typedef struct {
	DATABLOCK_HEADER dbh;
	WORD	wFillAttribute;
	WORD	wPopupFillAttribute;
	COORD	dwScreenBufferSize;
	COORD	dwWindowSize;
	COORD	dwWindowOrigin;
	DWORD	nFont;
	DWORD	nInputBufferSize;
	COORD	dwFontSize;
	UINT	uFontFamily;
	UINT	uFontWeight;
	WCHAR	FaceName[LF_FACESIZE];
	UINT	uCursorSize;
	BOOL	bFullScreen;
	BOOL	bQuickEdit;
	BOOL	bInsertMode;
	BOOL	bAutoPosition;
	UINT	uHistoryBufferSize;
	UINT	uNumberOfHistoryBuffers;
	BOOL	bHistoryNoDup;
	COLORREF ColorTable[16];
} NT_CONSOLE_PROPS, *LPNT_CONSOLE_PROPS;

#define NT_CONSOLE_PROPS_SIG 0xA0000002
