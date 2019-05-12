#include "image.h"

Image CreateImage(int32_t width, int32_t height) {
    Image image = {};
    image.width = width;
    image.height = height;
    image.pixelData = (uint32_t*)malloc(width * height * sizeof(uint32_t));
    return image;
}

void WriteImageFile(Image* image, const char* filename) {
    int pixelDataSize = image->width * image->height * sizeof(uint32_t);
    BitmapFileHeader fileHeader = {};
    fileHeader.headerIdentify = 0x4D42;
    fileHeader.fileSize = sizeof(BitmapFileHeader) + sizeof(BitmapInfoHeader) + pixelDataSize;
    fileHeader.reserved1 = 0;
    fileHeader.reserved2 = 0;
    fileHeader.pixelDataOffset = sizeof(BitmapFileHeader) + sizeof(BitmapInfoHeader);

    BitmapInfoHeader infoHeader = {};
    infoHeader.infoHeaderSize = sizeof(BitmapInfoHeader);
    infoHeader.width = image->width;
    infoHeader.height = -image->height;
    infoHeader.planes = 1;
    infoHeader.bitsPerPixel = 32;
    infoHeader.compression = 0;
    infoHeader.sizeOfBitmap = 0;
    infoHeader.horzResolution = 0;
    infoHeader.vertResolution = 0;
    infoHeader.numberOfColors = 0;
    infoHeader.importantColors = 0;

    FILE* file = fopen(filename, "wb");
    if (file) {
        fwrite(&fileHeader, sizeof(BitmapFileHeader), 1, file);
        fwrite(&infoHeader, sizeof(BitmapInfoHeader), 1, file);
        fwrite(image->pixelData, pixelDataSize, 1, file);
        fclose(file);
    }
}

void FreeImage(Image* image) {
    free(image->pixelData);
}

