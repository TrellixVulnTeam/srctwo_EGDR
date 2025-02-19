/* Copyright (c) 2013 The Chromium Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

/* From dev/ppb_truetype_font_dev.idl modified Tue Oct 15 05:52:52 2013. */

#ifndef PPAPI_C_DEV_PPB_TRUETYPE_FONT_DEV_H_
#define PPAPI_C_DEV_PPB_TRUETYPE_FONT_DEV_H_

#include "ppapi/c/pp_array_output.h"
#include "ppapi/c/pp_bool.h"
#include "ppapi/c/pp_completion_callback.h"
#include "ppapi/c/pp_instance.h"
#include "ppapi/c/pp_macros.h"
#include "ppapi/c/pp_resource.h"
#include "ppapi/c/pp_stdint.h"
#include "ppapi/c/pp_var.h"

#define PPB_TRUETYPEFONT_DEV_INTERFACE_0_1 "PPB_TrueTypeFont(Dev);0.1"
#define PPB_TRUETYPEFONT_DEV_INTERFACE PPB_TRUETYPEFONT_DEV_INTERFACE_0_1

/**
 * @file
 * This file defines the <code>PPB_TrueTypeFont_Dev</code> interface. This
 * interface exposes font table data for 'sfnt' fonts on the host system. These
 * include TrueType and OpenType fonts.
 */


/**
 * @addtogroup Enums
 * @{
 */
/**
 * The PP_TrueTypeFontFamily_Dev defines generic font families. These can be
 * used to create generic fonts consistent with the user's browser settings.
 */
typedef enum {
  /**
   * For a description of these default families, see the
   * <a href="http://www.w3.org/TR/css3-fonts/#generic-font-families">
   * 3.1.1 Generic font families</a> documentation.
   */
  PP_TRUETYPEFONTFAMILY_SERIF = 0,
  PP_TRUETYPEFONTFAMILY_SANSSERIF = 1,
  PP_TRUETYPEFONTFAMILY_CURSIVE = 2,
  PP_TRUETYPEFONTFAMILY_FANTASY = 3,
  PP_TRUETYPEFONTFAMILY_MONOSPACE = 4
} PP_TrueTypeFontFamily_Dev;
PP_COMPILE_ASSERT_SIZE_IN_BYTES(PP_TrueTypeFontFamily_Dev, 4);

/**
 * The PP_TrueTypeFontStyle_Dev enum defines font styles.
 */
typedef enum {
  PP_TRUETYPEFONTSTYLE_NORMAL = 0,
  PP_TRUETYPEFONTSTYLE_ITALIC = 1
} PP_TrueTypeFontStyle_Dev;
PP_COMPILE_ASSERT_SIZE_IN_BYTES(PP_TrueTypeFontStyle_Dev, 4);

/**
 * The PP_TrueTypeFontWeight_Dev enum defines font weights.
 */
typedef enum {
  PP_TRUETYPEFONTWEIGHT_THIN = 100,
  PP_TRUETYPEFONTWEIGHT_ULTRALIGHT = 200,
  PP_TRUETYPEFONTWEIGHT_LIGHT = 300,
  PP_TRUETYPEFONTWEIGHT_NORMAL = 400,
  PP_TRUETYPEFONTWEIGHT_MEDIUM = 500,
  PP_TRUETYPEFONTWEIGHT_SEMIBOLD = 600,
  PP_TRUETYPEFONTWEIGHT_BOLD = 700,
  PP_TRUETYPEFONTWEIGHT_ULTRABOLD = 800,
  PP_TRUETYPEFONTWEIGHT_HEAVY = 900
} PP_TrueTypeFontWeight_Dev;
PP_COMPILE_ASSERT_SIZE_IN_BYTES(PP_TrueTypeFontWeight_Dev, 4);

/**
 * The PP_TrueTypeFontWidth_Dev enum defines font widths.
 */
typedef enum {
  PP_TRUETYPEFONTWIDTH_ULTRACONDENSED = 0,
  PP_TRUETYPEFONTWIDTH_EXTRACONDENSED = 1,
  PP_TRUETYPEFONTWIDTH_CONDENSED = 2,
  PP_TRUETYPEFONTWIDTH_SEMICONDENSED = 3,
  PP_TRUETYPEFONTWIDTH_NORMAL = 4,
  PP_TRUETYPEFONTWIDTH_SEMIEXPANDED = 5,
  PP_TRUETYPEFONTWIDTH_EXPANDED = 6,
  PP_TRUETYPEFONTWIDTH_EXTRAEXPANDED = 7,
  PP_TRUETYPEFONTWIDTH_ULTRAEXPANDED = 8
} PP_TrueTypeFontWidth_Dev;
PP_COMPILE_ASSERT_SIZE_IN_BYTES(PP_TrueTypeFontWidth_Dev, 4);

