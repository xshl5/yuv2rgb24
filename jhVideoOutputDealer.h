/*
 **************************************************************************************
 *                      CopyRight (C) jhsys Corp, Ltd.
 *
 *       Filename:  jhVideoOutputDealer.h
 *    Description:  header file
 *
 *        Version:  1.0
 *        Created:  Friday, June 16, 2012 10:15:10 CST
 *         Author:  dujf     [dujf@koti.cn]
 *
 *       Revision:  initial draft;
 **************************************************************************************
 */
 /** Last updated by xshl5 2013-11-27 **/

#ifndef JHSYS_VIDEO_OUTPUT_DEALER_H
#define JHSYS_VIDEO_OUTPUT_DEALER_H

namespace jhsys {

#ifndef max
#define max(a,b) ({typeof(a) _a = (a); typeof(b) _b = (b); _a > _b ? _a : _b; })
#define min(a,b) ({typeof(a) _a = (a); typeof(b) _b = (b); _a < _b ? _a : _b; })
#endif

#pragma pack (2)
struct BmpFileHeader
{
     unsigned short      	bfType;         /*2byte 'BM'*/
     unsigned long     	bfSize;         /* 4byte file size*/
     unsigned short      	bfReserved1;   /*2byte*/
     unsigned short      	bfReserved2;   /*2byte  */
     unsigned long       	bfOffBits;       /*4byte */
};
#pragma pack ()

struct BmpInfoHeader
{
     unsigned long          biSize;                 /* 4byte */
     unsigned long          biWidth;                /*/4byte */
     unsigned long          biHeight;                /*4 byte */
     unsigned short         biPlanes;                 /*2 byte*/
     unsigned short         biBitCount  ;          /* 2 byte*/
     unsigned long        	biCompression;      /*4 byte  */
     unsigned long        	biSizeImage;           /*4 byte  */
     unsigned long          biXPelsPerMeter;    /*4 byte */
     unsigned long          biYPelsPerMeter;    /*4 byte */
     unsigned long        	biClrUsed;                /*4 byte*/
     unsigned long        	biClrImportant;      /*4 byte*/
};

class jhVideoOutputDealer
{
public:
    jhVideoOutputDealer();
    ~jhVideoOutputDealer();

    inline void YV12_to_RGB24 (unsigned char * pSrcY, unsigned char * pSrcU, unsigned char * pSrcV,
                                                                                        int Width, int Height, double dXSrc, double dYSrc, int & nR, int & nG, int & nB);
    void yuv420pToRgb24(unsigned char *pSrcY, unsigned char *pSrcU, 
                                                unsigned char *pSrcV, int width, int height, unsigned char *pDst);
    void saveAsBmpFile(char* fileName, unsigned char* data, int W, int H);

    void  yuv420pToRgb565Center(unsigned char *pSrcY, unsigned char *pSrcU, unsigned char *pSrcV, int srcWidth,
                                                                                int srcHeight, __u16 *pDst, int dstWidth, int dstHeight, int stepSize);
    void yuv420pNearestNeighborScale(unsigned char *pSrcY, unsigned char *pSrcU, unsigned char *pSrcV, int srcWidth, 
	                                                                         int srcHeight, __u16 *pDst, int dstWidth, int dstHeight, int stepSize);
    void yuv420pNearestNeighborScaleCenter(unsigned char *pSrcY, unsigned char *pSrcU, unsigned char *pSrcV, int srcWidth,
	                                                                         int srcHeight, __u16 *pDst, int dstWidth, int dstHeight, int stepSize);
    void yuv420pNearestNeighborScaleCenter_v2(unsigned char * pSrcY, unsigned char * pSrcU, unsigned char * pSrcV,
                                                                             int srcWidth, int srcHeight, __u16 * pDst, int dstWidth, int dstHeight, int stepSize);
    void yuv420pCalcNearestNeighbor(unsigned char *pSrcY, unsigned char *pSrcU,unsigned char *pSrcV, int Width, 
                                                                                int Height,double dXSrc, double dYSrc, int &nR, int &nG, int &nB);
    
    inline void getScaledSize(int srcWidth, int srcHeight,int dstWidth, int dstHeight, int *scaledWidth, int *scaledHeight);

private:
    struct BmpFileHeader mFileHeader;
    struct BmpInfoHeader mInfoHeader;
};

}// namespace jhsys

#endif //JHSYS_VIDEO_OUTPUT_DEALER_H
