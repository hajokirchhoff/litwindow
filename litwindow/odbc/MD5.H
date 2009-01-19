/* MD5 DLL header file, (c) 1994 Andrew Brown */
/* modified and ported to Win32 by Kirchhoff Consulting */

#ifndef __MD5_H__
#define __MD5_H__


#define LPBYTE unsigned char*
#ifndef DWORD
#define DWORD unsigned __int32
#endif

void  MD5(const LPBYTE data,int bytes,LPBYTE hash);
void  MD5Init (void);
void  MD5Update (const LPBYTE inBuf,DWORD inLen);
void  MD5Final (void);
void  MD5GetHash(LPBYTE hash);

inline void get_md5(const void *data, int bytes, unsigned char hash[16])
{
    MD5(reinterpret_cast<const LPBYTE>(data), bytes, hash);
}

#endif