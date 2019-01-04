#ifndef _IMAGE_H_
#define _IMAGE_H_

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#pragma pack(push, 1)

struct BitmapFileHeader {
  uint16_t headerIdentify;
  uint32_t fileSize;
  uint16_t reserved1;
  uint16_t reserved2;
  uint32_t pixelDataOffset;
};

struct BitmapInfoHeader {
  uint32_t infoHeaderSize;
  int32_t width;
  int32_t height;
  uint16_t planes;
  uint16_t bitsPerPixel;
  uint32_t compression;
  uint32_t sizeOfBitmap;
  int32_t horzResolution;
  int32_t vertResolution;
  uint32_t numberOfColors;
  uint32_t importantColors;
};

#pragma pack(pop)


struct Image {
  int32_t width;
  int32_t height;
  uint32_t* pixelData;
};

Image CreateImage(int32_t width, int32_t height);
void WriteImageFile(Image* image, const char* filename);
void FreeImage(Image* image);

#endif
