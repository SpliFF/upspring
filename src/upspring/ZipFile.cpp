//-----------------------------------------------------------------------
//  Upspring model editor
//  Copyright 2005 Jelmer Cnossen
//  This code is released under GPL license, see LICENSE.HTML for info.
//-----------------------------------------------------------------------

#include "EditorIncl.h"
#include "ZipFile.h"

#include <zlib.h>


// --------------------------------------------------------------------------
// file:        ZipReader.cpp
//
// Purpose:     The implementation of a quick'n dirty ZIP file reader class.
//              (C) Copyright 2000 Javier Arevalo. Use and modify as you like
//              Get zlib from http://www.cdrom.com/pub/infozip/zlib/
// --------------------------------------------------------------------------

// --------------------------------------------------------------------------
// Basic types.
// --------------------------------------------------------------------------
typedef unsigned long dword;
typedef unsigned short word;
typedef unsigned char byte;

typedef unsigned char Bytef;

// --------------------------------------------------------------------------
// ZIP file structures. Note these have to be packed.
// --------------------------------------------------------------------------

#pragma pack(2)
// --------------------------------------------------------------------------
// --------------------------------------------------------------------------
struct ZipFile::TZipLocalHeader
{
  enum
  {
    SIGNATURE = 0x04034b50,
    COMP_STORE  = 0,
    COMP_DEFLAT = 8,
  };
  dword   sig;
  word    version;
  word    flag;
  word    compression;      // COMP_xxxx
  word    modTime;
  word    modDate;
  dword   crc32;
  dword   cSize;
  dword   ucSize;
  word    fnameLen;         // filename string follows header.
  word    xtraLen;          // Extra field follows filename.
};

// --------------------------------------------------------------------------
// --------------------------------------------------------------------------
struct ZipFile::TZipDirHeader
{
  enum
  {
    SIGNATURE = 0x06054b50,
  };
  dword   sig;
  word    nDisk;
  word    nStartDisk;
  word    nDirEntries;
  word    totalDirEntries;
  dword   dirSize;
  dword   dirOffset;
  word    cmntLen;
};

// --------------------------------------------------------------------------
// --------------------------------------------------------------------------
struct ZipFile::TZipDirFileHeader
{
  enum
  {
    SIGNATURE   = 0x02014b50,
    COMP_STORE  = 0,
    COMP_DEFLAT = 8,
  };
  dword   sig;
  word    verMade;
  word    verNeeded;
  word    flag;
  word    compression;      // COMP_xxxx
  word    modTime;
  word    modDate;
  dword   crc32;
  dword   cSize;            // Compressed size
  dword   ucSize;           // Uncompressed size
  word    fnameLen;         // filename string follows header.
  word    xtraLen;          // Extra field follows filename.
  word    cmntLen;          // Comment field follows extra field.
  word    diskStart;
  word    intAttr;
  dword   extAttr;
  dword   hdrOffset;

  char *GetName   () const { return (char *)(this + 1);   }
  char *GetExtra  () const { return GetName() + fnameLen; }
  char *GetComment() const { return GetExtra() + xtraLen; }
};

#pragma pack()

// --------------------------------------------------------------------------
// Function:      Init
// Purpose:       Initialize the object and read the zip file directory.
// Parameters:    A stdio FILE* used for reading.
// --------------------------------------------------------------------------
TError ZipFile::Init(FILE *f)
{
  End();
  if (f == NULL)
    return RET_FAIL;

  // Assuming no extra comment at the end, read the whole end record.
  TZipDirHeader dh;

  fseek(f, -(int)sizeof(dh), SEEK_END);
  long dhOffset = ftell(f);
  memset(&dh, 0, sizeof(dh));
  if (fread(&dh, sizeof(dh), 1, f)) {}

  // Check
  if (dh.sig != TZipDirHeader::SIGNATURE)
    return RET_FAIL;

  // Go to the beginning of the directory.
  fseek(f, dhOffset - dh.dirSize, SEEK_SET);

  // Allocate the data buffer, and read the whole thing.
  m_pDirData = new char[dh.dirSize + dh.nDirEntries*sizeof(*m_papDir)];
  if (!m_pDirData)
    return RET_FAIL;
  memset(m_pDirData, 0, dh.dirSize + dh.nDirEntries*sizeof(*m_papDir));
  if (fread(m_pDirData, dh.dirSize, 1, f)) {}

  // Now process each entry.
  char *pfh = m_pDirData;
  m_papDir = (const TZipDirFileHeader **)(m_pDirData + dh.dirSize);

  TError ret = RET_OK;

  for (int i = 0; i < dh.nDirEntries && ret == RET_OK; i++)
  {
    TZipDirFileHeader &fh = *(TZipDirFileHeader*)pfh;

    // Store the address of nth file for quicker access.
    m_papDir[i] = &fh;

    // Check the directory entry integrity.
    if (fh.sig != TZipDirFileHeader::SIGNATURE)
      ret = RET_FAIL;
    else
    {
      pfh += sizeof(fh);

      // Convert UNIX slashes to DOS backlashes.
      for (int j = 0; j < fh.fnameLen; j++)
        if (pfh[j] == '/')
          pfh[j] = '\\';

      // Skip name, extra and comment fields.
      pfh += fh.fnameLen + fh.xtraLen + fh.cmntLen;
    }
  }
  if (ret != RET_OK)
    delete[] m_pDirData;
  else
  {
    m_nEntries = dh.nDirEntries;
    m_f = f;
  }

  return ret;
}

