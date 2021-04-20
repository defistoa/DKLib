
//ExcelImport.h

#pragma once

#import "C:\Program Files (x86)\Common Files\Microsoft Shared\Office15\mso.dll" rename("RGB", "MSRGB")

#import "C:\Program Files (x86)\Common Files\microsoft shared\VBA\VBA6\VBE6EXT.OLB" rename("Reference", "VBRef"), rename("VBE", "_VBE")

#import "C:\Program Files (x86)\Microsoft Office\Office15\excel.exe" \
	rename("IFont", "IXLFont") \
	rename("IPicture", "IXLPicture") \
	rename("RGB", "MSRGB") \
	rename("DialogBox", "XLDialogBox") \
	rename("VBE", "_VBE") \
	rename("ReplaceText", "XLReplaceText") \
	rename("CopyFile","XLCopyFile") \
	rename("FindText", "XLFindText") \
	rename("NoPrompt", "XLNoPrompt") \
	named_guids 
