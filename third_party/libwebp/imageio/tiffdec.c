// Copyright 2012 Google Inc. All Rights Reserved.
//
// Use of this source code is governed by a BSD-style license
// that can be found in the COPYING file in the root of the source
// tree. An additional intellectual property rights grant can be found
// in the file PATENTS. All contributing project authors may
// be found in the AUTHORS file in the root of the source tree.
// -----------------------------------------------------------------------------
//
// TIFF decode.

#include "./tiffdec.h"

#ifdef HAVE_CONFIG_H
#include "webp/config.h"
#endif

#include <stdio.h>
#include <string.h>

#ifdef WEBP_HAVE_TIFF
#include <tiffio.h>

#include "webp/encode.h"
#include "./imageio_util.h"
#include "./metadata.h"

static const struct {
  ttag_t tag;
  size_t storage_offset;
} kTIFFMetadataMap[] = {
  { TIFFTAG_ICCPROFILE, METADATA_OFFSET(iccp) },
  { TIFFTAG_XMLPACKET,  METADATA_OFFSET(xmp) },
  { 0, 0 },
};

// Returns true on success. The caller must use MetadataFree() on 'metadata' in
// all cases.
static int ExtractMetadataFromTIFF(TIFF* const tif, Metadata* const metadata) {
  int i;
  toff_t exif_ifd_offset;

  for (i = 0; kTIFFMetadataMap[i].tag != 0; ++i) {
    MetadataPayload* const payload =
        (MetadataPayload*)((uint8_t*)metadata +
                           kTIFFMetadataMap[i].storage_offset);
    void* tag_data;
    uint32 tag_data_len;

    if (TIFFGetField(tif, kTIFFMetadataMap[i].tag, &tag_data_len, &tag_data) &&
        !MetadataCopy((const char*)tag_data, tag_data_len, payload)) {
      return 0;
    }
  }

  // TODO(jzern): To extract the raw EXIF directory some parsing of it would be
  // necessary to determine the overall size. In addition, value offsets in
  // individual directory entries may need to be updated as, depending on the
  // type, they are file based.
  // Exif 2.2 Section 4.6.2 Tag Structure
  // TIFF Revision 6.0 Part 1 Section 2 TIFF Structure #Image File Directory
  if (TIFFGetField(tif, TIFFTAG_EXIFIFD, &exif_ifd_offset)) {
    fprintf(stderr, "Warning: EXIF extraction from TIFF is unsupported.\n");
  }
  return 1;
}

// Ad-hoc structure to supply read-from-memory functionalities.
typedef struct {
  const uint8_t* data;
  toff_t size;
  toff_t pos;
} MyData;

static int MyClose(thandle_t opaque) {
  (void)opaque;
  return 0;
}

static toff_t MySize(thandle_t opaque) {
  const MyData* const my_data = (MyData*)opaque;
  return my_data->size;
}

static toff_t MySeek(thandle_t opaque, toff_t offset, int whence) {
  MyData* const my_data = (MyData*)opaque;
  offset += (whence == SEEK_CUR) ? my_data->pos
          : (whence == SEEK_SET) ? 0
          : my_data->size;
  if (offset > my_data->size) return (toff_t)-1;
  my_data->pos = offset;
  return offset;
}

static int MyMapFile(thandle_t opaque, void** base, toff_t* size) {
  (void)opaque;
  (void)base;
  (void)size;
  return 0;
}
static void MyUnmapFile(thandle_t opaque, void* base, toff_t size) {
  (void)opaque;
  (void)base;
  (void)size;
}

static tsize_t MyRead(thandle_t opaque, void* dst, tsize_t size) {
  MyData* const my_data = (MyData*)opaque;
  if (my_data->pos + size > my_data->size) {
    size = my_data->size - my_data->pos;
  }
  if (size > 0) {
    memcpy(dst, my_data->data + my_data->pos, size);
    my_data->pos += size;
  }
  return size;
}

int ReadTIFF(const uint8_t* const data, size_t data_size,
             WebPPicture* const pic, int keep_alpha,
             Metadata* const metadata) {
  MyData my_data = { data, (toff_t)data_size, 0 };
  TIFF* tif;
  uint32 width, height;
  uint32* raster;
  int64_t alloc_size;
  int ok = 0;
  tdir_t dircount;

  if (data == NULL || data_size == 0 || pic == NULL) return 0;

  tif = TIFFClientOpen("Memory", "r", &my_data,
                       MyRead, MyRead, MySeek, MyClose,
                       MySize, MyMapFile, MyUnmapFile);
  if (tif == NULL) {
    fprintf(stderr, "Error! Cannot parse TIFF file\n");
    return 0;
  }

  dircount = TIFFNumberOfDirectories(tif);
  if (dircount > 1) {
    fprintf(stderr, "Warning: multi-directory TIFF files are not supported.\n"
                    "Only the first will be used, %d will be ignored.\n",
                    dircount - 1);
  }

  if (!(TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &width) &&
        TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &height))) {
    fprintf(stderr, "Error! Cannot retrieve TIFF image dimensions.\n");
    goto End;
  }

  if (!ImgIoUtilCheckSizeArgumentsOverflow((uint64_t)width * height,
                                           sizeof(*raster))) {
    goto End;
  }
  // _Tiffmalloc uses a signed type for size.
  alloc_size = (int64_t)((uint64_t)width * height * sizeof(*raster));
  if (alloc_size < 0 || alloc_size != (tsize_t)alloc_size) goto End;

  raster = (uint32*)_TIFFmalloc((tsize_t)alloc_size);
  if (raster != NULL) {
    if (TIFFReadRGBAImageOriented(tif, width, height, raster,
                                  ORIENTATION_TOPLEFT, 1)) {
      const int stride = width * sizeof(*raster);
      pic->width = width;
      pic->height = height;
      // TIFF data is ABGR
#ifdef WORDS_BIGENDIAN
      TIFFSwabArrayOfLong(raster, width * height);
#endif
      ok = keep_alpha
         ? WebPPictureImportRGBA(pic, (const uint8_t*)raster, stride)
         : WebPPictureImportRGBX(pic, (const uint8_t*)raster, stride);
    }
    _TIFFfree(raster);
  } else {
    fprintf(stderr, "Error allocating TIFF RGBA memory!\n");
  }

  if (ok) {
    if (metadata != NULL) {
      ok = ExtractMetadataFromTIFF(tif, metadata);
      if (!ok) {
        fprintf(stderr, "Error extracting TIFF metadata!\n");
        MetadataFree(metadata);
        WebPPictureFree(pic);
      }
    }
  }
 End:
  TIFFClose(tif);
  return ok;
}
#else  // !WEBP_HAVE_TIFF
int ReadTIFF(const uint8_t* const data, size_t data_size,
             struct WebPPicture* const pic, int keep_alpha,
             struct Metadata* const metadata) {
  (void)data;
  (void)data_size;
  (void)pic;
  (void)keep_alpha;
  (void)metadata;
  fprintf(stderr, "TIFF support not compiled. Please install the libtiff "
          "development package before building.\n");
  return 0;
}
#endif  // WEBP_HAVE_TIFF

// -----------------------------------------------------------------------------