// --------------------------------------------------------------------------
// Function:      End
// Purpose:       Finish the object
// Parameters:    
// --------------------------------------------------------------------------
void ZipFile::End()
{
  if (IsOk())
  {
    delete[] m_pDirData;
    m_nEntries = 0;
  }
}

// --------------------------------------------------------------------------
// Function:      GetFilename
// Purpose:       Return the name of a file
// Parameters:    The file index and the buffer where to store the filename
// --------------------------------------------------------------------------
void ZipFile::GetFilename(int i, char *pszDest, int Max)  const
{
  if (pszDest != NULL)
  {
    if (i < 0 || i >= m_nEntries)
      *pszDest = '\0';
    else
    {
		int m=m_papDir[i]->fnameLen;
		if (Max<m) m=Max;

		//strncpy (pszDest, m_papDir[i]->GetName(), MIN(Max, m_papDir[i]->fnameLen));
		memcpy(pszDest, m_papDir[i]->GetName(),m );
		pszDest[m] = '\0';
    }
  }
}

// --------------------------------------------------------------------------
// Function:      GetFileLen
// Purpose:       Return the length of a file so a buffer can be allocated
// Parameters:    The file index.
// --------------------------------------------------------------------------
int ZipFile::GetFileLen(int i) const
{
  if (i < 0 || i >= m_nEntries)
    return -1;
  else
    return m_papDir[i]->ucSize;
}

// --------------------------------------------------------------------------
// Function:      ReadFile
// Purpose:       Uncompress a complete file
// Parameters:    The file index and the pre-allocated buffer
// --------------------------------------------------------------------------
TError ZipFile::ReadFile(int i, void *pBuf)
{
  if (pBuf == NULL || i < 0 || i >= m_nEntries)
    return RET_FAIL;

  // Quick'n dirty read, the whole file at once.
  // Ungood if the ZIP has huge files inside

  // Go to the actual file and read the local header.
  fseek(m_f, m_papDir[i]->hdrOffset, SEEK_SET);
  TZipLocalHeader h;

  memset(&h, 0, sizeof(h));
  if (fread(&h, sizeof(h), 1, m_f)) {}
  if (h.sig != TZipLocalHeader::SIGNATURE)
    return RET_FAIL;

  // Skip extra fields
  fseek(m_f, h.fnameLen + h.xtraLen, SEEK_CUR);

  if (h.compression == TZipLocalHeader::COMP_STORE)
  {
    // Simply read in raw stored data.
    if (fread(pBuf, h.cSize, 1, m_f)) {}
    return RET_OK;
  }
  else if (h.compression != TZipLocalHeader::COMP_DEFLAT)
    return RET_FAIL;

  // Alloc compressed data buffer and read the whole stream
  char *pcData = new char[h.cSize];
  if (!pcData)
    return RET_FAIL;

  memset(pcData, 0, h.cSize);
  if (fread(pcData, h.cSize, 1, m_f)) {}

  TError ret = RET_OK;

  // Setup the inflate stream.
  z_stream stream;
  int err;

  stream.next_in = (Bytef*)pcData;
  stream.avail_in = (uInt)h.cSize;
  stream.next_out = (Bytef*)pBuf;
  stream.avail_out = h.ucSize;
  stream.zalloc = (alloc_func)0;
  stream.zfree = (free_func)0;

  // Perform inflation. wbits < 0 indicates no zlib header inside the data.
  err = inflateInit2(&stream, -MAX_WBITS);
  if (err == Z_OK)
  {
    err = inflate(&stream, Z_FINISH);
    inflateEnd(&stream);
    if (err == Z_STREAM_END)
      err = Z_OK;
    inflateEnd(&stream);
  }
  if (err != Z_OK)
    ret = RET_FAIL;

  delete[] pcData;
  return ret;
}
