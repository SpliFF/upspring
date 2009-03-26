//-----------------------------------------------------------------------
//  Upspring model editor
//  Copyright 2005 Jelmer Cnossen
//  This code is released under GPL license, see LICENSE.HTML for info.
//-----------------------------------------------------------------------
#ifndef _ZIPFILE_H_
#define _ZIPFILE_H_

#include <stdio.h>

// A quick'n dirty ZIP file reader class.
// (C) Copyright 2000 Javier Arevalo. Use and modify as you like
// Get zlib from http://www.cdrom.com/pub/infozip/zlib/

enum TError
{
  RET_OK,
  RET_FAIL,
};


class ZipFile
{
  public:

    ZipFile    (): m_nEntries(0) { }
    ~ZipFile   ()                { End(); }

    TError  Init          (FILE *f);
    void    End           ();
    bool    IsOk          ()         const { return (m_nEntries != 0); }

    int     GetNumFiles   ()         const { return m_nEntries; }

    void    GetFilename   (int i, char *pszDest, int Max) const;
    int     GetFileLen    (int i) const;

    TError  ReadFile      (int i, void *pBuf);

  private:

    struct TZipDirHeader;
    struct TZipDirFileHeader;
    struct TZipLocalHeader;

    FILE                     *m_f;
    char                     *m_pDirData; // Raw data buffer.
    int                       m_nEntries; // Number of entries.

    // Pointers to the dir entries in pDirData.
    const TZipDirFileHeader **m_papDir;   
};

#endif // _ZIPFILE_H_
