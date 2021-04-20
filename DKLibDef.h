#pragma once

#ifndef DK_API
	#ifdef _DKLIB_IMPL
		#define DK_API _declspec(dllexport)
	#else
		#define DK_API _declspec(dllimport)
	#endif
#endif
