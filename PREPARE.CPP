// This uses C++ classes that can be found at www.github.com/kosmonautdnb/watcomlibs
#include "IMAGE.HPP"

int main(int argc, const char *argv[]) {
  RGBAImage img = RGBAImage::fromFile("ZIFFERN.PNG");
  int count = 11;
  int w = img.width / count;
  int h = img.height;
  FILE *out = fopen("ZIFFERN.BIN","w");
  for (int i = 0; i < count; i++) {
    for (int y = 0; y < h; y++) {
      for (int x = 0; x < w; x+=32) {
        unsigned int writeOut = 0;
        for (int x3 = 0; x3 < 4; x3++) {
          for (int x2 = 0; x2 < 8; x2++) {
            unsigned int col = img.data[x2 + x3*8 + x + i * w + y * img.width];
            int v = (col>>24) & 255;
            int b = 0;
            if (v > 128) b = 1;
            writeOut |= b << ((7-x2)+x3*8);
          }
        }
        static int k = 0; k++;
        fprintf(out,"%u,",writeOut);
        if ((k & 15) == 0) fprintf(out,"\n");
      }
    }
  }
  fclose(out);
  return 0;
}
