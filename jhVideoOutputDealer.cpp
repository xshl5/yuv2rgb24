/*
 **************************************************************************************
 *                      CopyRight (C) jhsys Corp, Ltd.
 *
 *       Filename:  jhVideoOutputDealer.cpp
 *    Description:  source file
 *
 *        Version:  1.0
 *        Created:  Friday, June 16, 2012 10:15:10 CST
 *         Author:  dujf     [dujf@koti.cn]
 *
 *       Revision:  initial draft;
 **************************************************************************************
 */
/** Last updated by xshl5 2013-11-27 **/

#define LOG_NDEBUG 0
#define LOG_TAG "jhVideoOutputDealer"
#include <utils/Log.h>

#include "jhVideoOutputDealer.h"

static int init_dither_tab_flag = 0;
long int crv_tab[256];
long int cbu_tab[256];
long int cgu_tab[256];

long int cgv_tab[256];
long int tab_76309[256];
unsigned char clp[1024];

static inline void init_dither_tab()
{
    long int crv,cbu,cgu,cgv;
    int i,ind;

    crv = 104597; cbu = 132201;
    cgu = 25675;  cgv = 53279;

    for (i = 0; i < 256; i++) {
        crv_tab[i] = (i-128) * crv;
        cbu_tab[i] = (i-128) * cbu;
        cgu_tab[i] = (i-128) * cgu;
        cgv_tab[i] = (i-128) * cgv;
        tab_76309[i] = 76309*(i-16);
    }

    for (i=0; i<384; i++)
        clp[i] =0;
    ind=384;
    for (i=0;i<256; i++)
        clp[ind++]=i;
    ind=640;
    for (i=0;i<384;i++)
        clp[ind++]=255;
}

