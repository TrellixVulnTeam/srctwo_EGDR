/* Copyright 2015 Google Inc. All Rights Reserved.

   Distributed under MIT license.
   See file LICENSE for detail or copy at https://opensource.org/licenses/MIT
*/

/* Function for fast encoding of an input fragment, independently from the input
   history. This function uses two-pass processing: in the first pass we save
   the found backward matches and literal bytes into a buffer, and in the
   second pass we emit them into the bit stream using prefix codes built based
   on the actual command and literal byte histograms. */

#ifndef BROTLI_ENC_COMPRESS_FRAGMENT_TWO_PASS_H_
#define BROTLI_ENC_COMPRESS_FRAGMENT_TWO_PASS_H_

#include <brotli/types.h>
#include "./memory.h"
#include "./port.h"

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

static const size_t kCompressFragmentTwoPassBlockSize = 1 << 17;

/* Compresses "input" string to the "*storage" buffer as one or more complete
   meta-blocks, and updates the "*storage_ix" bit position.

   If "is_last" is 1, emits an additional empty last meta-block.

   REQUIRES: "input_size" is greater than zero, or "is_last" is 1.
   REQUIRES: "input_size" is less or equal to maximal metablock size (1 << 24).
   REQUIRES: "command_buf" and "literal_buf" point to at least
              kCompressFragmentTwoPassBlockSize long arrays.
   REQUIRES: All elements in "table[0..table_size-1]" are initialized to zero.
   REQUIRES: "table_size" is a power of two
   OUTPUT: maximal copy distance <= |input_size|
   OUTPUT: maximal copy distance <= BROTLI_MAX_BACKWARD_LIMIT(18) */
BROTLI_INTERNAL void BrotliCompressFragmentTwoPass(MemoryManager* m,
                                                   const uint8_t* input,
                                                   size_t input_size,
                                                   BROTLI_BOOL is_last,
                                                   uint32_t* command_buf,
                                                   uint8_t* literal_buf,
                                                   int* table,
                                                   size_t table_size,
                                                   size_t* storage_ix,
                                                   uint8_t* storage);

#if defined(__cplusplus) || defined(c_plusplus)
}  /* extern "C" */
#endif

#endif  /* BROTLI_ENC_COMPRESS_FRAGMENT_TWO_PASS_H_ */