/**
 * The PP_TrueTypeFontCharset enum defines font character sets.
 */
typedef enum {
  PP_TRUETYPEFONTCHARSET_ANSI = 0,
  PP_TRUETYPEFONTCHARSET_DEFAULT = 1,
  PP_TRUETYPEFONTCHARSET_SYMBOL = 2,
  PP_TRUETYPEFONTCHARSET_MAC = 77,
  PP_TRUETYPEFONTCHARSET_SHIFTJIS = 128,
  PP_TRUETYPEFONTCHARSET_HANGUL = 129,
  PP_TRUETYPEFONTCHARSET_JOHAB = 130,
  PP_TRUETYPEFONTCHARSET_GB2312 = 134,
  PP_TRUETYPEFONTCHARSET_CHINESEBIG5 = 136,
  PP_TRUETYPEFONTCHARSET_GREEK = 161,
  PP_TRUETYPEFONTCHARSET_TURKISH = 162,
  PP_TRUETYPEFONTCHARSET_VIETNAMESE = 163,
  PP_TRUETYPEFONTCHARSET_HEBREW = 177,
  PP_TRUETYPEFONTCHARSET_ARABIC = 178,
  PP_TRUETYPEFONTCHARSET_BALTIC = 186,
  PP_TRUETYPEFONTCHARSET_RUSSIAN = 204,
  PP_TRUETYPEFONTCHARSET_THAI = 222,
  PP_TRUETYPEFONTCHARSET_EASTEUROPE = 238,
  PP_TRUETYPEFONTCHARSET_OEM = 255
} PP_TrueTypeFontCharset_Dev;
PP_COMPILE_ASSERT_SIZE_IN_BYTES(PP_TrueTypeFontCharset_Dev, 4);
/**
 * @}
 */

/**
 * @addtogroup Structs
 * @{
 */
/**
 * The <code>PP_TrueTypeFontDesc</code> struct describes a TrueType font. It is
 * passed to Create(), and returned by Describe().
 */
struct PP_TrueTypeFontDesc_Dev {
  /**
   * Font family name as a string. This can also be an undefined var, in which
   * case the generic family will be obeyed. If the face is not available on
   * the system, the browser will attempt to do font fallback or pick a default
   * font.
   */
  struct PP_Var family;
  /** This value specifies a generic font family. If a family name string is
   * provided when creating a font, this is ignored. */
  PP_TrueTypeFontFamily_Dev generic_family;
  /** This value specifies the font style. */
  PP_TrueTypeFontStyle_Dev style;
  /** This value specifies the font weight. */
  PP_TrueTypeFontWeight_Dev weight;
  /** This value specifies the font width, for condensed or expanded fonts */
  PP_TrueTypeFontWidth_Dev width;
  /** This value specifies a character set. */
  PP_TrueTypeFontCharset_Dev charset;
  /**
   * Ensure that this struct is 40-bytes wide by padding the end.  In some
   * compilers, PP_Var is 8-byte aligned, so those compilers align this struct
   * on 8-byte boundaries as well and pad it to 16 bytes even without this
   * padding attribute.  This padding makes its size consistent across
   * compilers.
   */
  int32_t padding;
};
PP_COMPILE_ASSERT_STRUCT_SIZE_IN_BYTES(PP_TrueTypeFontDesc_Dev, 40);
/**
 * @}
 */

/**
 * @addtogroup Interfaces
 * @{
 */
