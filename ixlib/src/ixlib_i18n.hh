// ----------------------------------------------------------------------------
//  Description      : ixlib internationalization wrapper
// ----------------------------------------------------------------------------
//  (c) Copyright 2001 by iXiONmedia, all rights reserved.
// ----------------------------------------------------------------------------

#ifndef IXLIB_I18N

#include <string>
#ifdef _MSC_VER
#define _(String) (String)
#else
#include <libintl.h>
#define _(String) gettext(String)
#endif

#define N_(String) (String)

#endif