namespace jhsys{

jhVideoOutputDealer::jhVideoOutputDealer()
{
    if(init_dither_tab_flag == 0)
    {
        init_dither_tab();
        init_dither_tab_flag = 1;
    }
}

jhVideoOutputDealer::~jhVideoOutputDealer()
{
    ;
}

void jhVideoOutputDealer::yuv420pToRgb24(unsigned char * pSrcY, unsigned char * pSrcU, unsigned char * pSrcV,
                                                                                        int width, int height, unsigned char * pDst)
{
    int i, j;
    int nR, nG, nB;
    int nY, nU, nV;
    unsigned char *out = pDst;
    int offset = 0;
    unsigned char *pY = pSrcY;//pSrc;
    unsigned char *pV = pSrcV;//pY + width * height;
    unsigned char *pU = pSrcU;//pV + (width * height)/4;

    for (i = 0; i < height; i++)
    {
        for (j = 0; j < width; j++)
        {
            nY = *(pY + i * width + j);
            nU = *(pU + (i/4)* width + j/2);
            nV = *(pV +(i/4) * width + j/2);

            nY -= 16;
            nU -= 128;
            nV -= 128;

            if (nY < 0)
            {
                nY = 0;
            }

            nB = (int)(1192 * nY + 2066 * nU);
            nG = (int)(1192 * nY - 833 * nV - 400 * nU);
            nR = (int)(1192 * nY + 1634 * nV);

            nR = min(262143, max(0, nR));
            nG = min(262143, max(0, nG));
            nB = min(262143, max(0, nB));

            nR >>= 10; nR &= 0xff;
            nG >>= 10; nG &= 0xff;
            nB >>= 10; nB &= 0xff;

            out[offset++] = (unsigned char)nR;
            out[offset++] = (unsigned char)nG;
            out[offset++] = (unsigned char)nB;
        }
    }
}

void jhVideoOutputDealer::saveAsBmpFile(char * fileName, unsigned char * data, int W, int H)
{
    int size	= W*H*3;
    int stepSize = W*3;
    FILE *fp = NULL;
    int i;

    mFileHeader.bfType=0x4D42;
    mFileHeader.bfSize=size+sizeof(mFileHeader)+sizeof(mInfoHeader);
    mFileHeader.bfReserved1=0;
    mFileHeader.bfReserved2=0;
    mFileHeader.bfOffBits=sizeof(mFileHeader)+sizeof(mInfoHeader);

    mInfoHeader.biSize=sizeof(mInfoHeader);
    mInfoHeader.biWidth=W;
    mInfoHeader.biHeight= H;
    mInfoHeader.biPlanes=1;
    mInfoHeader.biBitCount=24;
    mInfoHeader.biCompression=0;
    mInfoHeader.biSizeImage=size;
    mInfoHeader.biXPelsPerMeter=0;
    mInfoHeader.biYPelsPerMeter=0;
    mInfoHeader.biClrUsed=0;
    mInfoHeader.biClrImportant=0;

    fp = fopen(fileName, "wb");
    if(fp < 0)
    {
        LOGE("open \"%s\" is error!", fileName);
        return;
    }

    fwrite(&mFileHeader,sizeof(mFileHeader),1,fp);
    fwrite(&mInfoHeader,sizeof(mInfoHeader),1,fp);

    for(i = 0;i < H;i++)
    {
        fwrite((data+(H-i-1)*W*3),stepSize,1,fp);
    }

    fclose(fp);
}

void jhVideoOutputDealer::yuv420pToRgb565Center(unsigned char * pSrcY, unsigned char * pSrcU, unsigned char * pSrcV,
                                                                                                int srcWidth, int srcHeight, __u16 * pDst, int dstWidth, int dstHeight, int stepSize)
{
    //LOGD("ENTER VideoUtil::yuv420pToRgb565Center()");
    int i, j;
    int nR, nG, nB;
    int nY, nU, nV;
    int mWdif = 0, mHdif = 0;
    __u16 *out = pDst;

    unsigned char *pY = pSrcY;//pSrc;
    unsigned char *pV = pSrcV;//pY + width * height;
    unsigned char *pU = pSrcU;//pV + (width * height)/4;

    mWdif = (srcWidth < stepSize) ? (stepSize-srcWidth) : 0;
    mHdif = (srcHeight < dstHeight) ? (dstHeight-srcHeight) : 0;

    out += (mHdif/2) * stepSize + (mWdif/2) - (stepSize-dstWidth)/2;

    for (i = 0; i < dstHeight-mHdif; i++)
    {
        for (j = 0; j < stepSize-mWdif; j++)
        {
            nY = *(pY + i * srcWidth + j);
            nU = *(pU + (i/4)* srcWidth + j/2);
            nV = *(pV +(i/4) * srcWidth + j/2);

            nY -= 16;
            nU -= 128;
            nV -= 128;

            if (nY < 0)
            {
                nY = 0;
            }

            nB = (int)(1192 * nY + 2066 * nU);
            nG = (int)(1192 * nY - 833 * nV - 400 * nU);
            nR = (int)(1192 * nY + 1634 * nV);

            nR = min(262143, max(0, nR));
            nG = min(262143, max(0, nG));
            nB = min(262143, max(0, nB));

            nR >>= 10; nR &= 0xff;
            nG >>= 10; nG &= 0xff;
            nB >>= 10; nB &= 0xff;

            *out++ = (((__u16)nR>>3)<<11) | (((__u16)nG>>2)<<5) | (((__u16)nB>>3)<<0);
        }
        out += mWdif;
    }
}

void jhVideoOutputDealer::yuv420pNearestNeighborScale(unsigned char * pSrcY, unsigned char * pSrcU, unsigned char * pSrcV,
                                                                                    int srcWidth, int srcHeight, __u16 * pDst, int dstWidth, int dstHeight, int stepSize)
{
    int nR = 0,nG = 0, nB = 0;
    double mXscale = 0,mYscale = 0;
    __u16 *out = pDst;

    mXscale = (double)srcWidth/(double)dstWidth;
    mYscale = (double)srcHeight/(double)dstHeight;

    double dYSrc = 0;
    int mWdif = stepSize - dstWidth;

    for (int nY = 0; nY < dstHeight; nY++)
    {
        double dXSrc = 0;
        for (int nX = 0; nX < dstWidth; nX++)
        {
            yuv420pCalcNearestNeighbor(pSrcY, pSrcU, pSrcV, srcWidth,srcHeight, dXSrc, dYSrc,nR,nG,nB);
            *out++ =(((__u16)nR>>3)<<11) | (((__u16)nG>>2)<<5) | (((__u16)nB>>3)<<0);
            dXSrc += mXscale;
        }

        out += mWdif;
        dYSrc += mYscale;
    }
}

void jhVideoOutputDealer::yuv420pCalcNearestNeighbor(unsigned char * pSrcY, unsigned char * pSrcU, unsigned char * pSrcV,
                                                                                    int Width, int Height, double dXSrc, double dYSrc, int & nR, int & nG, int & nB)
{
    int X = (int)dXSrc;
    int Y = (int)dYSrc;
    int srcWidth = Width;
    int srcHeight = Height;
    int nY, nU, nV;

    unsigned char *pY  = pSrcY;//pSrc;
    unsigned char *pV = pSrcV;//pY + Width * Height;
    unsigned char *pU = pSrcU;//pV + (Width * Height)/4;

    nY = *(pY + Y * Width + X);
    nU = *(pU + (Y/4)* Width + X/2);
    nV = *(pV +(Y/4) * Width + X/2);

    nY -= 16;
    nU -= 128;
    nV -= 128;

    if (nY < 0)
    {
        nY = 0;
    }

    nB = (int)(1192 * nY + 2066 * nU);
    nG = (int)(1192 * nY - 833 * nV - 400 * nU);
    nR = (int)(1192 * nY + 1634 * nV);

    nR = min(262143, max(0, nR));
    nG = min(262143, max(0, nG));
    nB = min(262143, max(0, nB));

    nR >>= 10; nR &= 0xff;
    nG >>= 10; nG &= 0xff;
    nB >>= 10; nB &= 0xff;
}

#if 1

const int Table_fv1[256]={ -180, -179, -177, -176, -174, -173, -172, -170, -169, -167, -166, -165, -163, -162, -160, -159, -158, -156, -155, -153, -152, -151, -149, -148, -146, -
145, -144, -142, -141, -139, -138, -137, -135, -134, -132, -131, -130, -128, -127, -125, -124, -123, -121, -120, -118, -117, -115, -114, -113, -111, -110, -108, -107, -106, -104
, -103, -101, -100, -99, -97, -96, -94, -93, -92, -90, -89, -87, -86, -85, -83, -82, -80, -79, -78, -76, -75, -73, -72, -71, -69, -68, -66, -65, -64, -62, -61, -59, -58, -57, -55
, -54, -52, -51, -50, -48, -47, -45, -44, -43, -41, -40, -38, -37, -36, -34, -33, -31, -30, -29, -27, -26, -24, -23, -22, -20, -19, -17, -16, -15, -13, -12, -10, -9, -8, -6, -5,
-3, -2, 0, 1, 2, 4, 5, 7, 8, 9, 11, 12, 14, 15, 16, 18, 19, 21, 22, 23, 25, 26, 28, 29, 30, 32, 33, 35, 36, 37, 39, 40, 42, 43, 44, 46, 47, 49, 50, 51, 53, 54, 56, 57, 58, 60, 61
, 63, 64, 65, 67, 68, 70, 71, 72, 74, 75, 77, 78, 79, 81, 82, 84, 85, 86, 88, 89, 91, 92, 93, 95, 96, 98, 99, 100, 102, 103, 105, 106, 107, 109, 110, 112, 113, 114, 116, 117, 119
, 120, 122, 123, 124, 126, 127, 129, 130, 131, 133, 134, 136, 137, 138, 140, 141, 143, 144, 145, 147, 148, 150, 151, 152, 154, 155, 157, 158, 159, 161, 162, 164, 165, 166, 168,
169, 171, 172, 173, 175, 176, 178 };

const int Table_fv2[256]={ -92, -91, -91, -90, -89, -88, -88, -87, -86, -86, -85, -84, -83, -83, -82, -81, -81, -80, -79, -78, -78, -77, -76, -76, -75, -74, -73, -73, -72, -71, -
71, -70, -69, -68, -68, -67, -66, -66, -65, -64, -63, -63, -62, -61, -61, -60, -59, -58, -58, -57, -56, -56, -55, -54, -53, -53, -52, -51, -51, -50, -49, -48, -48, -47, -46, -46
, -45, -44, -43, -43, -42, -41, -41, -40, -39, -38, -38, -37, -36, -36, -35, -34, -33, -33, -32, -31, -31, -30, -29, -28, -28, -27, -26, -26, -25, -24, -23, -23, -22, -21, -21, -
20, -19, -18, -18, -17, -16, -16, -15, -14, -13, -13, -12, -11, -11, -10, -9, -8, -8, -7, -6, -6, -5, -4, -3, -3, -2, -1, 0, 0, 1, 2, 2, 3, 4, 5, 5, 6, 7, 7, 8, 9, 10, 10, 11, 12
, 12, 13, 14, 15, 15, 16, 17, 17, 18, 19, 20, 20, 21, 22, 22, 23, 24, 25, 25, 26, 27, 27, 28, 29, 30, 30, 31, 32, 32, 33, 34, 35, 35, 36, 37, 37, 38, 39, 40, 40, 41, 42, 42, 43,
44, 45, 45, 46, 47, 47, 48, 49, 50, 50, 51, 52, 52, 53, 54, 55, 55, 56, 57, 57, 58, 59, 60, 60, 61, 62, 62, 63, 64, 65, 65, 66, 67, 67, 68, 69, 70, 70, 71, 72, 72, 73, 74, 75, 75
, 76, 77, 77, 78, 79, 80, 80, 81, 82, 82, 83, 84, 85, 85, 86, 87, 87, 88, 89, 90, 90 };

const int Table_fu1[256]={ -44, -44, -44, -43, -43, -43, -42, -42, -42, -41, -41, -41, -40, -40, -40, -39, -39, -39, -38, -38, -38, -37, -37, -37, -36, -36, -36, -35, -35, -35, -
34, -34, -33, -33, -33, -32, -32, -32, -31, -31, -31, -30, -30, -30, -29, -29, -29, -28, -28, -28, -27, -27, -27, -26, -26, -26, -25, -25, -25, -24, -24, -24, -23, -23, -22, -22
, -22, -21, -21, -21, -20, -20, -20, -19, -19, -19, -18, -18, -18, -17, -17, -17, -16, -16, -16, -15, -15, -15, -14, -14, -14, -13, -13, -13, -12, -12, -11, -11, -11, -10, -10, -
10, -9, -9, -9, -8, -8, -8, -7, -7, -7, -6, -6, -6, -5, -5, -5, -4, -4, -4, -3, -3, -3, -2, -2, -2, -1, -1, 0, 0, 0, 1, 1, 1, 2, 2, 2, 3, 3, 3, 4, 4, 4, 5, 5, 5, 6, 6, 6, 7, 7, 7
, 8, 8, 8, 9, 9, 9, 10, 10, 11, 11, 11, 12, 12, 12, 13, 13, 13, 14, 14, 14, 15, 15, 15, 16, 16, 16, 17, 17, 17, 18, 18, 18, 19, 19, 19, 20, 20, 20, 21, 21, 22, 22, 22, 23, 23, 23
, 24, 24, 24, 25, 25, 25, 26, 26, 26, 27, 27, 27, 28, 28, 28, 29, 29, 29, 30, 30, 30, 31, 31, 31, 32, 32, 33, 33, 33, 34, 34, 34, 35, 35, 35, 36, 36, 36, 37, 37, 37, 38, 38, 38,
39, 39, 39, 40, 40, 40, 41, 41, 41, 42, 42, 42, 43, 43 };

const int Table_fu2[256]={ -227, -226, -224, -222, -220, -219, -217, -215, -213, -212, -210, -208, -206, -204, -203, -201, -199, -197, -196, -194, -192, -190, -188, -187, -185, -
183, -181, -180, -178, -176, -174, -173, -171, -169, -167, -165, -164, -162, -160, -158, -157, -155, -153, -151, -149, -148, -146, -144, -142, -141, -139, -137, -135, -134, -132
, -130, -128, -126, -125, -123, -121, -119, -118, -116, -114, -112, -110, -109, -107, -105, -103, -102, -100, -98, -96, -94, -93, -91, -89, -87, -86, -84, -82, -80, -79, -77, -75
, -73, -71, -70, -68, -66, -64, -63, -61, -59, -57, -55, -54, -52, -50, -48, -47, -45, -43, -41, -40, -38, -36, -34, -32, -31, -29, -27, -25, -24, -22, -20, -18, -16, -15, -13, -
11, -9, -8, -6, -4, -2, 0, 1, 3, 5, 7, 8, 10, 12, 14, 15, 17, 19, 21, 23, 24, 26, 28, 30, 31, 33, 35, 37, 39, 40, 42, 44, 46, 47, 49, 51, 53, 54, 56, 58, 60, 62, 63, 65, 67, 69,
70, 72, 74, 76, 78, 79, 81, 83, 85, 86, 88, 90, 92, 93, 95, 97, 99, 101, 102, 104, 106, 108, 109, 111, 113, 115, 117, 118, 120, 122, 124, 125, 127, 129, 131, 133, 134, 136, 138,
140, 141, 143, 145, 147, 148, 150, 152, 154, 156, 157, 159, 161, 163, 164, 166, 168, 170, 172, 173, 175, 177, 179, 180, 182, 184, 186, 187, 189, 191, 193, 195, 196, 198, 200, 202
, 203, 205, 207, 209, 211, 212, 214, 216, 218, 219, 221, 223, 225 };


//////////////YUV420 TO RGB Formula/////////////////////////
//
//R = Y + 1.402 ( Cr - 128 )
//G = Y - 0.34414 ( Cb - 128 )  -  0.71414 ( Cr - 128 )
//B = Y + 1.772 ( Cb - 128 )
//
//change float  to int
//
//rdif = v + ( ( v  *  103 )  >>  8 );
//invgdif = ( ( u  *  88 )  >>  8 )  + ( ( v  *  183 )  >>  8 );
//bdif = u +(  ( u * 198 )  >>  8 );

//r = YUVdata[YPOS] + rdif;
//g = YUVdata[YPOS] - invgdif;
//b = YUVdata[YPOS] + bdif;
//
//
//////////////Look-up table translation//////////////////////

inline void jhVideoOutputDealer::YV12_to_RGB24 (unsigned char * pSrcY, unsigned char * pSrcU, unsigned char * pSrcV,
                                                                                        int Width, int Height, double dXSrc, double dYSrc, int & nR, int & nG, int & nB)

{
//get input video X and Y address
    int X = (int)dXSrc;
    int Y = (int)dYSrc;

    int nY, nU, nV;

    unsigned char *pY  = pSrcY;//pSrc;
    unsigned char *pV = pSrcV;//pY + Width * Height;
    unsigned char *pU = pSrcU;//pV + (Width * Height)/4;

//get the input video pixel 's Y U V value

    nY = *(pY + Y * Width + X);
    nU = *(pU + (Y/4)* Width + X/2);
    nV = *(pV +(Y/4) * Width + X/2);

    int rdif, invgdif, bdif;

    // search tables to get rdif invgdif and bidif
    rdif = Table_fv1[nV];    // fv1
    invgdif = Table_fu1[nU] + Table_fv2[nV]; // fu1+fv2
    bdif = Table_fu2[nU]; // fu2

    nR = nY + rdif;        // R
    nG = nY - invgdif;   // G
    nB = nY + bdif;       // B

    if(nR>255) nR=255;else if(nR<0) nR=0;
    if(nG>255) nG=255;else if(nG<0) nG=0;
    if(nB>255) nB=255;else if(nB<0) nB=0;
}

#endif



#if 0
void jhVideoOutputDealer::yuv420pNearestNeighborScaleCenter(unsigned char * pSrcY, unsigned char * pSrcU, unsigned char * pSrcV,
                                                                                                int srcWidth, int srcHeight, __u16 * pDst, int dstWidth, int dstHeight, int stepSize)
{
    int nR = 0,nG = 0, nB = 0;
    double mXscale = 0,mYscale = 0;
    int mWdif = 0, mHdif = 0;
    int scaledW =0, scaledH = 0;
    __u16 *out = pDst;

    getScaledSize(srcWidth, srcHeight, dstWidth, dstHeight, &scaledW, &scaledH);
    mXscale = (double)srcWidth/(double)scaledW;
    mYscale = (double)srcHeight/(double)scaledH;

    double dYSrc = 0;

    mWdif = (scaledW < stepSize) ? (stepSize-scaledW) : 0;
    mHdif = (scaledH < dstHeight) ? (dstHeight-scaledH) : 0;
    out += (mHdif/2) * stepSize + (mWdif/2) - (stepSize-dstWidth)/2;

    for (int nY = 0; nY < dstHeight-mHdif; nY++)
    {
        double dXSrc = 0;
        for (int nX = 0; nX < stepSize-mWdif; nX++)
        {
            yuv420pCalcNearestNeighbor(pSrcY, pSrcU, pSrcV, srcWidth,srcHeight, dXSrc, dYSrc,nR,nG,nB);
            *out++ =(((__u16)nR>>3)<<11) | (((__u16)nG>>2)<<5) | (((__u16)nB>>3)<<0);
            dXSrc += mXscale;
        }

        out += mWdif;
        dYSrc += mYscale;
    }
}

#else

void jhVideoOutputDealer::yuv420pNearestNeighborScaleCenter(unsigned char * pSrcY, unsigned char * pSrcU, unsigned char * pSrcV,
                                                                                                int srcWidth, int srcHeight, __u16 * pDst, int dstWidth, int dstHeight, int stepSize)
{
    int nR = 0,nG = 0, nB = 0;
    double mXscale = 0,mYscale = 0;
    int mWdif = 0, mHdif = 0;
    int scaledW =0, scaledH = 0;
    __u16 *out = pDst;
    double olddxsrc = 100000.0, olddYSrc = 100000.0;

    getScaledSize(srcWidth, srcHeight, dstWidth, dstHeight, &scaledW, &scaledH);
    mXscale = (double)srcWidth/(double)scaledW;
    mYscale = (double)srcHeight/(double)scaledH;

    double dYSrc = 0;

    mWdif = (scaledW < stepSize) ? (stepSize-scaledW) : 0;
    mHdif = (scaledH < dstHeight) ? (dstHeight-scaledH) : 0;
    out += (mHdif/2) * stepSize + (mWdif/2) - (stepSize-dstWidth)/2;

    __u16 out1[stepSize];
    for (int nY = 0; nY < dstHeight-mHdif; nY++)
    {
        double dXSrc = 0;

        //direct output
        if((int)olddYSrc == (int)dYSrc)
        {
            for (int nX = 0; nX < stepSize-mWdif; nX++)
            {
                *out++ = out1[nX];
            }

        }
        else
        {
            for (int nX = 0; nX < stepSize-mWdif; nX++)
            {
                //direct output
                if(((int)olddxsrc == (int)dXSrc)&&(nX != 0))
                {
                    out1[nX] = out1[nX-1];
                    *out++ = out1[nX];
                }
                else
                {
#if 0
                    yuv420pCalcNearestNeighbor(pSrcY, pSrcU, pSrcV, srcWidth,srcHeight, dXSrc, dYSrc,nR,nG,nB);
#else //Look-up Table
                    YV12_to_RGB24(pSrcY, pSrcU, pSrcV, srcWidth,srcHeight, dXSrc, dYSrc,nR,nG,nB);
#endif
                    out1[nX] = (((__u16)nR>>3)<<11) | (((__u16)nG>>2)<<5) | (((__u16)nB>>3)<<0);
                    *out++ = out1[nX];

                    //*out++ =(((__u16)nR>>3)<<11) | (((__u16)nG>>2)<<5) | (((__u16)nB>>3)<<0);
                    //out1[nX] = *out;
                }
                olddxsrc = dXSrc;
                dXSrc += mXscale;
            }
        }

        olddYSrc = dYSrc;
        out += mWdif;
        dYSrc += mYscale;
    }
}

/**************************************************************
 *
 * yuv420pNearestNeighborScaleCenter_v2 by pangqsh 2013/11/26
 * mailto: pangqsh@koti.cn, xshl5ster@gmail.com
 *
 **************************************************************/
 #define REMOVE_FLOAT_MULTI 1000000
void jhVideoOutputDealer::yuv420pNearestNeighborScaleCenter_v2(unsigned char * pSrcY, unsigned char * pSrcU, unsigned char * pSrcV,
                                                                                                int srcWidth, int srcHeight, __u16 * pDst, int dstWidth, int dstHeight, int stepSize)
{
    float mXscale = 0,mYscale = 0;
    int mXscale2=0,mYscale2=0;
    int mWdif = 0, mHdif = 0;
    int scaledW =0, scaledH = 0;
    __u16 *out = pDst;

    getScaledSize(srcWidth, srcHeight, dstWidth, dstHeight, &scaledW, &scaledH);
    mXscale = (float)scaledW/(float)srcWidth;
    mYscale = (float)scaledH/(float)srcHeight;
    mXscale2 = (int)(mXscale*REMOVE_FLOAT_MULTI);
    mYscale2 = (int)(mYscale*REMOVE_FLOAT_MULTI);

    mWdif = (scaledW < stepSize) ? (stepSize-scaledW) : 0;
    mHdif = (scaledH < dstHeight) ? (dstHeight-scaledH) : 0;
    out += (mHdif/2) * stepSize + (mWdif/2) - (stepSize-dstWidth)/2;
	int dXSrc = 0,dYSrc = 0;
    int y1,y2,u,v;
    unsigned char *py1,*py2;
    int i,j, c1, c2, c3, c4;
    unsigned char d1[3], d2[3];

    py1=pSrcY;
    py2=py1+srcWidth;
    __u16 out1[stepSize], out2[stepSize];
    int oi1 = -1, oi2 = -1, oj = -REMOVE_FLOAT_MULTI, oii = -REMOVE_FLOAT_MULTI,oj_max = (scaledH -1)*REMOVE_FLOAT_MULTI;
    int line_buf_size = (stepSize-mWdif) << 1;
    for (j = 0; j < srcHeight; j += 2) {

        oi1 = -1; oi2 = -1; dXSrc = 0; oii = -REMOVE_FLOAT_MULTI;
        for (i = 0; i < srcWidth; i += 2) {

            u = *pSrcU++;
            v = *pSrcV++;

            c1 = crv_tab[v];
            c2 = cgu_tab[u];
            c3 = cgv_tab[v];
            c4 = cbu_tab[u];

            //up-left
            y1 = tab_76309[*py1++];
            d1[0] = clp[384+((y1 + c1)>>16)];
            d1[1] = clp[384+((y1 - c2 - c3)>>16)];
            d1[2] = clp[384+((y1 + c4)>>16)];
//            if(d1[0] > 255) d1[0] = 255; else if(d1[0] < 0) d1[0] = 0;
//            if(d1[1] > 255) d1[1] = 255; else if(d1[1] < 0) d1[1] = 0;
//            if(d1[2] > 255) d1[2] = 255; else if(d1[2] < 0) d1[2] = 0;
            out1[++oi1] = (((__u16)d1[0]>>3)<<11) | (((__u16)d1[1]>>2)<<5) | (((__u16)d1[2]>>3));
            oii += REMOVE_FLOAT_MULTI;

            //down-left
            y2 = tab_76309[*py2++];
            d2[0] = clp[384+((y2 + c1)>>16)];
            d2[1] = clp[384+((y2 - c2 - c3)>>16)];
            d2[2] = clp[384+((y2 + c4)>>16)];
//            if(d2[0] > 255) d2[0] = 255; else if(d2[0] < 0) d2[0] = 0;
//            if(d2[1] > 255) d2[1] = 255; else if(d2[1] < 0) d2[1] = 0;
//            if(d2[2] > 255) d2[2] = 255; else if(d2[2] < 0) d2[2] = 0;
            out2[++oi2] = (((__u16)d2[0]>>3)<<11) | (((__u16)d2[1]>>2)<<5) | (((__u16)d2[2]>>3));

            dXSrc += mXscale2;
            if( (dXSrc-oii) > 0 )
            {
                out1[++oi1] = out1[oi1-1];
                out2[++oi2] = out2[oi2-1];
                oii += REMOVE_FLOAT_MULTI;
                if( (dXSrc-oii) > 0 )
                {
                    out1[++oi1] = out1[oi1-1];
                    out2[++oi2] = out2[oi2-1];
                    oii += REMOVE_FLOAT_MULTI;
                    if( (dXSrc-oii) > 0 )
                    {
                        out1[++oi1] = out1[oi1-1];
                        out2[++oi2] = out2[oi2-1];
                        oii += REMOVE_FLOAT_MULTI;
                    }
                }
            }

            //up-right
            y1 = tab_76309[*py1++];
            d1[0] = clp[384+((y1 + c1)>>16)];
            d1[1] = clp[384+((y1 - c2 - c3)>>16)];
            d1[2] = clp[384+((y1 + c4)>>16)];
//            if(d1[0] > 255) d1[0] = 255; else if(d1[0] < 0) d1[0] = 0;
//            if(d1[1] > 255) d1[1] = 255; else if(d1[1] < 0) d1[1] = 0;
//            if(d1[2] > 255) d1[2] = 255; else if(d1[2] < 0) d1[2] = 0;
            out1[++oi1] = (((__u16)d1[0]>>3)<<11) | (((__u16)d1[1]>>2)<<5) | (((__u16)d1[2]>>3));
            oii += REMOVE_FLOAT_MULTI;

            //down-right
            y2 = tab_76309[*py2++];
            d2[0] = clp[384+((y2 + c1)>>16)];
            d2[1] = clp[384+((y2 - c2 - c3)>>16)];
            d2[2] = clp[384+((y2 + c4)>>16)];
//            if(d2[0] > 255) d2[0] = 255; else if(d2[0] < 0) d2[0] = 0;
//            if(d2[1] > 255) d2[1] = 255; else if(d2[1] < 0) d2[1] = 0;
//            if(d2[2] > 255) d2[2] = 255; else if(d2[2] < 0) d2[2] = 0;
            out2[++oi2] = (((__u16)d2[0]>>3)<<11) | (((__u16)d2[1]>>2)<<5) | (((__u16)d2[2]>>3));

            dXSrc += mXscale2;
            if( (dXSrc-oii) > 0 )
            {
                out1[++oi1] = out1[oi1-1];
                out2[++oi2] = out2[oi2-1];
                oii += REMOVE_FLOAT_MULTI;
                if( (dXSrc-oii) > 0 )
                {
                    out1[++oi1] = out1[oi1-1];
                    out2[++oi2] = out2[oi2-1];
                    oii += REMOVE_FLOAT_MULTI;
                    if( (dXSrc-oii) > 0 )
                    {
                        out1[++oi1] = out1[oi1-1];
                        out2[++oi2] = out2[oi2-1];
                        oii += REMOVE_FLOAT_MULTI;
                    }
                }
            }
        }

        // copy out1 to out
		if(oj >= oj_max) return;
        memcpy(out, out1, line_buf_size);
        out += stepSize;
        oj += REMOVE_FLOAT_MULTI;

        dYSrc += mYscale2;
        if( (dYSrc-oj) > 0 )
        {
			if(oj >= oj_max) return;
            memcpy(out, out1, line_buf_size);
            out += stepSize;
            oj += REMOVE_FLOAT_MULTI;
            if( (dYSrc-oj) > 0 )
            {
				if(oj >= oj_max) return;
                memcpy(out, out1, line_buf_size);
                out += stepSize;
                oj += REMOVE_FLOAT_MULTI;
                if( (dYSrc-oj) > 0 )
                {
					if(oj >= oj_max) return;
                    memcpy(out, out1, line_buf_size);
                    out += stepSize;
                    oj += REMOVE_FLOAT_MULTI;
                }
            }
        }


        // copy out2 to out
		if(oj >= oj_max) return;
        memcpy(out, out2, line_buf_size);
        out += stepSize;
        oj += REMOVE_FLOAT_MULTI;

        dYSrc += mYscale2;
        if( (dYSrc-oj) > 0 )
        {
			if(oj >= oj_max) return;
            memcpy(out, out2, line_buf_size);
            out += stepSize;
            oj += REMOVE_FLOAT_MULTI;
            if( (dYSrc-oj) > 0 )
            {
				if(oj >= oj_max) return;
                memcpy(out, out2, line_buf_size);
                out += stepSize;
                oj += REMOVE_FLOAT_MULTI;
                if( (dYSrc-oj) > 0 )
                {
					if(oj >= oj_max) return;
                    memcpy(out, out2, line_buf_size);
                    out += stepSize;
                    oj += REMOVE_FLOAT_MULTI;
                }
            }
        }

        py1 += srcWidth;
        py2 += srcWidth;
    }
}

#endif

inline void jhVideoOutputDealer::getScaledSize(int srcWidth, int srcHeight, int dstWidth, int dstHeight, int * scaledWidth, int * scaledHeight)
{
    double scale = 0, w_tmp = 0, h_tmp = 0;

    if(srcWidth > 0 && srcHeight > 0 && dstWidth > 0 && dstHeight > 0)
    {
        scale = (double)srcWidth/(double)srcHeight;
        w_tmp = (double)dstHeight* scale;
        if(w_tmp <= dstWidth)
        {
            *scaledHeight = dstHeight;
            *scaledWidth = (int)w_tmp;
        }
        else
        {
            h_tmp = (double)dstWidth/scale;
            *scaledWidth = dstWidth;
            *scaledHeight = (int)h_tmp;
        }
    }
}

}//namespace jhsys
