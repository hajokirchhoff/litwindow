#ifndef _PRECOMPILED_H_060318LWL
#define _PRECOMPILED_H_060318LWL

#if defined(_MSC_VER)
#pragma once

// tell MS VC8 to stop issuing tons of (well meaning, but stupid) warnings
#define _CRT_SECURE_NO_DEPRECATE
#define _CRT_NONSTDC_NO_DEPRECATE

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

#ifdef _DEBUG
#define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
#include <crtdbg.h>
#else
#define DEBUG_NEW new
#endif



#endif

#endif