struct PPB_TrueTypeFont_Dev_0_1 {
  /**
   * Gets an array of TrueType font family names available on the host.
   * These names can be used to create a font from a specific family.
   *
   * @param[in] instance A <code>PP_Instance</code> requesting the family names.
   * @param[in] output A <code>PP_ArrayOutput</code> to hold the names.
   * The output is an array of PP_Vars, each holding a family name.
   * @param[in] callback A <code>PP_CompletionCallback</code> to be called upon
   * completion of GetFontFamilies.
   *
   * @return If >= 0, the number of family names returned, otherwise an error
   * code from <code>pp_errors.h</code>.
   */
  int32_t (*GetFontFamilies)(PP_Instance instance,
                             struct PP_ArrayOutput output,
                             struct PP_CompletionCallback callback);
  /**
   * Gets an array of TrueType font descriptors for a given font family. These
   * descriptors can be used to create a font in that family and matching the
   * descriptor attributes.
   *
   * @param[in] instance A <code>PP_Instance</code> requesting the font
   * descriptors.
   * @param[in] family A <code>PP_Var</code> holding a string specifying the
   * font family.
   * @param[in] output A <code>PP_ArrayOutput</code> to hold the descriptors.
   * The output is an array of <code>PP_TrueTypeFontDesc</code> structs. Each
   * desc contains a PP_Var for the family name which must be released.
   * @param[in] callback A <code>PP_CompletionCallback</code> to be called upon
   * completion of GetFontsInFamily.
   *
   * @return If >= 0, the number of font descriptors returned, otherwise an
   * error code from <code>pp_errors.h</code>.
   */
  int32_t (*GetFontsInFamily)(PP_Instance instance,
                              struct PP_Var family,
                              struct PP_ArrayOutput output,
                              struct PP_CompletionCallback callback);
  /**
   * Creates a font resource matching the given font characteristics. The
   * resource id will be non-zero on success, or zero on failure.
   *
   * @param[in] instance A <code>PP_Instance</code> to own the font.
   * @param[in] desc A pointer to a <code>PP_TrueTypeFontDesc</code> describing
   * the font.
   */
  PP_Resource (*Create)(PP_Instance instance,
                        const struct PP_TrueTypeFontDesc_Dev* desc);
  /**
   * Determines if the given resource is a TrueType font.
   *
   * @param[in] resource A <code>PP_Resource</code> corresponding to a resource.
   *
   * @return <code>PP_TRUE</code> if the resource is a
   * <code>PPB_TrueTypeFont_Dev</code>, <code>PP_FALSE</code> otherwise.
   */
  PP_Bool (*IsTrueTypeFont)(PP_Resource resource);
  /**
   * Returns a description of the given font resource. This description may
   * differ from the description passed to Create, reflecting the host's font
   * matching and fallback algorithm.
   *
   * @param[in] font A <code>PP_Resource</code> corresponding to a font.
   * @param[out] desc A pointer to a <code>PP_TrueTypeFontDesc</code> to hold
   * the description. The internal 'family' PP_Var should be set to undefined,
   * since this function overwrites the <code>PP_TrueTypeFontDesc</code>. After
   * successful completion, the family will be set to a PP_Var with a single
   * reference, which the caller must release after use.
   * @param[in] callback A <code>PP_CompletionCallback</code> to be called upon
   * completion of Describe.
   *
   * @return A return code from <code>pp_errors.h</code>. If an error code is
   * returned, the <code>PP_TrueTypeFontDesc</code> will be unchanged.
   */
  int32_t (*Describe)(PP_Resource font,
                      struct PP_TrueTypeFontDesc_Dev* desc,
                      struct PP_CompletionCallback callback);
  /**
   * Gets an array of identifying tags for each table in the font. These tags
   * can be used to request specific tables using GetTable.
   *
   * @param[in] font A <code>PP_Resource</code> corresponding to a font.
   * @param[in] output A <code>PP_ArrayOutput</code> to hold the tags.
   * The output is an array of 4 byte integers, each representing a table tag.
   * @param[in] callback A <code>PP_CompletionCallback</code> to be called upon
   * completion of GetTableTags.
   *
   * @return If >= 0, the number of table tags returned, otherwise an error
   * code from <code>pp_errors.h</code>.
   */
  int32_t (*GetTableTags)(PP_Resource font,
                          struct PP_ArrayOutput output,
                          struct PP_CompletionCallback callback);
  /**
   * Copies the given font table into client memory.
   *
   * @param[in] font A <code>PP_Resource</code> corresponding to a font.
   * @param[in] table A 4 byte value indicating which table to copy.
   * For example, 'glyf' will cause the outline table to be copied into the
   * output array. A zero tag value will cause the entire font to be copied.
   * @param[in] offset The offset into the font table. Passing an offset
   * greater than or equal to the table size will succeed with 0 bytes copied.
   * @param[in] max_data_length The maximum number of bytes to transfer from
   * <code>offset</code>.
   * @param[in] output A <code>PP_ArrayOutput</code> to hold the font data.
   * The output is an array of bytes.
   * @param[in] callback A <code>PP_CompletionCallback</code> to be called upon
   * completion of GetTable.
   *
   * @return If >= 0, the table size in bytes, otherwise an error code from
   * <code>pp_errors.h</code>.
   */
  int32_t (*GetTable)(PP_Resource font,
                      uint32_t table,
                      int32_t offset,
                      int32_t max_data_length,
                      struct PP_ArrayOutput output,
                      struct PP_CompletionCallback callback);
};

typedef struct PPB_TrueTypeFont_Dev_0_1 PPB_TrueTypeFont_Dev;
/**
 * @}
 */

#endif  /* PPAPI_C_DEV_PPB_TRUETYPE_FONT_DEV_H_ */

