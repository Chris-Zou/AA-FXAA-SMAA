#pragma once

#include <assert.h>
#define SAFE_RELEASE(x) if(x != nullptr){ x->Release(); x = nullptr;}
#define GET_X_LPARAM(lp)                        ((int)(short)LOWORD(lp))
#define GET_Y_LPARAM(lp)                        ((int)(short)HIWORD(lp))