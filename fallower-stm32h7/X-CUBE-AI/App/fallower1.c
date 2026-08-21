/**
  ******************************************************************************
  * @file    fallower1.c
  * @author  AST Embedded Analytics Research Platform
  * @date    2026-02-05T14:23:25+0300
  * @brief   AI Tool Automatic Code Generator for Embedded NN computing
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  ******************************************************************************
  */


#include "fallower1.h"
#include "fallower1_data.h"

#include "ai_platform.h"
#include "ai_platform_interface.h"
#include "ai_math_helpers.h"

#include "core_common.h"
#include "core_convert.h"

#include "layers.h"



#undef AI_NET_OBJ_INSTANCE
#define AI_NET_OBJ_INSTANCE g_fallower1
 
#undef AI_FALLOWER1_MODEL_SIGNATURE
#define AI_FALLOWER1_MODEL_SIGNATURE     "0x0088a28a7dbfa7ec39f6bc2db57c0ef6"

#ifndef AI_TOOLS_REVISION_ID
#define AI_TOOLS_REVISION_ID     ""
#endif

#undef AI_TOOLS_DATE_TIME
#define AI_TOOLS_DATE_TIME   "2026-02-05T14:23:25+0300"

#undef AI_TOOLS_COMPILE_TIME
#define AI_TOOLS_COMPILE_TIME    __DATE__ " " __TIME__

#undef AI_FALLOWER1_N_BATCHES
#define AI_FALLOWER1_N_BATCHES         (1)

static ai_ptr g_fallower1_activations_map[1] = AI_C_ARRAY_INIT;
static ai_ptr g_fallower1_weights_map[1] = AI_C_ARRAY_INIT;



/**  Array declarations section  **********************************************/
/* Array#0 */
AI_ARRAY_OBJ_DECLARE(
  input_output_array, AI_ARRAY_FORMAT_FLOAT|AI_FMT_FLAG_IS_IO,
  NULL, NULL, 20640, AI_STATIC)

/* Array#1 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_0_features_0_0_Conv_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 82560, AI_STATIC)

/* Array#2 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_0_features_0_2_HardSigmoid_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 82560, AI_STATIC)

/* Array#3 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_0_features_0_2_Mul_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 82560, AI_STATIC)

/* Array#4 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_1_block_block_0_block_0_0_Conv_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 22016, AI_STATIC)

/* Array#5 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_1_block_block_0_block_0_2_Relu_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 22016, AI_STATIC)

/* Array#6 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_1_block_block_1_avgpool_GlobalAveragePool_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 16, AI_STATIC)

/* Array#7 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_1_block_block_1_fc1_Conv_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 8, AI_STATIC)

/* Array#8 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_1_block_block_1_activation_Relu_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 8, AI_STATIC)

/* Array#9 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_1_block_block_1_fc2_Conv_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 16, AI_STATIC)

/* Array#10 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_1_block_block_1_scale_activation_HardSigmoid_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 16, AI_STATIC)

/* Array#11 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_1_block_block_1_Mul_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 22016, AI_STATIC)

/* Array#12 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_1_block_block_2_block_2_0_Conv_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 22016, AI_STATIC)

/* Array#13 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_2_block_block_0_block_0_0_Conv_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 99072, AI_STATIC)

/* Array#14 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_2_block_block_0_block_0_2_Relu_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 99072, AI_STATIC)

/* Array#15 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_2_block_block_1_block_1_0_Conv_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 24768, AI_STATIC)

/* Array#16 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_2_block_block_1_block_1_2_Relu_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 24768, AI_STATIC)

/* Array#17 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_2_block_block_2_block_2_0_Conv_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 8256, AI_STATIC)

/* Array#18 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_3_block_block_0_block_0_0_Conv_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 30272, AI_STATIC)

/* Array#19 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_3_block_block_0_block_0_2_Relu_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 30272, AI_STATIC)

/* Array#20 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_3_block_block_1_block_1_0_Conv_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 30272, AI_STATIC)

/* Array#21 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_3_block_block_1_block_1_2_Relu_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 30272, AI_STATIC)

/* Array#22 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_3_block_block_2_block_2_0_Conv_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 8256, AI_STATIC)

/* Array#23 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_3_Add_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 8256, AI_STATIC)

/* Array#24 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_4_block_block_0_block_0_0_Conv_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 33024, AI_STATIC)

/* Array#25 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_4_block_block_0_block_0_2_HardSigmoid_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 33024, AI_STATIC)

/* Array#26 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_4_block_block_0_block_0_2_Mul_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 33024, AI_STATIC)

/* Array#27 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_4_block_block_1_block_1_0_Conv_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 8256, AI_STATIC)

/* Array#28 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_4_block_block_1_block_1_2_HardSigmoid_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 8256, AI_STATIC)

/* Array#29 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_4_block_block_1_block_1_2_Mul_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 8256, AI_STATIC)

/* Array#30 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_4_block_block_2_avgpool_GlobalAveragePool_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 96, AI_STATIC)

/* Array#31 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_4_block_block_2_fc1_Conv_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 24, AI_STATIC)

/* Array#32 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_4_block_block_2_activation_Relu_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 24, AI_STATIC)

/* Array#33 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_4_block_block_2_fc2_Conv_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 96, AI_STATIC)

/* Array#34 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_4_block_block_2_scale_activation_HardSigmoid_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 96, AI_STATIC)

/* Array#35 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_4_block_block_2_Mul_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 8256, AI_STATIC)

/* Array#36 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_4_block_block_3_block_3_0_Conv_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 3440, AI_STATIC)

/* Array#37 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_5_block_block_0_block_0_0_Conv_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 20640, AI_STATIC)

/* Array#38 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_5_block_block_0_block_0_2_HardSigmoid_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 20640, AI_STATIC)

/* Array#39 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_5_block_block_0_block_0_2_Mul_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 20640, AI_STATIC)

/* Array#40 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_5_block_block_1_block_1_0_Conv_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 20640, AI_STATIC)

/* Array#41 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_5_block_block_1_block_1_2_HardSigmoid_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 20640, AI_STATIC)

/* Array#42 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_5_block_block_1_block_1_2_Mul_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 20640, AI_STATIC)

/* Array#43 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_5_block_block_2_avgpool_GlobalAveragePool_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 240, AI_STATIC)

/* Array#44 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_5_block_block_2_fc1_Conv_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 64, AI_STATIC)

/* Array#45 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_5_block_block_2_activation_Relu_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 64, AI_STATIC)

/* Array#46 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_5_block_block_2_fc2_Conv_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 240, AI_STATIC)

/* Array#47 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_5_block_block_2_scale_activation_HardSigmoid_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 240, AI_STATIC)

/* Array#48 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_5_block_block_2_Mul_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 20640, AI_STATIC)

/* Array#49 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_5_block_block_3_block_3_0_Conv_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 3440, AI_STATIC)

/* Array#50 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_5_Add_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 3440, AI_STATIC)

/* Array#51 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_6_block_block_0_block_0_0_Conv_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 20640, AI_STATIC)

/* Array#52 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_6_block_block_0_block_0_2_HardSigmoid_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 20640, AI_STATIC)

/* Array#53 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_6_block_block_0_block_0_2_Mul_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 20640, AI_STATIC)

/* Array#54 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_6_block_block_1_block_1_0_Conv_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 20640, AI_STATIC)

/* Array#55 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_6_block_block_1_block_1_2_HardSigmoid_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 20640, AI_STATIC)

/* Array#56 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_6_block_block_1_block_1_2_Mul_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 20640, AI_STATIC)

/* Array#57 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_6_block_block_2_avgpool_GlobalAveragePool_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 240, AI_STATIC)

/* Array#58 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_6_block_block_2_fc1_Conv_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 64, AI_STATIC)

/* Array#59 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_6_block_block_2_activation_Relu_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 64, AI_STATIC)

/* Array#60 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_6_block_block_2_fc2_Conv_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 240, AI_STATIC)

/* Array#61 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_6_block_block_2_scale_activation_HardSigmoid_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 240, AI_STATIC)

/* Array#62 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_6_block_block_2_Mul_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 20640, AI_STATIC)

/* Array#63 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_6_block_block_3_block_3_0_Conv_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 3440, AI_STATIC)

/* Array#64 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_6_Add_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 3440, AI_STATIC)

/* Array#65 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_7_block_block_0_block_0_0_Conv_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 10320, AI_STATIC)

/* Array#66 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_7_block_block_0_block_0_2_HardSigmoid_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 10320, AI_STATIC)

/* Array#67 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_7_block_block_0_block_0_2_Mul_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 10320, AI_STATIC)

/* Array#68 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_7_block_block_1_block_1_0_Conv_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 10320, AI_STATIC)

/* Array#69 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_7_block_block_1_block_1_2_HardSigmoid_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 10320, AI_STATIC)

/* Array#70 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_7_block_block_1_block_1_2_Mul_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 10320, AI_STATIC)

/* Array#71 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_7_block_block_2_avgpool_GlobalAveragePool_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 120, AI_STATIC)

/* Array#72 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_7_block_block_2_fc1_Conv_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 32, AI_STATIC)

/* Array#73 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_7_block_block_2_activation_Relu_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 32, AI_STATIC)

/* Array#74 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_7_block_block_2_fc2_Conv_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 120, AI_STATIC)

/* Array#75 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_7_block_block_2_scale_activation_HardSigmoid_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 120, AI_STATIC)

/* Array#76 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_7_block_block_2_Mul_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 10320, AI_STATIC)

/* Array#77 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_7_block_block_3_block_3_0_Conv_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 4128, AI_STATIC)

/* Array#78 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_8_block_block_0_block_0_0_Conv_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 12384, AI_STATIC)

/* Array#79 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_8_block_block_0_block_0_2_HardSigmoid_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 12384, AI_STATIC)

/* Array#80 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_8_block_block_0_block_0_2_Mul_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 12384, AI_STATIC)

/* Array#81 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_8_block_block_1_block_1_0_Conv_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 12384, AI_STATIC)

/* Array#82 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_8_block_block_1_block_1_2_HardSigmoid_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 12384, AI_STATIC)

/* Array#83 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_8_block_block_1_block_1_2_Mul_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 12384, AI_STATIC)

/* Array#84 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_8_block_block_2_avgpool_GlobalAveragePool_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 144, AI_STATIC)

/* Array#85 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_8_block_block_2_fc1_Conv_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 40, AI_STATIC)

/* Array#86 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_8_block_block_2_activation_Relu_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 40, AI_STATIC)

/* Array#87 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_8_block_block_2_fc2_Conv_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 144, AI_STATIC)

/* Array#88 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_8_block_block_2_scale_activation_HardSigmoid_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 144, AI_STATIC)

/* Array#89 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_8_block_block_2_Mul_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 12384, AI_STATIC)

/* Array#90 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_8_block_block_3_block_3_0_Conv_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 4128, AI_STATIC)

/* Array#91 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_8_Add_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 4128, AI_STATIC)

/* Array#92 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_9_block_block_0_block_0_0_Conv_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 24768, AI_STATIC)

/* Array#93 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_9_block_block_0_block_0_2_HardSigmoid_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 24768, AI_STATIC)

/* Array#94 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_9_block_block_0_block_0_2_Mul_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 24768, AI_STATIC)

/* Array#95 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_9_block_block_1_block_1_0_Conv_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 6336, AI_STATIC)

/* Array#96 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_9_block_block_1_block_1_2_HardSigmoid_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 6336, AI_STATIC)

/* Array#97 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_9_block_block_1_block_1_2_Mul_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 6336, AI_STATIC)

/* Array#98 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_9_block_block_2_avgpool_GlobalAveragePool_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 288, AI_STATIC)

/* Array#99 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_9_block_block_2_fc1_Conv_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 72, AI_STATIC)

/* Array#100 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_9_block_block_2_activation_Relu_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 72, AI_STATIC)

/* Array#101 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_9_block_block_2_fc2_Conv_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 288, AI_STATIC)

/* Array#102 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_9_block_block_2_scale_activation_HardSigmoid_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 288, AI_STATIC)

/* Array#103 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_9_block_block_2_Mul_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 6336, AI_STATIC)

/* Array#104 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_9_block_block_3_block_3_0_Conv_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 2112, AI_STATIC)

/* Array#105 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_10_block_block_0_block_0_0_Conv_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 12672, AI_STATIC)

/* Array#106 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_10_block_block_0_block_0_2_HardSigmoid_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 12672, AI_STATIC)

/* Array#107 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_10_block_block_0_block_0_2_Mul_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 12672, AI_STATIC)

/* Array#108 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_10_block_block_1_block_1_0_Conv_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 12672, AI_STATIC)

/* Array#109 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_10_block_block_1_block_1_2_HardSigmoid_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 12672, AI_STATIC)

/* Array#110 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_10_block_block_1_block_1_2_Mul_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 12672, AI_STATIC)

/* Array#111 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_10_block_block_2_avgpool_GlobalAveragePool_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 576, AI_STATIC)

/* Array#112 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_10_block_block_2_fc1_Conv_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 144, AI_STATIC)

/* Array#113 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_10_block_block_2_activation_Relu_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 144, AI_STATIC)

/* Array#114 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_10_block_block_2_fc2_Conv_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 576, AI_STATIC)

/* Array#115 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_10_block_block_2_scale_activation_HardSigmoid_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 576, AI_STATIC)

/* Array#116 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_10_block_block_2_Mul_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 12672, AI_STATIC)

/* Array#117 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_10_block_block_3_block_3_0_Conv_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 2112, AI_STATIC)

/* Array#118 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_10_Add_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 2112, AI_STATIC)

/* Array#119 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_11_block_block_0_block_0_0_Conv_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 12672, AI_STATIC)

/* Array#120 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_11_block_block_0_block_0_2_HardSigmoid_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 12672, AI_STATIC)

/* Array#121 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_11_block_block_0_block_0_2_Mul_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 12672, AI_STATIC)

/* Array#122 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_11_block_block_1_block_1_0_Conv_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 12672, AI_STATIC)

/* Array#123 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_11_block_block_1_block_1_2_HardSigmoid_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 12672, AI_STATIC)

/* Array#124 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_11_block_block_1_block_1_2_Mul_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 12672, AI_STATIC)

/* Array#125 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_11_block_block_2_avgpool_GlobalAveragePool_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 576, AI_STATIC)

/* Array#126 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_11_block_block_2_fc1_Conv_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 144, AI_STATIC)

/* Array#127 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_11_block_block_2_activation_Relu_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 144, AI_STATIC)

/* Array#128 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_11_block_block_2_fc2_Conv_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 576, AI_STATIC)

/* Array#129 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_11_block_block_2_scale_activation_HardSigmoid_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 576, AI_STATIC)

/* Array#130 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_11_block_block_2_Mul_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 12672, AI_STATIC)

/* Array#131 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_11_block_block_3_block_3_0_Conv_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 2112, AI_STATIC)

/* Array#132 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_11_Add_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 2112, AI_STATIC)

/* Array#133 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_12_features_12_0_Conv_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 12672, AI_STATIC)

/* Array#134 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_12_features_12_2_HardSigmoid_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 12672, AI_STATIC)

/* Array#135 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_12_features_12_2_Mul_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 12672, AI_STATIC)

/* Array#136 */
AI_ARRAY_OBJ_DECLARE(
  _avgpool_GlobalAveragePool_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 576, AI_STATIC)

/* Array#137 */
AI_ARRAY_OBJ_DECLARE(
  _classifier_classifier_0_Gemm_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 256, AI_STATIC)

/* Array#138 */
AI_ARRAY_OBJ_DECLARE(
  _classifier_classifier_1_Relu_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 256, AI_STATIC)

/* Array#139 */
AI_ARRAY_OBJ_DECLARE(
  output_output_array, AI_ARRAY_FORMAT_FLOAT|AI_FMT_FLAG_IS_IO,
  NULL, NULL, 7, AI_STATIC)

/* Array#140 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_0_features_0_0_Conv_output_0_weights_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 144, AI_STATIC)

/* Array#141 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_0_features_0_0_Conv_output_0_bias_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 16, AI_STATIC)

/* Array#142 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_1_block_block_0_block_0_0_Conv_output_0_weights_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 144, AI_STATIC)

/* Array#143 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_1_block_block_0_block_0_0_Conv_output_0_bias_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 16, AI_STATIC)

/* Array#144 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_1_block_block_1_fc1_Conv_output_0_weights_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 128, AI_STATIC)

/* Array#145 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_1_block_block_1_fc1_Conv_output_0_bias_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 8, AI_STATIC)

/* Array#146 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_1_block_block_1_fc2_Conv_output_0_weights_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 128, AI_STATIC)

/* Array#147 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_1_block_block_1_fc2_Conv_output_0_bias_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 16, AI_STATIC)

/* Array#148 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_1_block_block_2_block_2_0_Conv_output_0_weights_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 256, AI_STATIC)

/* Array#149 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_1_block_block_2_block_2_0_Conv_output_0_bias_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 16, AI_STATIC)

/* Array#150 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_2_block_block_0_block_0_0_Conv_output_0_weights_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 1152, AI_STATIC)

/* Array#151 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_2_block_block_0_block_0_0_Conv_output_0_bias_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 72, AI_STATIC)

/* Array#152 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_2_block_block_1_block_1_0_Conv_output_0_weights_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 648, AI_STATIC)

/* Array#153 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_2_block_block_1_block_1_0_Conv_output_0_bias_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 72, AI_STATIC)

/* Array#154 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_2_block_block_2_block_2_0_Conv_output_0_weights_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 1728, AI_STATIC)

/* Array#155 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_2_block_block_2_block_2_0_Conv_output_0_bias_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 24, AI_STATIC)

/* Array#156 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_3_block_block_0_block_0_0_Conv_output_0_weights_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 2112, AI_STATIC)

/* Array#157 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_3_block_block_0_block_0_0_Conv_output_0_bias_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 88, AI_STATIC)

/* Array#158 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_3_block_block_1_block_1_0_Conv_output_0_weights_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 792, AI_STATIC)

/* Array#159 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_3_block_block_1_block_1_0_Conv_output_0_bias_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 88, AI_STATIC)

/* Array#160 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_3_block_block_2_block_2_0_Conv_output_0_weights_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 2112, AI_STATIC)

/* Array#161 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_3_block_block_2_block_2_0_Conv_output_0_bias_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 24, AI_STATIC)

/* Array#162 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_4_block_block_0_block_0_0_Conv_output_0_weights_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 2304, AI_STATIC)

/* Array#163 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_4_block_block_0_block_0_0_Conv_output_0_bias_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 96, AI_STATIC)

/* Array#164 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_4_block_block_1_block_1_0_Conv_output_0_weights_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 2400, AI_STATIC)

/* Array#165 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_4_block_block_1_block_1_0_Conv_output_0_bias_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 96, AI_STATIC)

/* Array#166 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_4_block_block_2_fc1_Conv_output_0_weights_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 2304, AI_STATIC)

/* Array#167 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_4_block_block_2_fc1_Conv_output_0_bias_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 24, AI_STATIC)

/* Array#168 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_4_block_block_2_fc2_Conv_output_0_weights_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 2304, AI_STATIC)

/* Array#169 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_4_block_block_2_fc2_Conv_output_0_bias_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 96, AI_STATIC)

/* Array#170 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_4_block_block_3_block_3_0_Conv_output_0_weights_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 3840, AI_STATIC)

/* Array#171 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_4_block_block_3_block_3_0_Conv_output_0_bias_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 40, AI_STATIC)

/* Array#172 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_5_block_block_0_block_0_0_Conv_output_0_weights_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 9600, AI_STATIC)

/* Array#173 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_5_block_block_0_block_0_0_Conv_output_0_bias_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 240, AI_STATIC)

/* Array#174 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_5_block_block_1_block_1_0_Conv_output_0_weights_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 6000, AI_STATIC)

/* Array#175 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_5_block_block_1_block_1_0_Conv_output_0_bias_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 240, AI_STATIC)

/* Array#176 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_5_block_block_2_fc1_Conv_output_0_weights_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 15360, AI_STATIC)

/* Array#177 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_5_block_block_2_fc1_Conv_output_0_bias_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 64, AI_STATIC)

/* Array#178 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_5_block_block_2_fc2_Conv_output_0_weights_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 15360, AI_STATIC)

/* Array#179 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_5_block_block_2_fc2_Conv_output_0_bias_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 240, AI_STATIC)

/* Array#180 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_5_block_block_3_block_3_0_Conv_output_0_weights_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 9600, AI_STATIC)

/* Array#181 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_5_block_block_3_block_3_0_Conv_output_0_bias_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 40, AI_STATIC)

/* Array#182 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_6_block_block_0_block_0_0_Conv_output_0_weights_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 9600, AI_STATIC)

/* Array#183 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_6_block_block_0_block_0_0_Conv_output_0_bias_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 240, AI_STATIC)

/* Array#184 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_6_block_block_1_block_1_0_Conv_output_0_weights_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 6000, AI_STATIC)

/* Array#185 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_6_block_block_1_block_1_0_Conv_output_0_bias_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 240, AI_STATIC)

/* Array#186 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_6_block_block_2_fc1_Conv_output_0_weights_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 15360, AI_STATIC)

/* Array#187 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_6_block_block_2_fc1_Conv_output_0_bias_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 64, AI_STATIC)

/* Array#188 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_6_block_block_2_fc2_Conv_output_0_weights_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 15360, AI_STATIC)

/* Array#189 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_6_block_block_2_fc2_Conv_output_0_bias_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 240, AI_STATIC)

/* Array#190 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_6_block_block_3_block_3_0_Conv_output_0_weights_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 9600, AI_STATIC)

/* Array#191 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_6_block_block_3_block_3_0_Conv_output_0_bias_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 40, AI_STATIC)

/* Array#192 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_7_block_block_0_block_0_0_Conv_output_0_weights_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 4800, AI_STATIC)

/* Array#193 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_7_block_block_0_block_0_0_Conv_output_0_bias_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 120, AI_STATIC)

/* Array#194 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_7_block_block_1_block_1_0_Conv_output_0_weights_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 3000, AI_STATIC)

/* Array#195 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_7_block_block_1_block_1_0_Conv_output_0_bias_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 120, AI_STATIC)

/* Array#196 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_7_block_block_2_fc1_Conv_output_0_weights_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 3840, AI_STATIC)

/* Array#197 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_7_block_block_2_fc1_Conv_output_0_bias_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 32, AI_STATIC)

/* Array#198 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_7_block_block_2_fc2_Conv_output_0_weights_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 3840, AI_STATIC)

/* Array#199 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_7_block_block_2_fc2_Conv_output_0_bias_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 120, AI_STATIC)

/* Array#200 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_7_block_block_3_block_3_0_Conv_output_0_weights_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 5760, AI_STATIC)

/* Array#201 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_7_block_block_3_block_3_0_Conv_output_0_bias_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 48, AI_STATIC)

/* Array#202 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_8_block_block_0_block_0_0_Conv_output_0_weights_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 6912, AI_STATIC)

/* Array#203 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_8_block_block_0_block_0_0_Conv_output_0_bias_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 144, AI_STATIC)

/* Array#204 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_8_block_block_1_block_1_0_Conv_output_0_weights_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 3600, AI_STATIC)

/* Array#205 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_8_block_block_1_block_1_0_Conv_output_0_bias_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 144, AI_STATIC)

/* Array#206 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_8_block_block_2_fc1_Conv_output_0_weights_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 5760, AI_STATIC)

/* Array#207 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_8_block_block_2_fc1_Conv_output_0_bias_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 40, AI_STATIC)

/* Array#208 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_8_block_block_2_fc2_Conv_output_0_weights_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 5760, AI_STATIC)

/* Array#209 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_8_block_block_2_fc2_Conv_output_0_bias_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 144, AI_STATIC)

/* Array#210 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_8_block_block_3_block_3_0_Conv_output_0_weights_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 6912, AI_STATIC)

/* Array#211 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_8_block_block_3_block_3_0_Conv_output_0_bias_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 48, AI_STATIC)

/* Array#212 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_9_block_block_0_block_0_0_Conv_output_0_weights_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 13824, AI_STATIC)

/* Array#213 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_9_block_block_0_block_0_0_Conv_output_0_bias_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 288, AI_STATIC)

/* Array#214 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_9_block_block_1_block_1_0_Conv_output_0_weights_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 7200, AI_STATIC)

/* Array#215 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_9_block_block_1_block_1_0_Conv_output_0_bias_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 288, AI_STATIC)

/* Array#216 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_9_block_block_2_fc1_Conv_output_0_weights_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 20736, AI_STATIC)

/* Array#217 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_9_block_block_2_fc1_Conv_output_0_bias_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 72, AI_STATIC)

/* Array#218 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_9_block_block_2_fc2_Conv_output_0_weights_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 20736, AI_STATIC)

/* Array#219 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_9_block_block_2_fc2_Conv_output_0_bias_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 288, AI_STATIC)

/* Array#220 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_9_block_block_3_block_3_0_Conv_output_0_weights_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 27648, AI_STATIC)

/* Array#221 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_9_block_block_3_block_3_0_Conv_output_0_bias_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 96, AI_STATIC)

/* Array#222 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_10_block_block_0_block_0_0_Conv_output_0_weights_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 55296, AI_STATIC)

/* Array#223 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_10_block_block_0_block_0_0_Conv_output_0_bias_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 576, AI_STATIC)

/* Array#224 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_10_block_block_1_block_1_0_Conv_output_0_weights_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 14400, AI_STATIC)

/* Array#225 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_10_block_block_1_block_1_0_Conv_output_0_bias_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 576, AI_STATIC)

/* Array#226 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_10_block_block_2_fc1_Conv_output_0_weights_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 82944, AI_STATIC)

/* Array#227 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_10_block_block_2_fc1_Conv_output_0_bias_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 144, AI_STATIC)

/* Array#228 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_10_block_block_2_fc2_Conv_output_0_weights_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 82944, AI_STATIC)

/* Array#229 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_10_block_block_2_fc2_Conv_output_0_bias_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 576, AI_STATIC)

/* Array#230 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_10_block_block_3_block_3_0_Conv_output_0_weights_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 55296, AI_STATIC)

/* Array#231 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_10_block_block_3_block_3_0_Conv_output_0_bias_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 96, AI_STATIC)

/* Array#232 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_11_block_block_0_block_0_0_Conv_output_0_weights_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 55296, AI_STATIC)

/* Array#233 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_11_block_block_0_block_0_0_Conv_output_0_bias_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 576, AI_STATIC)

/* Array#234 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_11_block_block_1_block_1_0_Conv_output_0_weights_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 14400, AI_STATIC)

/* Array#235 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_11_block_block_1_block_1_0_Conv_output_0_bias_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 576, AI_STATIC)

/* Array#236 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_11_block_block_2_fc1_Conv_output_0_weights_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 82944, AI_STATIC)

/* Array#237 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_11_block_block_2_fc1_Conv_output_0_bias_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 144, AI_STATIC)

/* Array#238 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_11_block_block_2_fc2_Conv_output_0_weights_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 82944, AI_STATIC)

/* Array#239 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_11_block_block_2_fc2_Conv_output_0_bias_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 576, AI_STATIC)

/* Array#240 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_11_block_block_3_block_3_0_Conv_output_0_weights_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 55296, AI_STATIC)

/* Array#241 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_11_block_block_3_block_3_0_Conv_output_0_bias_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 96, AI_STATIC)

/* Array#242 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_12_features_12_0_Conv_output_0_weights_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 55296, AI_STATIC)

/* Array#243 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_12_features_12_0_Conv_output_0_bias_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 576, AI_STATIC)

/* Array#244 */
AI_ARRAY_OBJ_DECLARE(
  _classifier_classifier_0_Gemm_output_0_weights_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 147456, AI_STATIC)

/* Array#245 */
AI_ARRAY_OBJ_DECLARE(
  _classifier_classifier_0_Gemm_output_0_bias_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 256, AI_STATIC)

/* Array#246 */
AI_ARRAY_OBJ_DECLARE(
  output_weights_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 1792, AI_STATIC)

/* Array#247 */
AI_ARRAY_OBJ_DECLARE(
  output_bias_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 7, AI_STATIC)

/* Array#248 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_0_features_0_0_Conv_output_0_scratch0_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 9, AI_STATIC)

/* Array#249 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_1_block_block_1_fc1_Conv_output_0_scratch0_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 16, AI_STATIC)

/* Array#250 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_1_block_block_1_fc2_Conv_output_0_scratch0_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 8, AI_STATIC)

/* Array#251 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_1_block_block_2_block_2_0_Conv_output_0_scratch0_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 16, AI_STATIC)

/* Array#252 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_2_block_block_0_block_0_0_Conv_output_0_scratch0_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 16, AI_STATIC)

/* Array#253 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_2_block_block_2_block_2_0_Conv_output_0_scratch0_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 72, AI_STATIC)

/* Array#254 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_3_block_block_0_block_0_0_Conv_output_0_scratch0_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 24, AI_STATIC)

/* Array#255 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_3_block_block_2_block_2_0_Conv_output_0_scratch0_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 88, AI_STATIC)

/* Array#256 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_4_block_block_0_block_0_0_Conv_output_0_scratch0_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 24, AI_STATIC)

/* Array#257 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_4_block_block_2_fc1_Conv_output_0_scratch0_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 96, AI_STATIC)

/* Array#258 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_4_block_block_2_fc2_Conv_output_0_scratch0_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 24, AI_STATIC)

/* Array#259 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_4_block_block_3_block_3_0_Conv_output_0_scratch0_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 96, AI_STATIC)

/* Array#260 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_5_block_block_0_block_0_0_Conv_output_0_scratch0_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 40, AI_STATIC)

/* Array#261 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_5_block_block_2_fc1_Conv_output_0_scratch0_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 240, AI_STATIC)

/* Array#262 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_5_block_block_2_fc2_Conv_output_0_scratch0_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 64, AI_STATIC)

/* Array#263 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_5_block_block_3_block_3_0_Conv_output_0_scratch0_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 240, AI_STATIC)

/* Array#264 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_6_block_block_0_block_0_0_Conv_output_0_scratch0_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 40, AI_STATIC)

/* Array#265 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_6_block_block_2_fc1_Conv_output_0_scratch0_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 240, AI_STATIC)

/* Array#266 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_6_block_block_2_fc2_Conv_output_0_scratch0_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 64, AI_STATIC)

/* Array#267 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_6_block_block_3_block_3_0_Conv_output_0_scratch0_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 240, AI_STATIC)

/* Array#268 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_7_block_block_0_block_0_0_Conv_output_0_scratch0_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 40, AI_STATIC)

/* Array#269 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_7_block_block_2_fc1_Conv_output_0_scratch0_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 120, AI_STATIC)

/* Array#270 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_7_block_block_2_fc2_Conv_output_0_scratch0_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 32, AI_STATIC)

/* Array#271 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_7_block_block_3_block_3_0_Conv_output_0_scratch0_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 120, AI_STATIC)

/* Array#272 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_8_block_block_0_block_0_0_Conv_output_0_scratch0_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 48, AI_STATIC)

/* Array#273 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_8_block_block_2_fc1_Conv_output_0_scratch0_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 144, AI_STATIC)

/* Array#274 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_8_block_block_2_fc2_Conv_output_0_scratch0_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 40, AI_STATIC)

/* Array#275 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_8_block_block_3_block_3_0_Conv_output_0_scratch0_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 144, AI_STATIC)

/* Array#276 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_9_block_block_0_block_0_0_Conv_output_0_scratch0_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 48, AI_STATIC)

/* Array#277 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_9_block_block_2_fc1_Conv_output_0_scratch0_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 288, AI_STATIC)

/* Array#278 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_9_block_block_2_fc2_Conv_output_0_scratch0_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 72, AI_STATIC)

/* Array#279 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_9_block_block_3_block_3_0_Conv_output_0_scratch0_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 288, AI_STATIC)

/* Array#280 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_10_block_block_0_block_0_0_Conv_output_0_scratch0_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 96, AI_STATIC)

/* Array#281 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_10_block_block_2_fc1_Conv_output_0_scratch0_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 576, AI_STATIC)

/* Array#282 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_10_block_block_2_fc2_Conv_output_0_scratch0_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 144, AI_STATIC)

/* Array#283 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_10_block_block_3_block_3_0_Conv_output_0_scratch0_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 576, AI_STATIC)

/* Array#284 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_11_block_block_0_block_0_0_Conv_output_0_scratch0_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 96, AI_STATIC)

/* Array#285 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_11_block_block_2_fc1_Conv_output_0_scratch0_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 576, AI_STATIC)

/* Array#286 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_11_block_block_2_fc2_Conv_output_0_scratch0_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 144, AI_STATIC)

/* Array#287 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_11_block_block_3_block_3_0_Conv_output_0_scratch0_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 576, AI_STATIC)

/* Array#288 */
AI_ARRAY_OBJ_DECLARE(
  _features_features_12_features_12_0_Conv_output_0_scratch0_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 96, AI_STATIC)

/**  Tensor declarations section  *********************************************/
/* Tensor #0 */
AI_TENSOR_OBJ_DECLARE(
  _avgpool_GlobalAveragePool_output_0_output, AI_STATIC,
  0, 0x0,
  AI_SHAPE_INIT(4, 1, 576, 1, 1), AI_STRIDE_INIT(4, 4, 4, 2304, 2304),
  1, &_avgpool_GlobalAveragePool_output_0_output_array, NULL)

/* Tensor #1 */
AI_TENSOR_OBJ_DECLARE(
  _classifier_classifier_0_Gemm_output_0_bias, AI_STATIC,
  1, 0x0,
  AI_SHAPE_INIT(4, 1, 256, 1, 1), AI_STRIDE_INIT(4, 4, 4, 1024, 1024),
  1, &_classifier_classifier_0_Gemm_output_0_bias_array, NULL)

/* Tensor #2 */
AI_TENSOR_OBJ_DECLARE(
  _classifier_classifier_0_Gemm_output_0_output, AI_STATIC,
  2, 0x0,
  AI_SHAPE_INIT(4, 1, 256, 1, 1), AI_STRIDE_INIT(4, 4, 4, 1024, 1024),
  1, &_classifier_classifier_0_Gemm_output_0_output_array, NULL)

/* Tensor #3 */
AI_TENSOR_OBJ_DECLARE(
  _classifier_classifier_0_Gemm_output_0_weights, AI_STATIC,
  3, 0x0,
  AI_SHAPE_INIT(4, 576, 256, 1, 1), AI_STRIDE_INIT(4, 4, 2304, 589824, 589824),
  1, &_classifier_classifier_0_Gemm_output_0_weights_array, NULL)

/* Tensor #4 */
AI_TENSOR_OBJ_DECLARE(
  _classifier_classifier_1_Relu_output_0_output, AI_STATIC,
  4, 0x0,
  AI_SHAPE_INIT(4, 1, 256, 1, 1), AI_STRIDE_INIT(4, 4, 4, 1024, 1024),
  1, &_classifier_classifier_1_Relu_output_0_output_array, NULL)

/* Tensor #5 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_0_features_0_0_Conv_output_0_bias, AI_STATIC,
  5, 0x0,
  AI_SHAPE_INIT(4, 1, 16, 1, 1), AI_STRIDE_INIT(4, 4, 4, 64, 64),
  1, &_features_features_0_features_0_0_Conv_output_0_bias_array, NULL)

/* Tensor #6 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_0_features_0_0_Conv_output_0_output, AI_STATIC,
  6, 0x0,
  AI_SHAPE_INIT(4, 1, 16, 344, 15), AI_STRIDE_INIT(4, 4, 4, 64, 22016),
  1, &_features_features_0_features_0_0_Conv_output_0_output_array, NULL)

/* Tensor #7 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_0_features_0_0_Conv_output_0_scratch0, AI_STATIC,
  7, 0x0,
  AI_SHAPE_INIT(4, 1, 1, 3, 3), AI_STRIDE_INIT(4, 4, 4, 4, 12),
  1, &_features_features_0_features_0_0_Conv_output_0_scratch0_array, NULL)

/* Tensor #8 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_0_features_0_0_Conv_output_0_weights, AI_STATIC,
  8, 0x0,
  AI_SHAPE_INIT(4, 1, 3, 3, 16), AI_STRIDE_INIT(4, 4, 4, 64, 192),
  1, &_features_features_0_features_0_0_Conv_output_0_weights_array, NULL)

/* Tensor #9 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_0_features_0_2_HardSigmoid_output_0_output, AI_STATIC,
  9, 0x0,
  AI_SHAPE_INIT(4, 1, 16, 344, 15), AI_STRIDE_INIT(4, 4, 4, 64, 22016),
  1, &_features_features_0_features_0_2_HardSigmoid_output_0_output_array, NULL)

/* Tensor #10 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_0_features_0_2_Mul_output_0_output, AI_STATIC,
  10, 0x0,
  AI_SHAPE_INIT(4, 1, 16, 344, 15), AI_STRIDE_INIT(4, 4, 4, 64, 22016),
  1, &_features_features_0_features_0_2_Mul_output_0_output_array, NULL)

/* Tensor #11 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_10_Add_output_0_output, AI_STATIC,
  11, 0x0,
  AI_SHAPE_INIT(4, 1, 96, 22, 1), AI_STRIDE_INIT(4, 4, 4, 384, 8448),
  1, &_features_features_10_Add_output_0_output_array, NULL)

/* Tensor #12 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_10_block_block_0_block_0_0_Conv_output_0_bias, AI_STATIC,
  12, 0x0,
  AI_SHAPE_INIT(4, 1, 576, 1, 1), AI_STRIDE_INIT(4, 4, 4, 2304, 2304),
  1, &_features_features_10_block_block_0_block_0_0_Conv_output_0_bias_array, NULL)

/* Tensor #13 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_10_block_block_0_block_0_0_Conv_output_0_output, AI_STATIC,
  13, 0x0,
  AI_SHAPE_INIT(4, 1, 576, 22, 1), AI_STRIDE_INIT(4, 4, 4, 2304, 50688),
  1, &_features_features_10_block_block_0_block_0_0_Conv_output_0_output_array, NULL)

/* Tensor #14 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_10_block_block_0_block_0_0_Conv_output_0_scratch0, AI_STATIC,
  14, 0x0,
  AI_SHAPE_INIT(4, 1, 96, 1, 1), AI_STRIDE_INIT(4, 4, 4, 384, 384),
  1, &_features_features_10_block_block_0_block_0_0_Conv_output_0_scratch0_array, NULL)

/* Tensor #15 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_10_block_block_0_block_0_0_Conv_output_0_weights, AI_STATIC,
  15, 0x0,
  AI_SHAPE_INIT(4, 96, 1, 1, 576), AI_STRIDE_INIT(4, 4, 384, 221184, 221184),
  1, &_features_features_10_block_block_0_block_0_0_Conv_output_0_weights_array, NULL)

/* Tensor #16 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_10_block_block_0_block_0_2_HardSigmoid_output_0_output, AI_STATIC,
  16, 0x0,
  AI_SHAPE_INIT(4, 1, 576, 22, 1), AI_STRIDE_INIT(4, 4, 4, 2304, 50688),
  1, &_features_features_10_block_block_0_block_0_2_HardSigmoid_output_0_output_array, NULL)

/* Tensor #17 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_10_block_block_0_block_0_2_Mul_output_0_output, AI_STATIC,
  17, 0x0,
  AI_SHAPE_INIT(4, 1, 576, 22, 1), AI_STRIDE_INIT(4, 4, 4, 2304, 50688),
  1, &_features_features_10_block_block_0_block_0_2_Mul_output_0_output_array, NULL)

/* Tensor #18 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_10_block_block_1_block_1_0_Conv_output_0_bias, AI_STATIC,
  18, 0x0,
  AI_SHAPE_INIT(4, 1, 576, 1, 1), AI_STRIDE_INIT(4, 4, 4, 2304, 2304),
  1, &_features_features_10_block_block_1_block_1_0_Conv_output_0_bias_array, NULL)

/* Tensor #19 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_10_block_block_1_block_1_0_Conv_output_0_output, AI_STATIC,
  19, 0x0,
  AI_SHAPE_INIT(4, 1, 576, 22, 1), AI_STRIDE_INIT(4, 4, 4, 2304, 50688),
  1, &_features_features_10_block_block_1_block_1_0_Conv_output_0_output_array, NULL)

/* Tensor #20 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_10_block_block_1_block_1_0_Conv_output_0_weights, AI_STATIC,
  20, 0x0,
  AI_SHAPE_INIT(4, 1, 5, 5, 576), AI_STRIDE_INIT(4, 1, 576, 576, 576),
  1, &_features_features_10_block_block_1_block_1_0_Conv_output_0_weights_array, NULL)

/* Tensor #21 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_10_block_block_1_block_1_2_HardSigmoid_output_0_output, AI_STATIC,
  21, 0x0,
  AI_SHAPE_INIT(4, 1, 576, 22, 1), AI_STRIDE_INIT(4, 4, 4, 2304, 50688),
  1, &_features_features_10_block_block_1_block_1_2_HardSigmoid_output_0_output_array, NULL)

/* Tensor #22 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_10_block_block_1_block_1_2_Mul_output_0_output, AI_STATIC,
  22, 0x0,
  AI_SHAPE_INIT(4, 1, 576, 22, 1), AI_STRIDE_INIT(4, 4, 4, 2304, 50688),
  1, &_features_features_10_block_block_1_block_1_2_Mul_output_0_output_array, NULL)

/* Tensor #23 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_10_block_block_2_Mul_output_0_output, AI_STATIC,
  23, 0x0,
  AI_SHAPE_INIT(4, 1, 576, 22, 1), AI_STRIDE_INIT(4, 4, 4, 2304, 50688),
  1, &_features_features_10_block_block_2_Mul_output_0_output_array, NULL)

/* Tensor #24 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_10_block_block_2_activation_Relu_output_0_output, AI_STATIC,
  24, 0x0,
  AI_SHAPE_INIT(4, 1, 144, 1, 1), AI_STRIDE_INIT(4, 4, 4, 576, 576),
  1, &_features_features_10_block_block_2_activation_Relu_output_0_output_array, NULL)

/* Tensor #25 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_10_block_block_2_avgpool_GlobalAveragePool_output_0_output, AI_STATIC,
  25, 0x0,
  AI_SHAPE_INIT(4, 1, 576, 1, 1), AI_STRIDE_INIT(4, 4, 4, 2304, 2304),
  1, &_features_features_10_block_block_2_avgpool_GlobalAveragePool_output_0_output_array, NULL)

/* Tensor #26 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_10_block_block_2_fc1_Conv_output_0_bias, AI_STATIC,
  26, 0x0,
  AI_SHAPE_INIT(4, 1, 144, 1, 1), AI_STRIDE_INIT(4, 4, 4, 576, 576),
  1, &_features_features_10_block_block_2_fc1_Conv_output_0_bias_array, NULL)

/* Tensor #27 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_10_block_block_2_fc1_Conv_output_0_output, AI_STATIC,
  27, 0x0,
  AI_SHAPE_INIT(4, 1, 144, 1, 1), AI_STRIDE_INIT(4, 4, 4, 576, 576),
  1, &_features_features_10_block_block_2_fc1_Conv_output_0_output_array, NULL)

/* Tensor #28 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_10_block_block_2_fc1_Conv_output_0_scratch0, AI_STATIC,
  28, 0x0,
  AI_SHAPE_INIT(4, 1, 576, 1, 1), AI_STRIDE_INIT(4, 4, 4, 2304, 2304),
  1, &_features_features_10_block_block_2_fc1_Conv_output_0_scratch0_array, NULL)

/* Tensor #29 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_10_block_block_2_fc1_Conv_output_0_weights, AI_STATIC,
  29, 0x0,
  AI_SHAPE_INIT(4, 576, 1, 1, 144), AI_STRIDE_INIT(4, 4, 2304, 331776, 331776),
  1, &_features_features_10_block_block_2_fc1_Conv_output_0_weights_array, NULL)

/* Tensor #30 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_10_block_block_2_fc2_Conv_output_0_bias, AI_STATIC,
  30, 0x0,
  AI_SHAPE_INIT(4, 1, 576, 1, 1), AI_STRIDE_INIT(4, 4, 4, 2304, 2304),
  1, &_features_features_10_block_block_2_fc2_Conv_output_0_bias_array, NULL)

/* Tensor #31 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_10_block_block_2_fc2_Conv_output_0_output, AI_STATIC,
  31, 0x0,
  AI_SHAPE_INIT(4, 1, 576, 1, 1), AI_STRIDE_INIT(4, 4, 4, 2304, 2304),
  1, &_features_features_10_block_block_2_fc2_Conv_output_0_output_array, NULL)

/* Tensor #32 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_10_block_block_2_fc2_Conv_output_0_scratch0, AI_STATIC,
  32, 0x0,
  AI_SHAPE_INIT(4, 1, 144, 1, 1), AI_STRIDE_INIT(4, 4, 4, 576, 576),
  1, &_features_features_10_block_block_2_fc2_Conv_output_0_scratch0_array, NULL)

/* Tensor #33 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_10_block_block_2_fc2_Conv_output_0_weights, AI_STATIC,
  33, 0x0,
  AI_SHAPE_INIT(4, 144, 1, 1, 576), AI_STRIDE_INIT(4, 4, 576, 331776, 331776),
  1, &_features_features_10_block_block_2_fc2_Conv_output_0_weights_array, NULL)

/* Tensor #34 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_10_block_block_2_scale_activation_HardSigmoid_output_0_output, AI_STATIC,
  34, 0x0,
  AI_SHAPE_INIT(4, 1, 576, 1, 1), AI_STRIDE_INIT(4, 4, 4, 2304, 2304),
  1, &_features_features_10_block_block_2_scale_activation_HardSigmoid_output_0_output_array, NULL)

/* Tensor #35 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_10_block_block_3_block_3_0_Conv_output_0_bias, AI_STATIC,
  35, 0x0,
  AI_SHAPE_INIT(4, 1, 96, 1, 1), AI_STRIDE_INIT(4, 4, 4, 384, 384),
  1, &_features_features_10_block_block_3_block_3_0_Conv_output_0_bias_array, NULL)

/* Tensor #36 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_10_block_block_3_block_3_0_Conv_output_0_output, AI_STATIC,
  36, 0x0,
  AI_SHAPE_INIT(4, 1, 96, 22, 1), AI_STRIDE_INIT(4, 4, 4, 384, 8448),
  1, &_features_features_10_block_block_3_block_3_0_Conv_output_0_output_array, NULL)

/* Tensor #37 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_10_block_block_3_block_3_0_Conv_output_0_scratch0, AI_STATIC,
  37, 0x0,
  AI_SHAPE_INIT(4, 1, 576, 1, 1), AI_STRIDE_INIT(4, 4, 4, 2304, 2304),
  1, &_features_features_10_block_block_3_block_3_0_Conv_output_0_scratch0_array, NULL)

/* Tensor #38 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_10_block_block_3_block_3_0_Conv_output_0_weights, AI_STATIC,
  38, 0x0,
  AI_SHAPE_INIT(4, 576, 1, 1, 96), AI_STRIDE_INIT(4, 4, 2304, 221184, 221184),
  1, &_features_features_10_block_block_3_block_3_0_Conv_output_0_weights_array, NULL)

/* Tensor #39 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_11_Add_output_0_output, AI_STATIC,
  39, 0x0,
  AI_SHAPE_INIT(4, 1, 96, 22, 1), AI_STRIDE_INIT(4, 4, 4, 384, 8448),
  1, &_features_features_11_Add_output_0_output_array, NULL)

/* Tensor #40 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_11_block_block_0_block_0_0_Conv_output_0_bias, AI_STATIC,
  40, 0x0,
  AI_SHAPE_INIT(4, 1, 576, 1, 1), AI_STRIDE_INIT(4, 4, 4, 2304, 2304),
  1, &_features_features_11_block_block_0_block_0_0_Conv_output_0_bias_array, NULL)

/* Tensor #41 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_11_block_block_0_block_0_0_Conv_output_0_output, AI_STATIC,
  41, 0x0,
  AI_SHAPE_INIT(4, 1, 576, 22, 1), AI_STRIDE_INIT(4, 4, 4, 2304, 50688),
  1, &_features_features_11_block_block_0_block_0_0_Conv_output_0_output_array, NULL)

/* Tensor #42 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_11_block_block_0_block_0_0_Conv_output_0_scratch0, AI_STATIC,
  42, 0x0,
  AI_SHAPE_INIT(4, 1, 96, 1, 1), AI_STRIDE_INIT(4, 4, 4, 384, 384),
  1, &_features_features_11_block_block_0_block_0_0_Conv_output_0_scratch0_array, NULL)

/* Tensor #43 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_11_block_block_0_block_0_0_Conv_output_0_weights, AI_STATIC,
  43, 0x0,
  AI_SHAPE_INIT(4, 96, 1, 1, 576), AI_STRIDE_INIT(4, 4, 384, 221184, 221184),
  1, &_features_features_11_block_block_0_block_0_0_Conv_output_0_weights_array, NULL)

/* Tensor #44 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_11_block_block_0_block_0_2_HardSigmoid_output_0_output, AI_STATIC,
  44, 0x0,
  AI_SHAPE_INIT(4, 1, 576, 22, 1), AI_STRIDE_INIT(4, 4, 4, 2304, 50688),
  1, &_features_features_11_block_block_0_block_0_2_HardSigmoid_output_0_output_array, NULL)

/* Tensor #45 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_11_block_block_0_block_0_2_Mul_output_0_output, AI_STATIC,
  45, 0x0,
  AI_SHAPE_INIT(4, 1, 576, 22, 1), AI_STRIDE_INIT(4, 4, 4, 2304, 50688),
  1, &_features_features_11_block_block_0_block_0_2_Mul_output_0_output_array, NULL)

/* Tensor #46 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_11_block_block_1_block_1_0_Conv_output_0_bias, AI_STATIC,
  46, 0x0,
  AI_SHAPE_INIT(4, 1, 576, 1, 1), AI_STRIDE_INIT(4, 4, 4, 2304, 2304),
  1, &_features_features_11_block_block_1_block_1_0_Conv_output_0_bias_array, NULL)

/* Tensor #47 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_11_block_block_1_block_1_0_Conv_output_0_output, AI_STATIC,
  47, 0x0,
  AI_SHAPE_INIT(4, 1, 576, 22, 1), AI_STRIDE_INIT(4, 4, 4, 2304, 50688),
  1, &_features_features_11_block_block_1_block_1_0_Conv_output_0_output_array, NULL)

/* Tensor #48 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_11_block_block_1_block_1_0_Conv_output_0_weights, AI_STATIC,
  48, 0x0,
  AI_SHAPE_INIT(4, 1, 5, 5, 576), AI_STRIDE_INIT(4, 1, 576, 576, 576),
  1, &_features_features_11_block_block_1_block_1_0_Conv_output_0_weights_array, NULL)

/* Tensor #49 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_11_block_block_1_block_1_2_HardSigmoid_output_0_output, AI_STATIC,
  49, 0x0,
  AI_SHAPE_INIT(4, 1, 576, 22, 1), AI_STRIDE_INIT(4, 4, 4, 2304, 50688),
  1, &_features_features_11_block_block_1_block_1_2_HardSigmoid_output_0_output_array, NULL)

/* Tensor #50 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_11_block_block_1_block_1_2_Mul_output_0_output, AI_STATIC,
  50, 0x0,
  AI_SHAPE_INIT(4, 1, 576, 22, 1), AI_STRIDE_INIT(4, 4, 4, 2304, 50688),
  1, &_features_features_11_block_block_1_block_1_2_Mul_output_0_output_array, NULL)

/* Tensor #51 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_11_block_block_2_Mul_output_0_output, AI_STATIC,
  51, 0x0,
  AI_SHAPE_INIT(4, 1, 576, 22, 1), AI_STRIDE_INIT(4, 4, 4, 2304, 50688),
  1, &_features_features_11_block_block_2_Mul_output_0_output_array, NULL)

/* Tensor #52 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_11_block_block_2_activation_Relu_output_0_output, AI_STATIC,
  52, 0x0,
  AI_SHAPE_INIT(4, 1, 144, 1, 1), AI_STRIDE_INIT(4, 4, 4, 576, 576),
  1, &_features_features_11_block_block_2_activation_Relu_output_0_output_array, NULL)

/* Tensor #53 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_11_block_block_2_avgpool_GlobalAveragePool_output_0_output, AI_STATIC,
  53, 0x0,
  AI_SHAPE_INIT(4, 1, 576, 1, 1), AI_STRIDE_INIT(4, 4, 4, 2304, 2304),
  1, &_features_features_11_block_block_2_avgpool_GlobalAveragePool_output_0_output_array, NULL)

/* Tensor #54 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_11_block_block_2_fc1_Conv_output_0_bias, AI_STATIC,
  54, 0x0,
  AI_SHAPE_INIT(4, 1, 144, 1, 1), AI_STRIDE_INIT(4, 4, 4, 576, 576),
  1, &_features_features_11_block_block_2_fc1_Conv_output_0_bias_array, NULL)

/* Tensor #55 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_11_block_block_2_fc1_Conv_output_0_output, AI_STATIC,
  55, 0x0,
  AI_SHAPE_INIT(4, 1, 144, 1, 1), AI_STRIDE_INIT(4, 4, 4, 576, 576),
  1, &_features_features_11_block_block_2_fc1_Conv_output_0_output_array, NULL)

/* Tensor #56 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_11_block_block_2_fc1_Conv_output_0_scratch0, AI_STATIC,
  56, 0x0,
  AI_SHAPE_INIT(4, 1, 576, 1, 1), AI_STRIDE_INIT(4, 4, 4, 2304, 2304),
  1, &_features_features_11_block_block_2_fc1_Conv_output_0_scratch0_array, NULL)

/* Tensor #57 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_11_block_block_2_fc1_Conv_output_0_weights, AI_STATIC,
  57, 0x0,
  AI_SHAPE_INIT(4, 576, 1, 1, 144), AI_STRIDE_INIT(4, 4, 2304, 331776, 331776),
  1, &_features_features_11_block_block_2_fc1_Conv_output_0_weights_array, NULL)

/* Tensor #58 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_11_block_block_2_fc2_Conv_output_0_bias, AI_STATIC,
  58, 0x0,
  AI_SHAPE_INIT(4, 1, 576, 1, 1), AI_STRIDE_INIT(4, 4, 4, 2304, 2304),
  1, &_features_features_11_block_block_2_fc2_Conv_output_0_bias_array, NULL)

/* Tensor #59 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_11_block_block_2_fc2_Conv_output_0_output, AI_STATIC,
  59, 0x0,
  AI_SHAPE_INIT(4, 1, 576, 1, 1), AI_STRIDE_INIT(4, 4, 4, 2304, 2304),
  1, &_features_features_11_block_block_2_fc2_Conv_output_0_output_array, NULL)

/* Tensor #60 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_11_block_block_2_fc2_Conv_output_0_scratch0, AI_STATIC,
  60, 0x0,
  AI_SHAPE_INIT(4, 1, 144, 1, 1), AI_STRIDE_INIT(4, 4, 4, 576, 576),
  1, &_features_features_11_block_block_2_fc2_Conv_output_0_scratch0_array, NULL)

/* Tensor #61 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_11_block_block_2_fc2_Conv_output_0_weights, AI_STATIC,
  61, 0x0,
  AI_SHAPE_INIT(4, 144, 1, 1, 576), AI_STRIDE_INIT(4, 4, 576, 331776, 331776),
  1, &_features_features_11_block_block_2_fc2_Conv_output_0_weights_array, NULL)

/* Tensor #62 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_11_block_block_2_scale_activation_HardSigmoid_output_0_output, AI_STATIC,
  62, 0x0,
  AI_SHAPE_INIT(4, 1, 576, 1, 1), AI_STRIDE_INIT(4, 4, 4, 2304, 2304),
  1, &_features_features_11_block_block_2_scale_activation_HardSigmoid_output_0_output_array, NULL)

/* Tensor #63 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_11_block_block_3_block_3_0_Conv_output_0_bias, AI_STATIC,
  63, 0x0,
  AI_SHAPE_INIT(4, 1, 96, 1, 1), AI_STRIDE_INIT(4, 4, 4, 384, 384),
  1, &_features_features_11_block_block_3_block_3_0_Conv_output_0_bias_array, NULL)

/* Tensor #64 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_11_block_block_3_block_3_0_Conv_output_0_output, AI_STATIC,
  64, 0x0,
  AI_SHAPE_INIT(4, 1, 96, 22, 1), AI_STRIDE_INIT(4, 4, 4, 384, 8448),
  1, &_features_features_11_block_block_3_block_3_0_Conv_output_0_output_array, NULL)

/* Tensor #65 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_11_block_block_3_block_3_0_Conv_output_0_scratch0, AI_STATIC,
  65, 0x0,
  AI_SHAPE_INIT(4, 1, 576, 1, 1), AI_STRIDE_INIT(4, 4, 4, 2304, 2304),
  1, &_features_features_11_block_block_3_block_3_0_Conv_output_0_scratch0_array, NULL)

/* Tensor #66 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_11_block_block_3_block_3_0_Conv_output_0_weights, AI_STATIC,
  66, 0x0,
  AI_SHAPE_INIT(4, 576, 1, 1, 96), AI_STRIDE_INIT(4, 4, 2304, 221184, 221184),
  1, &_features_features_11_block_block_3_block_3_0_Conv_output_0_weights_array, NULL)

/* Tensor #67 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_12_features_12_0_Conv_output_0_bias, AI_STATIC,
  67, 0x0,
  AI_SHAPE_INIT(4, 1, 576, 1, 1), AI_STRIDE_INIT(4, 4, 4, 2304, 2304),
  1, &_features_features_12_features_12_0_Conv_output_0_bias_array, NULL)

/* Tensor #68 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_12_features_12_0_Conv_output_0_output, AI_STATIC,
  68, 0x0,
  AI_SHAPE_INIT(4, 1, 576, 22, 1), AI_STRIDE_INIT(4, 4, 4, 2304, 50688),
  1, &_features_features_12_features_12_0_Conv_output_0_output_array, NULL)

/* Tensor #69 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_12_features_12_0_Conv_output_0_scratch0, AI_STATIC,
  69, 0x0,
  AI_SHAPE_INIT(4, 1, 96, 1, 1), AI_STRIDE_INIT(4, 4, 4, 384, 384),
  1, &_features_features_12_features_12_0_Conv_output_0_scratch0_array, NULL)

/* Tensor #70 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_12_features_12_0_Conv_output_0_weights, AI_STATIC,
  70, 0x0,
  AI_SHAPE_INIT(4, 96, 1, 1, 576), AI_STRIDE_INIT(4, 4, 384, 221184, 221184),
  1, &_features_features_12_features_12_0_Conv_output_0_weights_array, NULL)

/* Tensor #71 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_12_features_12_2_HardSigmoid_output_0_output, AI_STATIC,
  71, 0x0,
  AI_SHAPE_INIT(4, 1, 576, 22, 1), AI_STRIDE_INIT(4, 4, 4, 2304, 50688),
  1, &_features_features_12_features_12_2_HardSigmoid_output_0_output_array, NULL)

/* Tensor #72 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_12_features_12_2_Mul_output_0_output, AI_STATIC,
  72, 0x0,
  AI_SHAPE_INIT(4, 1, 576, 22, 1), AI_STRIDE_INIT(4, 4, 4, 2304, 50688),
  1, &_features_features_12_features_12_2_Mul_output_0_output_array, NULL)

/* Tensor #73 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_1_block_block_0_block_0_0_Conv_output_0_bias, AI_STATIC,
  73, 0x0,
  AI_SHAPE_INIT(4, 1, 16, 1, 1), AI_STRIDE_INIT(4, 4, 4, 64, 64),
  1, &_features_features_1_block_block_0_block_0_0_Conv_output_0_bias_array, NULL)

/* Tensor #74 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_1_block_block_0_block_0_0_Conv_output_0_output, AI_STATIC,
  74, 0x0,
  AI_SHAPE_INIT(4, 1, 16, 172, 8), AI_STRIDE_INIT(4, 4, 4, 64, 11008),
  1, &_features_features_1_block_block_0_block_0_0_Conv_output_0_output_array, NULL)

/* Tensor #75 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_1_block_block_0_block_0_0_Conv_output_0_weights, AI_STATIC,
  75, 0x0,
  AI_SHAPE_INIT(4, 1, 3, 3, 16), AI_STRIDE_INIT(4, 1, 16, 16, 16),
  1, &_features_features_1_block_block_0_block_0_0_Conv_output_0_weights_array, NULL)

/* Tensor #76 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_1_block_block_0_block_0_2_Relu_output_0_output, AI_STATIC,
  76, 0x0,
  AI_SHAPE_INIT(4, 1, 16, 172, 8), AI_STRIDE_INIT(4, 4, 4, 64, 11008),
  1, &_features_features_1_block_block_0_block_0_2_Relu_output_0_output_array, NULL)

/* Tensor #77 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_1_block_block_1_Mul_output_0_output, AI_STATIC,
  77, 0x0,
  AI_SHAPE_INIT(4, 1, 16, 172, 8), AI_STRIDE_INIT(4, 4, 4, 64, 11008),
  1, &_features_features_1_block_block_1_Mul_output_0_output_array, NULL)

/* Tensor #78 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_1_block_block_1_activation_Relu_output_0_output, AI_STATIC,
  78, 0x0,
  AI_SHAPE_INIT(4, 1, 8, 1, 1), AI_STRIDE_INIT(4, 4, 4, 32, 32),
  1, &_features_features_1_block_block_1_activation_Relu_output_0_output_array, NULL)

/* Tensor #79 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_1_block_block_1_avgpool_GlobalAveragePool_output_0_output, AI_STATIC,
  79, 0x0,
  AI_SHAPE_INIT(4, 1, 16, 1, 1), AI_STRIDE_INIT(4, 4, 4, 64, 64),
  1, &_features_features_1_block_block_1_avgpool_GlobalAveragePool_output_0_output_array, NULL)

/* Tensor #80 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_1_block_block_1_fc1_Conv_output_0_bias, AI_STATIC,
  80, 0x0,
  AI_SHAPE_INIT(4, 1, 8, 1, 1), AI_STRIDE_INIT(4, 4, 4, 32, 32),
  1, &_features_features_1_block_block_1_fc1_Conv_output_0_bias_array, NULL)

/* Tensor #81 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_1_block_block_1_fc1_Conv_output_0_output, AI_STATIC,
  81, 0x0,
  AI_SHAPE_INIT(4, 1, 8, 1, 1), AI_STRIDE_INIT(4, 4, 4, 32, 32),
  1, &_features_features_1_block_block_1_fc1_Conv_output_0_output_array, NULL)

/* Tensor #82 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_1_block_block_1_fc1_Conv_output_0_scratch0, AI_STATIC,
  82, 0x0,
  AI_SHAPE_INIT(4, 1, 16, 1, 1), AI_STRIDE_INIT(4, 4, 4, 64, 64),
  1, &_features_features_1_block_block_1_fc1_Conv_output_0_scratch0_array, NULL)

/* Tensor #83 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_1_block_block_1_fc1_Conv_output_0_weights, AI_STATIC,
  83, 0x0,
  AI_SHAPE_INIT(4, 16, 1, 1, 8), AI_STRIDE_INIT(4, 4, 64, 512, 512),
  1, &_features_features_1_block_block_1_fc1_Conv_output_0_weights_array, NULL)

/* Tensor #84 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_1_block_block_1_fc2_Conv_output_0_bias, AI_STATIC,
  84, 0x0,
  AI_SHAPE_INIT(4, 1, 16, 1, 1), AI_STRIDE_INIT(4, 4, 4, 64, 64),
  1, &_features_features_1_block_block_1_fc2_Conv_output_0_bias_array, NULL)

/* Tensor #85 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_1_block_block_1_fc2_Conv_output_0_output, AI_STATIC,
  85, 0x0,
  AI_SHAPE_INIT(4, 1, 16, 1, 1), AI_STRIDE_INIT(4, 4, 4, 64, 64),
  1, &_features_features_1_block_block_1_fc2_Conv_output_0_output_array, NULL)

/* Tensor #86 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_1_block_block_1_fc2_Conv_output_0_scratch0, AI_STATIC,
  86, 0x0,
  AI_SHAPE_INIT(4, 1, 8, 1, 1), AI_STRIDE_INIT(4, 4, 4, 32, 32),
  1, &_features_features_1_block_block_1_fc2_Conv_output_0_scratch0_array, NULL)

/* Tensor #87 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_1_block_block_1_fc2_Conv_output_0_weights, AI_STATIC,
  87, 0x0,
  AI_SHAPE_INIT(4, 8, 1, 1, 16), AI_STRIDE_INIT(4, 4, 32, 512, 512),
  1, &_features_features_1_block_block_1_fc2_Conv_output_0_weights_array, NULL)

/* Tensor #88 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_1_block_block_1_scale_activation_HardSigmoid_output_0_output, AI_STATIC,
  88, 0x0,
  AI_SHAPE_INIT(4, 1, 16, 1, 1), AI_STRIDE_INIT(4, 4, 4, 64, 64),
  1, &_features_features_1_block_block_1_scale_activation_HardSigmoid_output_0_output_array, NULL)

/* Tensor #89 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_1_block_block_2_block_2_0_Conv_output_0_bias, AI_STATIC,
  89, 0x0,
  AI_SHAPE_INIT(4, 1, 16, 1, 1), AI_STRIDE_INIT(4, 4, 4, 64, 64),
  1, &_features_features_1_block_block_2_block_2_0_Conv_output_0_bias_array, NULL)

/* Tensor #90 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_1_block_block_2_block_2_0_Conv_output_0_output, AI_STATIC,
  90, 0x0,
  AI_SHAPE_INIT(4, 1, 16, 172, 8), AI_STRIDE_INIT(4, 4, 4, 64, 11008),
  1, &_features_features_1_block_block_2_block_2_0_Conv_output_0_output_array, NULL)

/* Tensor #91 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_1_block_block_2_block_2_0_Conv_output_0_scratch0, AI_STATIC,
  91, 0x0,
  AI_SHAPE_INIT(4, 1, 16, 1, 1), AI_STRIDE_INIT(4, 4, 4, 64, 64),
  1, &_features_features_1_block_block_2_block_2_0_Conv_output_0_scratch0_array, NULL)

/* Tensor #92 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_1_block_block_2_block_2_0_Conv_output_0_weights, AI_STATIC,
  92, 0x0,
  AI_SHAPE_INIT(4, 16, 1, 1, 16), AI_STRIDE_INIT(4, 4, 64, 1024, 1024),
  1, &_features_features_1_block_block_2_block_2_0_Conv_output_0_weights_array, NULL)

/* Tensor #93 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_2_block_block_0_block_0_0_Conv_output_0_bias, AI_STATIC,
  93, 0x0,
  AI_SHAPE_INIT(4, 1, 72, 1, 1), AI_STRIDE_INIT(4, 4, 4, 288, 288),
  1, &_features_features_2_block_block_0_block_0_0_Conv_output_0_bias_array, NULL)

/* Tensor #94 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_2_block_block_0_block_0_0_Conv_output_0_output, AI_STATIC,
  94, 0x0,
  AI_SHAPE_INIT(4, 1, 72, 172, 8), AI_STRIDE_INIT(4, 4, 4, 288, 49536),
  1, &_features_features_2_block_block_0_block_0_0_Conv_output_0_output_array, NULL)

/* Tensor #95 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_2_block_block_0_block_0_0_Conv_output_0_scratch0, AI_STATIC,
  95, 0x0,
  AI_SHAPE_INIT(4, 1, 16, 1, 1), AI_STRIDE_INIT(4, 4, 4, 64, 64),
  1, &_features_features_2_block_block_0_block_0_0_Conv_output_0_scratch0_array, NULL)

/* Tensor #96 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_2_block_block_0_block_0_0_Conv_output_0_weights, AI_STATIC,
  96, 0x0,
  AI_SHAPE_INIT(4, 16, 1, 1, 72), AI_STRIDE_INIT(4, 4, 64, 4608, 4608),
  1, &_features_features_2_block_block_0_block_0_0_Conv_output_0_weights_array, NULL)

/* Tensor #97 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_2_block_block_0_block_0_2_Relu_output_0_output, AI_STATIC,
  97, 0x0,
  AI_SHAPE_INIT(4, 1, 72, 172, 8), AI_STRIDE_INIT(4, 4, 4, 288, 49536),
  1, &_features_features_2_block_block_0_block_0_2_Relu_output_0_output_array, NULL)

/* Tensor #98 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_2_block_block_1_block_1_0_Conv_output_0_bias, AI_STATIC,
  98, 0x0,
  AI_SHAPE_INIT(4, 1, 72, 1, 1), AI_STRIDE_INIT(4, 4, 4, 288, 288),
  1, &_features_features_2_block_block_1_block_1_0_Conv_output_0_bias_array, NULL)

/* Tensor #99 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_2_block_block_1_block_1_0_Conv_output_0_output, AI_STATIC,
  99, 0x0,
  AI_SHAPE_INIT(4, 1, 72, 86, 4), AI_STRIDE_INIT(4, 4, 4, 288, 24768),
  1, &_features_features_2_block_block_1_block_1_0_Conv_output_0_output_array, NULL)

/* Tensor #100 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_2_block_block_1_block_1_0_Conv_output_0_weights, AI_STATIC,
  100, 0x0,
  AI_SHAPE_INIT(4, 1, 3, 3, 72), AI_STRIDE_INIT(4, 1, 72, 72, 72),
  1, &_features_features_2_block_block_1_block_1_0_Conv_output_0_weights_array, NULL)

/* Tensor #101 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_2_block_block_1_block_1_2_Relu_output_0_output, AI_STATIC,
  101, 0x0,
  AI_SHAPE_INIT(4, 1, 72, 86, 4), AI_STRIDE_INIT(4, 4, 4, 288, 24768),
  1, &_features_features_2_block_block_1_block_1_2_Relu_output_0_output_array, NULL)

/* Tensor #102 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_2_block_block_2_block_2_0_Conv_output_0_bias, AI_STATIC,
  102, 0x0,
  AI_SHAPE_INIT(4, 1, 24, 1, 1), AI_STRIDE_INIT(4, 4, 4, 96, 96),
  1, &_features_features_2_block_block_2_block_2_0_Conv_output_0_bias_array, NULL)

/* Tensor #103 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_2_block_block_2_block_2_0_Conv_output_0_output, AI_STATIC,
  103, 0x0,
  AI_SHAPE_INIT(4, 1, 24, 86, 4), AI_STRIDE_INIT(4, 4, 4, 96, 8256),
  1, &_features_features_2_block_block_2_block_2_0_Conv_output_0_output_array, NULL)

/* Tensor #104 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_2_block_block_2_block_2_0_Conv_output_0_scratch0, AI_STATIC,
  104, 0x0,
  AI_SHAPE_INIT(4, 1, 72, 1, 1), AI_STRIDE_INIT(4, 4, 4, 288, 288),
  1, &_features_features_2_block_block_2_block_2_0_Conv_output_0_scratch0_array, NULL)

/* Tensor #105 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_2_block_block_2_block_2_0_Conv_output_0_weights, AI_STATIC,
  105, 0x0,
  AI_SHAPE_INIT(4, 72, 1, 1, 24), AI_STRIDE_INIT(4, 4, 288, 6912, 6912),
  1, &_features_features_2_block_block_2_block_2_0_Conv_output_0_weights_array, NULL)

/* Tensor #106 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_3_Add_output_0_output, AI_STATIC,
  106, 0x0,
  AI_SHAPE_INIT(4, 1, 24, 86, 4), AI_STRIDE_INIT(4, 4, 4, 96, 8256),
  1, &_features_features_3_Add_output_0_output_array, NULL)

/* Tensor #107 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_3_block_block_0_block_0_0_Conv_output_0_bias, AI_STATIC,
  107, 0x0,
  AI_SHAPE_INIT(4, 1, 88, 1, 1), AI_STRIDE_INIT(4, 4, 4, 352, 352),
  1, &_features_features_3_block_block_0_block_0_0_Conv_output_0_bias_array, NULL)

/* Tensor #108 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_3_block_block_0_block_0_0_Conv_output_0_output, AI_STATIC,
  108, 0x0,
  AI_SHAPE_INIT(4, 1, 88, 86, 4), AI_STRIDE_INIT(4, 4, 4, 352, 30272),
  1, &_features_features_3_block_block_0_block_0_0_Conv_output_0_output_array, NULL)

/* Tensor #109 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_3_block_block_0_block_0_0_Conv_output_0_scratch0, AI_STATIC,
  109, 0x0,
  AI_SHAPE_INIT(4, 1, 24, 1, 1), AI_STRIDE_INIT(4, 4, 4, 96, 96),
  1, &_features_features_3_block_block_0_block_0_0_Conv_output_0_scratch0_array, NULL)

/* Tensor #110 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_3_block_block_0_block_0_0_Conv_output_0_weights, AI_STATIC,
  110, 0x0,
  AI_SHAPE_INIT(4, 24, 1, 1, 88), AI_STRIDE_INIT(4, 4, 96, 8448, 8448),
  1, &_features_features_3_block_block_0_block_0_0_Conv_output_0_weights_array, NULL)

/* Tensor #111 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_3_block_block_0_block_0_2_Relu_output_0_output, AI_STATIC,
  111, 0x0,
  AI_SHAPE_INIT(4, 1, 88, 86, 4), AI_STRIDE_INIT(4, 4, 4, 352, 30272),
  1, &_features_features_3_block_block_0_block_0_2_Relu_output_0_output_array, NULL)

/* Tensor #112 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_3_block_block_1_block_1_0_Conv_output_0_bias, AI_STATIC,
  112, 0x0,
  AI_SHAPE_INIT(4, 1, 88, 1, 1), AI_STRIDE_INIT(4, 4, 4, 352, 352),
  1, &_features_features_3_block_block_1_block_1_0_Conv_output_0_bias_array, NULL)

/* Tensor #113 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_3_block_block_1_block_1_0_Conv_output_0_output, AI_STATIC,
  113, 0x0,
  AI_SHAPE_INIT(4, 1, 88, 86, 4), AI_STRIDE_INIT(4, 4, 4, 352, 30272),
  1, &_features_features_3_block_block_1_block_1_0_Conv_output_0_output_array, NULL)

/* Tensor #114 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_3_block_block_1_block_1_0_Conv_output_0_weights, AI_STATIC,
  114, 0x0,
  AI_SHAPE_INIT(4, 1, 3, 3, 88), AI_STRIDE_INIT(4, 1, 88, 88, 88),
  1, &_features_features_3_block_block_1_block_1_0_Conv_output_0_weights_array, NULL)

/* Tensor #115 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_3_block_block_1_block_1_2_Relu_output_0_output, AI_STATIC,
  115, 0x0,
  AI_SHAPE_INIT(4, 1, 88, 86, 4), AI_STRIDE_INIT(4, 4, 4, 352, 30272),
  1, &_features_features_3_block_block_1_block_1_2_Relu_output_0_output_array, NULL)

/* Tensor #116 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_3_block_block_2_block_2_0_Conv_output_0_bias, AI_STATIC,
  116, 0x0,
  AI_SHAPE_INIT(4, 1, 24, 1, 1), AI_STRIDE_INIT(4, 4, 4, 96, 96),
  1, &_features_features_3_block_block_2_block_2_0_Conv_output_0_bias_array, NULL)

/* Tensor #117 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_3_block_block_2_block_2_0_Conv_output_0_output, AI_STATIC,
  117, 0x0,
  AI_SHAPE_INIT(4, 1, 24, 86, 4), AI_STRIDE_INIT(4, 4, 4, 96, 8256),
  1, &_features_features_3_block_block_2_block_2_0_Conv_output_0_output_array, NULL)

/* Tensor #118 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_3_block_block_2_block_2_0_Conv_output_0_scratch0, AI_STATIC,
  118, 0x0,
  AI_SHAPE_INIT(4, 1, 88, 1, 1), AI_STRIDE_INIT(4, 4, 4, 352, 352),
  1, &_features_features_3_block_block_2_block_2_0_Conv_output_0_scratch0_array, NULL)

/* Tensor #119 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_3_block_block_2_block_2_0_Conv_output_0_weights, AI_STATIC,
  119, 0x0,
  AI_SHAPE_INIT(4, 88, 1, 1, 24), AI_STRIDE_INIT(4, 4, 352, 8448, 8448),
  1, &_features_features_3_block_block_2_block_2_0_Conv_output_0_weights_array, NULL)

/* Tensor #120 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_4_block_block_0_block_0_0_Conv_output_0_bias, AI_STATIC,
  120, 0x0,
  AI_SHAPE_INIT(4, 1, 96, 1, 1), AI_STRIDE_INIT(4, 4, 4, 384, 384),
  1, &_features_features_4_block_block_0_block_0_0_Conv_output_0_bias_array, NULL)

/* Tensor #121 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_4_block_block_0_block_0_0_Conv_output_0_output, AI_STATIC,
  121, 0x0,
  AI_SHAPE_INIT(4, 1, 96, 86, 4), AI_STRIDE_INIT(4, 4, 4, 384, 33024),
  1, &_features_features_4_block_block_0_block_0_0_Conv_output_0_output_array, NULL)

/* Tensor #122 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_4_block_block_0_block_0_0_Conv_output_0_scratch0, AI_STATIC,
  122, 0x0,
  AI_SHAPE_INIT(4, 1, 24, 1, 1), AI_STRIDE_INIT(4, 4, 4, 96, 96),
  1, &_features_features_4_block_block_0_block_0_0_Conv_output_0_scratch0_array, NULL)

/* Tensor #123 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_4_block_block_0_block_0_0_Conv_output_0_weights, AI_STATIC,
  123, 0x0,
  AI_SHAPE_INIT(4, 24, 1, 1, 96), AI_STRIDE_INIT(4, 4, 96, 9216, 9216),
  1, &_features_features_4_block_block_0_block_0_0_Conv_output_0_weights_array, NULL)

/* Tensor #124 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_4_block_block_0_block_0_2_HardSigmoid_output_0_output, AI_STATIC,
  124, 0x0,
  AI_SHAPE_INIT(4, 1, 96, 86, 4), AI_STRIDE_INIT(4, 4, 4, 384, 33024),
  1, &_features_features_4_block_block_0_block_0_2_HardSigmoid_output_0_output_array, NULL)

/* Tensor #125 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_4_block_block_0_block_0_2_Mul_output_0_output, AI_STATIC,
  125, 0x0,
  AI_SHAPE_INIT(4, 1, 96, 86, 4), AI_STRIDE_INIT(4, 4, 4, 384, 33024),
  1, &_features_features_4_block_block_0_block_0_2_Mul_output_0_output_array, NULL)

/* Tensor #126 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_4_block_block_1_block_1_0_Conv_output_0_bias, AI_STATIC,
  126, 0x0,
  AI_SHAPE_INIT(4, 1, 96, 1, 1), AI_STRIDE_INIT(4, 4, 4, 384, 384),
  1, &_features_features_4_block_block_1_block_1_0_Conv_output_0_bias_array, NULL)

/* Tensor #127 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_4_block_block_1_block_1_0_Conv_output_0_output, AI_STATIC,
  127, 0x0,
  AI_SHAPE_INIT(4, 1, 96, 43, 2), AI_STRIDE_INIT(4, 4, 4, 384, 16512),
  1, &_features_features_4_block_block_1_block_1_0_Conv_output_0_output_array, NULL)

/* Tensor #128 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_4_block_block_1_block_1_0_Conv_output_0_weights, AI_STATIC,
  128, 0x0,
  AI_SHAPE_INIT(4, 1, 5, 5, 96), AI_STRIDE_INIT(4, 1, 96, 96, 96),
  1, &_features_features_4_block_block_1_block_1_0_Conv_output_0_weights_array, NULL)

/* Tensor #129 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_4_block_block_1_block_1_2_HardSigmoid_output_0_output, AI_STATIC,
  129, 0x0,
  AI_SHAPE_INIT(4, 1, 96, 43, 2), AI_STRIDE_INIT(4, 4, 4, 384, 16512),
  1, &_features_features_4_block_block_1_block_1_2_HardSigmoid_output_0_output_array, NULL)

/* Tensor #130 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_4_block_block_1_block_1_2_Mul_output_0_output, AI_STATIC,
  130, 0x0,
  AI_SHAPE_INIT(4, 1, 96, 43, 2), AI_STRIDE_INIT(4, 4, 4, 384, 16512),
  1, &_features_features_4_block_block_1_block_1_2_Mul_output_0_output_array, NULL)

/* Tensor #131 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_4_block_block_2_Mul_output_0_output, AI_STATIC,
  131, 0x0,
  AI_SHAPE_INIT(4, 1, 96, 43, 2), AI_STRIDE_INIT(4, 4, 4, 384, 16512),
  1, &_features_features_4_block_block_2_Mul_output_0_output_array, NULL)

/* Tensor #132 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_4_block_block_2_activation_Relu_output_0_output, AI_STATIC,
  132, 0x0,
  AI_SHAPE_INIT(4, 1, 24, 1, 1), AI_STRIDE_INIT(4, 4, 4, 96, 96),
  1, &_features_features_4_block_block_2_activation_Relu_output_0_output_array, NULL)

/* Tensor #133 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_4_block_block_2_avgpool_GlobalAveragePool_output_0_output, AI_STATIC,
  133, 0x0,
  AI_SHAPE_INIT(4, 1, 96, 1, 1), AI_STRIDE_INIT(4, 4, 4, 384, 384),
  1, &_features_features_4_block_block_2_avgpool_GlobalAveragePool_output_0_output_array, NULL)

/* Tensor #134 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_4_block_block_2_fc1_Conv_output_0_bias, AI_STATIC,
  134, 0x0,
  AI_SHAPE_INIT(4, 1, 24, 1, 1), AI_STRIDE_INIT(4, 4, 4, 96, 96),
  1, &_features_features_4_block_block_2_fc1_Conv_output_0_bias_array, NULL)

/* Tensor #135 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_4_block_block_2_fc1_Conv_output_0_output, AI_STATIC,
  135, 0x0,
  AI_SHAPE_INIT(4, 1, 24, 1, 1), AI_STRIDE_INIT(4, 4, 4, 96, 96),
  1, &_features_features_4_block_block_2_fc1_Conv_output_0_output_array, NULL)

/* Tensor #136 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_4_block_block_2_fc1_Conv_output_0_scratch0, AI_STATIC,
  136, 0x0,
  AI_SHAPE_INIT(4, 1, 96, 1, 1), AI_STRIDE_INIT(4, 4, 4, 384, 384),
  1, &_features_features_4_block_block_2_fc1_Conv_output_0_scratch0_array, NULL)

/* Tensor #137 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_4_block_block_2_fc1_Conv_output_0_weights, AI_STATIC,
  137, 0x0,
  AI_SHAPE_INIT(4, 96, 1, 1, 24), AI_STRIDE_INIT(4, 4, 384, 9216, 9216),
  1, &_features_features_4_block_block_2_fc1_Conv_output_0_weights_array, NULL)

/* Tensor #138 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_4_block_block_2_fc2_Conv_output_0_bias, AI_STATIC,
  138, 0x0,
  AI_SHAPE_INIT(4, 1, 96, 1, 1), AI_STRIDE_INIT(4, 4, 4, 384, 384),
  1, &_features_features_4_block_block_2_fc2_Conv_output_0_bias_array, NULL)

/* Tensor #139 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_4_block_block_2_fc2_Conv_output_0_output, AI_STATIC,
  139, 0x0,
  AI_SHAPE_INIT(4, 1, 96, 1, 1), AI_STRIDE_INIT(4, 4, 4, 384, 384),
  1, &_features_features_4_block_block_2_fc2_Conv_output_0_output_array, NULL)

/* Tensor #140 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_4_block_block_2_fc2_Conv_output_0_scratch0, AI_STATIC,
  140, 0x0,
  AI_SHAPE_INIT(4, 1, 24, 1, 1), AI_STRIDE_INIT(4, 4, 4, 96, 96),
  1, &_features_features_4_block_block_2_fc2_Conv_output_0_scratch0_array, NULL)

/* Tensor #141 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_4_block_block_2_fc2_Conv_output_0_weights, AI_STATIC,
  141, 0x0,
  AI_SHAPE_INIT(4, 24, 1, 1, 96), AI_STRIDE_INIT(4, 4, 96, 9216, 9216),
  1, &_features_features_4_block_block_2_fc2_Conv_output_0_weights_array, NULL)

/* Tensor #142 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_4_block_block_2_scale_activation_HardSigmoid_output_0_output, AI_STATIC,
  142, 0x0,
  AI_SHAPE_INIT(4, 1, 96, 1, 1), AI_STRIDE_INIT(4, 4, 4, 384, 384),
  1, &_features_features_4_block_block_2_scale_activation_HardSigmoid_output_0_output_array, NULL)

/* Tensor #143 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_4_block_block_3_block_3_0_Conv_output_0_bias, AI_STATIC,
  143, 0x0,
  AI_SHAPE_INIT(4, 1, 40, 1, 1), AI_STRIDE_INIT(4, 4, 4, 160, 160),
  1, &_features_features_4_block_block_3_block_3_0_Conv_output_0_bias_array, NULL)

/* Tensor #144 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_4_block_block_3_block_3_0_Conv_output_0_output, AI_STATIC,
  144, 0x0,
  AI_SHAPE_INIT(4, 1, 40, 43, 2), AI_STRIDE_INIT(4, 4, 4, 160, 6880),
  1, &_features_features_4_block_block_3_block_3_0_Conv_output_0_output_array, NULL)

/* Tensor #145 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_4_block_block_3_block_3_0_Conv_output_0_scratch0, AI_STATIC,
  145, 0x0,
  AI_SHAPE_INIT(4, 1, 96, 1, 1), AI_STRIDE_INIT(4, 4, 4, 384, 384),
  1, &_features_features_4_block_block_3_block_3_0_Conv_output_0_scratch0_array, NULL)

/* Tensor #146 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_4_block_block_3_block_3_0_Conv_output_0_weights, AI_STATIC,
  146, 0x0,
  AI_SHAPE_INIT(4, 96, 1, 1, 40), AI_STRIDE_INIT(4, 4, 384, 15360, 15360),
  1, &_features_features_4_block_block_3_block_3_0_Conv_output_0_weights_array, NULL)

/* Tensor #147 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_5_Add_output_0_output, AI_STATIC,
  147, 0x0,
  AI_SHAPE_INIT(4, 1, 40, 43, 2), AI_STRIDE_INIT(4, 4, 4, 160, 6880),
  1, &_features_features_5_Add_output_0_output_array, NULL)

/* Tensor #148 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_5_block_block_0_block_0_0_Conv_output_0_bias, AI_STATIC,
  148, 0x0,
  AI_SHAPE_INIT(4, 1, 240, 1, 1), AI_STRIDE_INIT(4, 4, 4, 960, 960),
  1, &_features_features_5_block_block_0_block_0_0_Conv_output_0_bias_array, NULL)

/* Tensor #149 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_5_block_block_0_block_0_0_Conv_output_0_output, AI_STATIC,
  149, 0x0,
  AI_SHAPE_INIT(4, 1, 240, 43, 2), AI_STRIDE_INIT(4, 4, 4, 960, 41280),
  1, &_features_features_5_block_block_0_block_0_0_Conv_output_0_output_array, NULL)

/* Tensor #150 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_5_block_block_0_block_0_0_Conv_output_0_scratch0, AI_STATIC,
  150, 0x0,
  AI_SHAPE_INIT(4, 1, 40, 1, 1), AI_STRIDE_INIT(4, 4, 4, 160, 160),
  1, &_features_features_5_block_block_0_block_0_0_Conv_output_0_scratch0_array, NULL)

/* Tensor #151 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_5_block_block_0_block_0_0_Conv_output_0_weights, AI_STATIC,
  151, 0x0,
  AI_SHAPE_INIT(4, 40, 1, 1, 240), AI_STRIDE_INIT(4, 4, 160, 38400, 38400),
  1, &_features_features_5_block_block_0_block_0_0_Conv_output_0_weights_array, NULL)

/* Tensor #152 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_5_block_block_0_block_0_2_HardSigmoid_output_0_output, AI_STATIC,
  152, 0x0,
  AI_SHAPE_INIT(4, 1, 240, 43, 2), AI_STRIDE_INIT(4, 4, 4, 960, 41280),
  1, &_features_features_5_block_block_0_block_0_2_HardSigmoid_output_0_output_array, NULL)

/* Tensor #153 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_5_block_block_0_block_0_2_Mul_output_0_output, AI_STATIC,
  153, 0x0,
  AI_SHAPE_INIT(4, 1, 240, 43, 2), AI_STRIDE_INIT(4, 4, 4, 960, 41280),
  1, &_features_features_5_block_block_0_block_0_2_Mul_output_0_output_array, NULL)

/* Tensor #154 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_5_block_block_1_block_1_0_Conv_output_0_bias, AI_STATIC,
  154, 0x0,
  AI_SHAPE_INIT(4, 1, 240, 1, 1), AI_STRIDE_INIT(4, 4, 4, 960, 960),
  1, &_features_features_5_block_block_1_block_1_0_Conv_output_0_bias_array, NULL)

/* Tensor #155 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_5_block_block_1_block_1_0_Conv_output_0_output, AI_STATIC,
  155, 0x0,
  AI_SHAPE_INIT(4, 1, 240, 43, 2), AI_STRIDE_INIT(4, 4, 4, 960, 41280),
  1, &_features_features_5_block_block_1_block_1_0_Conv_output_0_output_array, NULL)

/* Tensor #156 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_5_block_block_1_block_1_0_Conv_output_0_weights, AI_STATIC,
  156, 0x0,
  AI_SHAPE_INIT(4, 1, 5, 5, 240), AI_STRIDE_INIT(4, 1, 240, 240, 240),
  1, &_features_features_5_block_block_1_block_1_0_Conv_output_0_weights_array, NULL)

/* Tensor #157 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_5_block_block_1_block_1_2_HardSigmoid_output_0_output, AI_STATIC,
  157, 0x0,
  AI_SHAPE_INIT(4, 1, 240, 43, 2), AI_STRIDE_INIT(4, 4, 4, 960, 41280),
  1, &_features_features_5_block_block_1_block_1_2_HardSigmoid_output_0_output_array, NULL)

/* Tensor #158 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_5_block_block_1_block_1_2_Mul_output_0_output, AI_STATIC,
  158, 0x0,
  AI_SHAPE_INIT(4, 1, 240, 43, 2), AI_STRIDE_INIT(4, 4, 4, 960, 41280),
  1, &_features_features_5_block_block_1_block_1_2_Mul_output_0_output_array, NULL)

/* Tensor #159 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_5_block_block_2_Mul_output_0_output, AI_STATIC,
  159, 0x0,
  AI_SHAPE_INIT(4, 1, 240, 43, 2), AI_STRIDE_INIT(4, 4, 4, 960, 41280),
  1, &_features_features_5_block_block_2_Mul_output_0_output_array, NULL)

/* Tensor #160 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_5_block_block_2_activation_Relu_output_0_output, AI_STATIC,
  160, 0x0,
  AI_SHAPE_INIT(4, 1, 64, 1, 1), AI_STRIDE_INIT(4, 4, 4, 256, 256),
  1, &_features_features_5_block_block_2_activation_Relu_output_0_output_array, NULL)

/* Tensor #161 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_5_block_block_2_avgpool_GlobalAveragePool_output_0_output, AI_STATIC,
  161, 0x0,
  AI_SHAPE_INIT(4, 1, 240, 1, 1), AI_STRIDE_INIT(4, 4, 4, 960, 960),
  1, &_features_features_5_block_block_2_avgpool_GlobalAveragePool_output_0_output_array, NULL)

/* Tensor #162 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_5_block_block_2_fc1_Conv_output_0_bias, AI_STATIC,
  162, 0x0,
  AI_SHAPE_INIT(4, 1, 64, 1, 1), AI_STRIDE_INIT(4, 4, 4, 256, 256),
  1, &_features_features_5_block_block_2_fc1_Conv_output_0_bias_array, NULL)

/* Tensor #163 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_5_block_block_2_fc1_Conv_output_0_output, AI_STATIC,
  163, 0x0,
  AI_SHAPE_INIT(4, 1, 64, 1, 1), AI_STRIDE_INIT(4, 4, 4, 256, 256),
  1, &_features_features_5_block_block_2_fc1_Conv_output_0_output_array, NULL)

/* Tensor #164 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_5_block_block_2_fc1_Conv_output_0_scratch0, AI_STATIC,
  164, 0x0,
  AI_SHAPE_INIT(4, 1, 240, 1, 1), AI_STRIDE_INIT(4, 4, 4, 960, 960),
  1, &_features_features_5_block_block_2_fc1_Conv_output_0_scratch0_array, NULL)

/* Tensor #165 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_5_block_block_2_fc1_Conv_output_0_weights, AI_STATIC,
  165, 0x0,
  AI_SHAPE_INIT(4, 240, 1, 1, 64), AI_STRIDE_INIT(4, 4, 960, 61440, 61440),
  1, &_features_features_5_block_block_2_fc1_Conv_output_0_weights_array, NULL)

/* Tensor #166 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_5_block_block_2_fc2_Conv_output_0_bias, AI_STATIC,
  166, 0x0,
  AI_SHAPE_INIT(4, 1, 240, 1, 1), AI_STRIDE_INIT(4, 4, 4, 960, 960),
  1, &_features_features_5_block_block_2_fc2_Conv_output_0_bias_array, NULL)

/* Tensor #167 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_5_block_block_2_fc2_Conv_output_0_output, AI_STATIC,
  167, 0x0,
  AI_SHAPE_INIT(4, 1, 240, 1, 1), AI_STRIDE_INIT(4, 4, 4, 960, 960),
  1, &_features_features_5_block_block_2_fc2_Conv_output_0_output_array, NULL)

/* Tensor #168 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_5_block_block_2_fc2_Conv_output_0_scratch0, AI_STATIC,
  168, 0x0,
  AI_SHAPE_INIT(4, 1, 64, 1, 1), AI_STRIDE_INIT(4, 4, 4, 256, 256),
  1, &_features_features_5_block_block_2_fc2_Conv_output_0_scratch0_array, NULL)

/* Tensor #169 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_5_block_block_2_fc2_Conv_output_0_weights, AI_STATIC,
  169, 0x0,
  AI_SHAPE_INIT(4, 64, 1, 1, 240), AI_STRIDE_INIT(4, 4, 256, 61440, 61440),
  1, &_features_features_5_block_block_2_fc2_Conv_output_0_weights_array, NULL)

/* Tensor #170 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_5_block_block_2_scale_activation_HardSigmoid_output_0_output, AI_STATIC,
  170, 0x0,
  AI_SHAPE_INIT(4, 1, 240, 1, 1), AI_STRIDE_INIT(4, 4, 4, 960, 960),
  1, &_features_features_5_block_block_2_scale_activation_HardSigmoid_output_0_output_array, NULL)

/* Tensor #171 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_5_block_block_3_block_3_0_Conv_output_0_bias, AI_STATIC,
  171, 0x0,
  AI_SHAPE_INIT(4, 1, 40, 1, 1), AI_STRIDE_INIT(4, 4, 4, 160, 160),
  1, &_features_features_5_block_block_3_block_3_0_Conv_output_0_bias_array, NULL)

/* Tensor #172 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_5_block_block_3_block_3_0_Conv_output_0_output, AI_STATIC,
  172, 0x0,
  AI_SHAPE_INIT(4, 1, 40, 43, 2), AI_STRIDE_INIT(4, 4, 4, 160, 6880),
  1, &_features_features_5_block_block_3_block_3_0_Conv_output_0_output_array, NULL)

/* Tensor #173 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_5_block_block_3_block_3_0_Conv_output_0_scratch0, AI_STATIC,
  173, 0x0,
  AI_SHAPE_INIT(4, 1, 240, 1, 1), AI_STRIDE_INIT(4, 4, 4, 960, 960),
  1, &_features_features_5_block_block_3_block_3_0_Conv_output_0_scratch0_array, NULL)

/* Tensor #174 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_5_block_block_3_block_3_0_Conv_output_0_weights, AI_STATIC,
  174, 0x0,
  AI_SHAPE_INIT(4, 240, 1, 1, 40), AI_STRIDE_INIT(4, 4, 960, 38400, 38400),
  1, &_features_features_5_block_block_3_block_3_0_Conv_output_0_weights_array, NULL)

/* Tensor #175 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_6_Add_output_0_output, AI_STATIC,
  175, 0x0,
  AI_SHAPE_INIT(4, 1, 40, 43, 2), AI_STRIDE_INIT(4, 4, 4, 160, 6880),
  1, &_features_features_6_Add_output_0_output_array, NULL)

/* Tensor #176 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_6_block_block_0_block_0_0_Conv_output_0_bias, AI_STATIC,
  176, 0x0,
  AI_SHAPE_INIT(4, 1, 240, 1, 1), AI_STRIDE_INIT(4, 4, 4, 960, 960),
  1, &_features_features_6_block_block_0_block_0_0_Conv_output_0_bias_array, NULL)

/* Tensor #177 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_6_block_block_0_block_0_0_Conv_output_0_output, AI_STATIC,
  177, 0x0,
  AI_SHAPE_INIT(4, 1, 240, 43, 2), AI_STRIDE_INIT(4, 4, 4, 960, 41280),
  1, &_features_features_6_block_block_0_block_0_0_Conv_output_0_output_array, NULL)

/* Tensor #178 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_6_block_block_0_block_0_0_Conv_output_0_scratch0, AI_STATIC,
  178, 0x0,
  AI_SHAPE_INIT(4, 1, 40, 1, 1), AI_STRIDE_INIT(4, 4, 4, 160, 160),
  1, &_features_features_6_block_block_0_block_0_0_Conv_output_0_scratch0_array, NULL)

/* Tensor #179 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_6_block_block_0_block_0_0_Conv_output_0_weights, AI_STATIC,
  179, 0x0,
  AI_SHAPE_INIT(4, 40, 1, 1, 240), AI_STRIDE_INIT(4, 4, 160, 38400, 38400),
  1, &_features_features_6_block_block_0_block_0_0_Conv_output_0_weights_array, NULL)

/* Tensor #180 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_6_block_block_0_block_0_2_HardSigmoid_output_0_output, AI_STATIC,
  180, 0x0,
  AI_SHAPE_INIT(4, 1, 240, 43, 2), AI_STRIDE_INIT(4, 4, 4, 960, 41280),
  1, &_features_features_6_block_block_0_block_0_2_HardSigmoid_output_0_output_array, NULL)

/* Tensor #181 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_6_block_block_0_block_0_2_Mul_output_0_output, AI_STATIC,
  181, 0x0,
  AI_SHAPE_INIT(4, 1, 240, 43, 2), AI_STRIDE_INIT(4, 4, 4, 960, 41280),
  1, &_features_features_6_block_block_0_block_0_2_Mul_output_0_output_array, NULL)

/* Tensor #182 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_6_block_block_1_block_1_0_Conv_output_0_bias, AI_STATIC,
  182, 0x0,
  AI_SHAPE_INIT(4, 1, 240, 1, 1), AI_STRIDE_INIT(4, 4, 4, 960, 960),
  1, &_features_features_6_block_block_1_block_1_0_Conv_output_0_bias_array, NULL)

/* Tensor #183 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_6_block_block_1_block_1_0_Conv_output_0_output, AI_STATIC,
  183, 0x0,
  AI_SHAPE_INIT(4, 1, 240, 43, 2), AI_STRIDE_INIT(4, 4, 4, 960, 41280),
  1, &_features_features_6_block_block_1_block_1_0_Conv_output_0_output_array, NULL)

/* Tensor #184 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_6_block_block_1_block_1_0_Conv_output_0_weights, AI_STATIC,
  184, 0x0,
  AI_SHAPE_INIT(4, 1, 5, 5, 240), AI_STRIDE_INIT(4, 1, 240, 240, 240),
  1, &_features_features_6_block_block_1_block_1_0_Conv_output_0_weights_array, NULL)

/* Tensor #185 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_6_block_block_1_block_1_2_HardSigmoid_output_0_output, AI_STATIC,
  185, 0x0,
  AI_SHAPE_INIT(4, 1, 240, 43, 2), AI_STRIDE_INIT(4, 4, 4, 960, 41280),
  1, &_features_features_6_block_block_1_block_1_2_HardSigmoid_output_0_output_array, NULL)

/* Tensor #186 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_6_block_block_1_block_1_2_Mul_output_0_output, AI_STATIC,
  186, 0x0,
  AI_SHAPE_INIT(4, 1, 240, 43, 2), AI_STRIDE_INIT(4, 4, 4, 960, 41280),
  1, &_features_features_6_block_block_1_block_1_2_Mul_output_0_output_array, NULL)

/* Tensor #187 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_6_block_block_2_Mul_output_0_output, AI_STATIC,
  187, 0x0,
  AI_SHAPE_INIT(4, 1, 240, 43, 2), AI_STRIDE_INIT(4, 4, 4, 960, 41280),
  1, &_features_features_6_block_block_2_Mul_output_0_output_array, NULL)

/* Tensor #188 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_6_block_block_2_activation_Relu_output_0_output, AI_STATIC,
  188, 0x0,
  AI_SHAPE_INIT(4, 1, 64, 1, 1), AI_STRIDE_INIT(4, 4, 4, 256, 256),
  1, &_features_features_6_block_block_2_activation_Relu_output_0_output_array, NULL)

/* Tensor #189 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_6_block_block_2_avgpool_GlobalAveragePool_output_0_output, AI_STATIC,
  189, 0x0,
  AI_SHAPE_INIT(4, 1, 240, 1, 1), AI_STRIDE_INIT(4, 4, 4, 960, 960),
  1, &_features_features_6_block_block_2_avgpool_GlobalAveragePool_output_0_output_array, NULL)

/* Tensor #190 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_6_block_block_2_fc1_Conv_output_0_bias, AI_STATIC,
  190, 0x0,
  AI_SHAPE_INIT(4, 1, 64, 1, 1), AI_STRIDE_INIT(4, 4, 4, 256, 256),
  1, &_features_features_6_block_block_2_fc1_Conv_output_0_bias_array, NULL)

/* Tensor #191 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_6_block_block_2_fc1_Conv_output_0_output, AI_STATIC,
  191, 0x0,
  AI_SHAPE_INIT(4, 1, 64, 1, 1), AI_STRIDE_INIT(4, 4, 4, 256, 256),
  1, &_features_features_6_block_block_2_fc1_Conv_output_0_output_array, NULL)

/* Tensor #192 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_6_block_block_2_fc1_Conv_output_0_scratch0, AI_STATIC,
  192, 0x0,
  AI_SHAPE_INIT(4, 1, 240, 1, 1), AI_STRIDE_INIT(4, 4, 4, 960, 960),
  1, &_features_features_6_block_block_2_fc1_Conv_output_0_scratch0_array, NULL)

/* Tensor #193 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_6_block_block_2_fc1_Conv_output_0_weights, AI_STATIC,
  193, 0x0,
  AI_SHAPE_INIT(4, 240, 1, 1, 64), AI_STRIDE_INIT(4, 4, 960, 61440, 61440),
  1, &_features_features_6_block_block_2_fc1_Conv_output_0_weights_array, NULL)

/* Tensor #194 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_6_block_block_2_fc2_Conv_output_0_bias, AI_STATIC,
  194, 0x0,
  AI_SHAPE_INIT(4, 1, 240, 1, 1), AI_STRIDE_INIT(4, 4, 4, 960, 960),
  1, &_features_features_6_block_block_2_fc2_Conv_output_0_bias_array, NULL)

/* Tensor #195 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_6_block_block_2_fc2_Conv_output_0_output, AI_STATIC,
  195, 0x0,
  AI_SHAPE_INIT(4, 1, 240, 1, 1), AI_STRIDE_INIT(4, 4, 4, 960, 960),
  1, &_features_features_6_block_block_2_fc2_Conv_output_0_output_array, NULL)

/* Tensor #196 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_6_block_block_2_fc2_Conv_output_0_scratch0, AI_STATIC,
  196, 0x0,
  AI_SHAPE_INIT(4, 1, 64, 1, 1), AI_STRIDE_INIT(4, 4, 4, 256, 256),
  1, &_features_features_6_block_block_2_fc2_Conv_output_0_scratch0_array, NULL)

/* Tensor #197 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_6_block_block_2_fc2_Conv_output_0_weights, AI_STATIC,
  197, 0x0,
  AI_SHAPE_INIT(4, 64, 1, 1, 240), AI_STRIDE_INIT(4, 4, 256, 61440, 61440),
  1, &_features_features_6_block_block_2_fc2_Conv_output_0_weights_array, NULL)

/* Tensor #198 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_6_block_block_2_scale_activation_HardSigmoid_output_0_output, AI_STATIC,
  198, 0x0,
  AI_SHAPE_INIT(4, 1, 240, 1, 1), AI_STRIDE_INIT(4, 4, 4, 960, 960),
  1, &_features_features_6_block_block_2_scale_activation_HardSigmoid_output_0_output_array, NULL)

/* Tensor #199 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_6_block_block_3_block_3_0_Conv_output_0_bias, AI_STATIC,
  199, 0x0,
  AI_SHAPE_INIT(4, 1, 40, 1, 1), AI_STRIDE_INIT(4, 4, 4, 160, 160),
  1, &_features_features_6_block_block_3_block_3_0_Conv_output_0_bias_array, NULL)

/* Tensor #200 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_6_block_block_3_block_3_0_Conv_output_0_output, AI_STATIC,
  200, 0x0,
  AI_SHAPE_INIT(4, 1, 40, 43, 2), AI_STRIDE_INIT(4, 4, 4, 160, 6880),
  1, &_features_features_6_block_block_3_block_3_0_Conv_output_0_output_array, NULL)

/* Tensor #201 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_6_block_block_3_block_3_0_Conv_output_0_scratch0, AI_STATIC,
  201, 0x0,
  AI_SHAPE_INIT(4, 1, 240, 1, 1), AI_STRIDE_INIT(4, 4, 4, 960, 960),
  1, &_features_features_6_block_block_3_block_3_0_Conv_output_0_scratch0_array, NULL)

/* Tensor #202 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_6_block_block_3_block_3_0_Conv_output_0_weights, AI_STATIC,
  202, 0x0,
  AI_SHAPE_INIT(4, 240, 1, 1, 40), AI_STRIDE_INIT(4, 4, 960, 38400, 38400),
  1, &_features_features_6_block_block_3_block_3_0_Conv_output_0_weights_array, NULL)

/* Tensor #203 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_7_block_block_0_block_0_0_Conv_output_0_bias, AI_STATIC,
  203, 0x0,
  AI_SHAPE_INIT(4, 1, 120, 1, 1), AI_STRIDE_INIT(4, 4, 4, 480, 480),
  1, &_features_features_7_block_block_0_block_0_0_Conv_output_0_bias_array, NULL)

/* Tensor #204 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_7_block_block_0_block_0_0_Conv_output_0_output, AI_STATIC,
  204, 0x0,
  AI_SHAPE_INIT(4, 1, 120, 43, 2), AI_STRIDE_INIT(4, 4, 4, 480, 20640),
  1, &_features_features_7_block_block_0_block_0_0_Conv_output_0_output_array, NULL)

/* Tensor #205 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_7_block_block_0_block_0_0_Conv_output_0_scratch0, AI_STATIC,
  205, 0x0,
  AI_SHAPE_INIT(4, 1, 40, 1, 1), AI_STRIDE_INIT(4, 4, 4, 160, 160),
  1, &_features_features_7_block_block_0_block_0_0_Conv_output_0_scratch0_array, NULL)

/* Tensor #206 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_7_block_block_0_block_0_0_Conv_output_0_weights, AI_STATIC,
  206, 0x0,
  AI_SHAPE_INIT(4, 40, 1, 1, 120), AI_STRIDE_INIT(4, 4, 160, 19200, 19200),
  1, &_features_features_7_block_block_0_block_0_0_Conv_output_0_weights_array, NULL)

/* Tensor #207 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_7_block_block_0_block_0_2_HardSigmoid_output_0_output, AI_STATIC,
  207, 0x0,
  AI_SHAPE_INIT(4, 1, 120, 43, 2), AI_STRIDE_INIT(4, 4, 4, 480, 20640),
  1, &_features_features_7_block_block_0_block_0_2_HardSigmoid_output_0_output_array, NULL)

/* Tensor #208 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_7_block_block_0_block_0_2_Mul_output_0_output, AI_STATIC,
  208, 0x0,
  AI_SHAPE_INIT(4, 1, 120, 43, 2), AI_STRIDE_INIT(4, 4, 4, 480, 20640),
  1, &_features_features_7_block_block_0_block_0_2_Mul_output_0_output_array, NULL)

/* Tensor #209 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_7_block_block_1_block_1_0_Conv_output_0_bias, AI_STATIC,
  209, 0x0,
  AI_SHAPE_INIT(4, 1, 120, 1, 1), AI_STRIDE_INIT(4, 4, 4, 480, 480),
  1, &_features_features_7_block_block_1_block_1_0_Conv_output_0_bias_array, NULL)

/* Tensor #210 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_7_block_block_1_block_1_0_Conv_output_0_output, AI_STATIC,
  210, 0x0,
  AI_SHAPE_INIT(4, 1, 120, 43, 2), AI_STRIDE_INIT(4, 4, 4, 480, 20640),
  1, &_features_features_7_block_block_1_block_1_0_Conv_output_0_output_array, NULL)

/* Tensor #211 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_7_block_block_1_block_1_0_Conv_output_0_weights, AI_STATIC,
  211, 0x0,
  AI_SHAPE_INIT(4, 1, 5, 5, 120), AI_STRIDE_INIT(4, 1, 120, 120, 120),
  1, &_features_features_7_block_block_1_block_1_0_Conv_output_0_weights_array, NULL)

/* Tensor #212 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_7_block_block_1_block_1_2_HardSigmoid_output_0_output, AI_STATIC,
  212, 0x0,
  AI_SHAPE_INIT(4, 1, 120, 43, 2), AI_STRIDE_INIT(4, 4, 4, 480, 20640),
  1, &_features_features_7_block_block_1_block_1_2_HardSigmoid_output_0_output_array, NULL)

/* Tensor #213 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_7_block_block_1_block_1_2_Mul_output_0_output, AI_STATIC,
  213, 0x0,
  AI_SHAPE_INIT(4, 1, 120, 43, 2), AI_STRIDE_INIT(4, 4, 4, 480, 20640),
  1, &_features_features_7_block_block_1_block_1_2_Mul_output_0_output_array, NULL)

/* Tensor #214 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_7_block_block_2_Mul_output_0_output, AI_STATIC,
  214, 0x0,
  AI_SHAPE_INIT(4, 1, 120, 43, 2), AI_STRIDE_INIT(4, 4, 4, 480, 20640),
  1, &_features_features_7_block_block_2_Mul_output_0_output_array, NULL)

/* Tensor #215 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_7_block_block_2_activation_Relu_output_0_output, AI_STATIC,
  215, 0x0,
  AI_SHAPE_INIT(4, 1, 32, 1, 1), AI_STRIDE_INIT(4, 4, 4, 128, 128),
  1, &_features_features_7_block_block_2_activation_Relu_output_0_output_array, NULL)

/* Tensor #216 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_7_block_block_2_avgpool_GlobalAveragePool_output_0_output, AI_STATIC,
  216, 0x0,
  AI_SHAPE_INIT(4, 1, 120, 1, 1), AI_STRIDE_INIT(4, 4, 4, 480, 480),
  1, &_features_features_7_block_block_2_avgpool_GlobalAveragePool_output_0_output_array, NULL)

/* Tensor #217 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_7_block_block_2_fc1_Conv_output_0_bias, AI_STATIC,
  217, 0x0,
  AI_SHAPE_INIT(4, 1, 32, 1, 1), AI_STRIDE_INIT(4, 4, 4, 128, 128),
  1, &_features_features_7_block_block_2_fc1_Conv_output_0_bias_array, NULL)

/* Tensor #218 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_7_block_block_2_fc1_Conv_output_0_output, AI_STATIC,
  218, 0x0,
  AI_SHAPE_INIT(4, 1, 32, 1, 1), AI_STRIDE_INIT(4, 4, 4, 128, 128),
  1, &_features_features_7_block_block_2_fc1_Conv_output_0_output_array, NULL)

/* Tensor #219 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_7_block_block_2_fc1_Conv_output_0_scratch0, AI_STATIC,
  219, 0x0,
  AI_SHAPE_INIT(4, 1, 120, 1, 1), AI_STRIDE_INIT(4, 4, 4, 480, 480),
  1, &_features_features_7_block_block_2_fc1_Conv_output_0_scratch0_array, NULL)

/* Tensor #220 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_7_block_block_2_fc1_Conv_output_0_weights, AI_STATIC,
  220, 0x0,
  AI_SHAPE_INIT(4, 120, 1, 1, 32), AI_STRIDE_INIT(4, 4, 480, 15360, 15360),
  1, &_features_features_7_block_block_2_fc1_Conv_output_0_weights_array, NULL)

/* Tensor #221 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_7_block_block_2_fc2_Conv_output_0_bias, AI_STATIC,
  221, 0x0,
  AI_SHAPE_INIT(4, 1, 120, 1, 1), AI_STRIDE_INIT(4, 4, 4, 480, 480),
  1, &_features_features_7_block_block_2_fc2_Conv_output_0_bias_array, NULL)

/* Tensor #222 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_7_block_block_2_fc2_Conv_output_0_output, AI_STATIC,
  222, 0x0,
  AI_SHAPE_INIT(4, 1, 120, 1, 1), AI_STRIDE_INIT(4, 4, 4, 480, 480),
  1, &_features_features_7_block_block_2_fc2_Conv_output_0_output_array, NULL)

/* Tensor #223 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_7_block_block_2_fc2_Conv_output_0_scratch0, AI_STATIC,
  223, 0x0,
  AI_SHAPE_INIT(4, 1, 32, 1, 1), AI_STRIDE_INIT(4, 4, 4, 128, 128),
  1, &_features_features_7_block_block_2_fc2_Conv_output_0_scratch0_array, NULL)

/* Tensor #224 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_7_block_block_2_fc2_Conv_output_0_weights, AI_STATIC,
  224, 0x0,
  AI_SHAPE_INIT(4, 32, 1, 1, 120), AI_STRIDE_INIT(4, 4, 128, 15360, 15360),
  1, &_features_features_7_block_block_2_fc2_Conv_output_0_weights_array, NULL)

/* Tensor #225 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_7_block_block_2_scale_activation_HardSigmoid_output_0_output, AI_STATIC,
  225, 0x0,
  AI_SHAPE_INIT(4, 1, 120, 1, 1), AI_STRIDE_INIT(4, 4, 4, 480, 480),
  1, &_features_features_7_block_block_2_scale_activation_HardSigmoid_output_0_output_array, NULL)

/* Tensor #226 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_7_block_block_3_block_3_0_Conv_output_0_bias, AI_STATIC,
  226, 0x0,
  AI_SHAPE_INIT(4, 1, 48, 1, 1), AI_STRIDE_INIT(4, 4, 4, 192, 192),
  1, &_features_features_7_block_block_3_block_3_0_Conv_output_0_bias_array, NULL)

/* Tensor #227 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_7_block_block_3_block_3_0_Conv_output_0_output, AI_STATIC,
  227, 0x0,
  AI_SHAPE_INIT(4, 1, 48, 43, 2), AI_STRIDE_INIT(4, 4, 4, 192, 8256),
  1, &_features_features_7_block_block_3_block_3_0_Conv_output_0_output_array, NULL)

/* Tensor #228 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_7_block_block_3_block_3_0_Conv_output_0_scratch0, AI_STATIC,
  228, 0x0,
  AI_SHAPE_INIT(4, 1, 120, 1, 1), AI_STRIDE_INIT(4, 4, 4, 480, 480),
  1, &_features_features_7_block_block_3_block_3_0_Conv_output_0_scratch0_array, NULL)

/* Tensor #229 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_7_block_block_3_block_3_0_Conv_output_0_weights, AI_STATIC,
  229, 0x0,
  AI_SHAPE_INIT(4, 120, 1, 1, 48), AI_STRIDE_INIT(4, 4, 480, 23040, 23040),
  1, &_features_features_7_block_block_3_block_3_0_Conv_output_0_weights_array, NULL)

/* Tensor #230 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_8_Add_output_0_output, AI_STATIC,
  230, 0x0,
  AI_SHAPE_INIT(4, 1, 48, 43, 2), AI_STRIDE_INIT(4, 4, 4, 192, 8256),
  1, &_features_features_8_Add_output_0_output_array, NULL)

/* Tensor #231 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_8_block_block_0_block_0_0_Conv_output_0_bias, AI_STATIC,
  231, 0x0,
  AI_SHAPE_INIT(4, 1, 144, 1, 1), AI_STRIDE_INIT(4, 4, 4, 576, 576),
  1, &_features_features_8_block_block_0_block_0_0_Conv_output_0_bias_array, NULL)

/* Tensor #232 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_8_block_block_0_block_0_0_Conv_output_0_output, AI_STATIC,
  232, 0x0,
  AI_SHAPE_INIT(4, 1, 144, 43, 2), AI_STRIDE_INIT(4, 4, 4, 576, 24768),
  1, &_features_features_8_block_block_0_block_0_0_Conv_output_0_output_array, NULL)

/* Tensor #233 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_8_block_block_0_block_0_0_Conv_output_0_scratch0, AI_STATIC,
  233, 0x0,
  AI_SHAPE_INIT(4, 1, 48, 1, 1), AI_STRIDE_INIT(4, 4, 4, 192, 192),
  1, &_features_features_8_block_block_0_block_0_0_Conv_output_0_scratch0_array, NULL)

/* Tensor #234 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_8_block_block_0_block_0_0_Conv_output_0_weights, AI_STATIC,
  234, 0x0,
  AI_SHAPE_INIT(4, 48, 1, 1, 144), AI_STRIDE_INIT(4, 4, 192, 27648, 27648),
  1, &_features_features_8_block_block_0_block_0_0_Conv_output_0_weights_array, NULL)

/* Tensor #235 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_8_block_block_0_block_0_2_HardSigmoid_output_0_output, AI_STATIC,
  235, 0x0,
  AI_SHAPE_INIT(4, 1, 144, 43, 2), AI_STRIDE_INIT(4, 4, 4, 576, 24768),
  1, &_features_features_8_block_block_0_block_0_2_HardSigmoid_output_0_output_array, NULL)

/* Tensor #236 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_8_block_block_0_block_0_2_Mul_output_0_output, AI_STATIC,
  236, 0x0,
  AI_SHAPE_INIT(4, 1, 144, 43, 2), AI_STRIDE_INIT(4, 4, 4, 576, 24768),
  1, &_features_features_8_block_block_0_block_0_2_Mul_output_0_output_array, NULL)

/* Tensor #237 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_8_block_block_1_block_1_0_Conv_output_0_bias, AI_STATIC,
  237, 0x0,
  AI_SHAPE_INIT(4, 1, 144, 1, 1), AI_STRIDE_INIT(4, 4, 4, 576, 576),
  1, &_features_features_8_block_block_1_block_1_0_Conv_output_0_bias_array, NULL)

/* Tensor #238 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_8_block_block_1_block_1_0_Conv_output_0_output, AI_STATIC,
  238, 0x0,
  AI_SHAPE_INIT(4, 1, 144, 43, 2), AI_STRIDE_INIT(4, 4, 4, 576, 24768),
  1, &_features_features_8_block_block_1_block_1_0_Conv_output_0_output_array, NULL)

/* Tensor #239 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_8_block_block_1_block_1_0_Conv_output_0_weights, AI_STATIC,
  239, 0x0,
  AI_SHAPE_INIT(4, 1, 5, 5, 144), AI_STRIDE_INIT(4, 1, 144, 144, 144),
  1, &_features_features_8_block_block_1_block_1_0_Conv_output_0_weights_array, NULL)

/* Tensor #240 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_8_block_block_1_block_1_2_HardSigmoid_output_0_output, AI_STATIC,
  240, 0x0,
  AI_SHAPE_INIT(4, 1, 144, 43, 2), AI_STRIDE_INIT(4, 4, 4, 576, 24768),
  1, &_features_features_8_block_block_1_block_1_2_HardSigmoid_output_0_output_array, NULL)

/* Tensor #241 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_8_block_block_1_block_1_2_Mul_output_0_output, AI_STATIC,
  241, 0x0,
  AI_SHAPE_INIT(4, 1, 144, 43, 2), AI_STRIDE_INIT(4, 4, 4, 576, 24768),
  1, &_features_features_8_block_block_1_block_1_2_Mul_output_0_output_array, NULL)

/* Tensor #242 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_8_block_block_2_Mul_output_0_output, AI_STATIC,
  242, 0x0,
  AI_SHAPE_INIT(4, 1, 144, 43, 2), AI_STRIDE_INIT(4, 4, 4, 576, 24768),
  1, &_features_features_8_block_block_2_Mul_output_0_output_array, NULL)

/* Tensor #243 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_8_block_block_2_activation_Relu_output_0_output, AI_STATIC,
  243, 0x0,
  AI_SHAPE_INIT(4, 1, 40, 1, 1), AI_STRIDE_INIT(4, 4, 4, 160, 160),
  1, &_features_features_8_block_block_2_activation_Relu_output_0_output_array, NULL)

/* Tensor #244 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_8_block_block_2_avgpool_GlobalAveragePool_output_0_output, AI_STATIC,
  244, 0x0,
  AI_SHAPE_INIT(4, 1, 144, 1, 1), AI_STRIDE_INIT(4, 4, 4, 576, 576),
  1, &_features_features_8_block_block_2_avgpool_GlobalAveragePool_output_0_output_array, NULL)

/* Tensor #245 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_8_block_block_2_fc1_Conv_output_0_bias, AI_STATIC,
  245, 0x0,
  AI_SHAPE_INIT(4, 1, 40, 1, 1), AI_STRIDE_INIT(4, 4, 4, 160, 160),
  1, &_features_features_8_block_block_2_fc1_Conv_output_0_bias_array, NULL)

/* Tensor #246 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_8_block_block_2_fc1_Conv_output_0_output, AI_STATIC,
  246, 0x0,
  AI_SHAPE_INIT(4, 1, 40, 1, 1), AI_STRIDE_INIT(4, 4, 4, 160, 160),
  1, &_features_features_8_block_block_2_fc1_Conv_output_0_output_array, NULL)

/* Tensor #247 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_8_block_block_2_fc1_Conv_output_0_scratch0, AI_STATIC,
  247, 0x0,
  AI_SHAPE_INIT(4, 1, 144, 1, 1), AI_STRIDE_INIT(4, 4, 4, 576, 576),
  1, &_features_features_8_block_block_2_fc1_Conv_output_0_scratch0_array, NULL)

/* Tensor #248 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_8_block_block_2_fc1_Conv_output_0_weights, AI_STATIC,
  248, 0x0,
  AI_SHAPE_INIT(4, 144, 1, 1, 40), AI_STRIDE_INIT(4, 4, 576, 23040, 23040),
  1, &_features_features_8_block_block_2_fc1_Conv_output_0_weights_array, NULL)

/* Tensor #249 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_8_block_block_2_fc2_Conv_output_0_bias, AI_STATIC,
  249, 0x0,
  AI_SHAPE_INIT(4, 1, 144, 1, 1), AI_STRIDE_INIT(4, 4, 4, 576, 576),
  1, &_features_features_8_block_block_2_fc2_Conv_output_0_bias_array, NULL)

/* Tensor #250 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_8_block_block_2_fc2_Conv_output_0_output, AI_STATIC,
  250, 0x0,
  AI_SHAPE_INIT(4, 1, 144, 1, 1), AI_STRIDE_INIT(4, 4, 4, 576, 576),
  1, &_features_features_8_block_block_2_fc2_Conv_output_0_output_array, NULL)

/* Tensor #251 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_8_block_block_2_fc2_Conv_output_0_scratch0, AI_STATIC,
  251, 0x0,
  AI_SHAPE_INIT(4, 1, 40, 1, 1), AI_STRIDE_INIT(4, 4, 4, 160, 160),
  1, &_features_features_8_block_block_2_fc2_Conv_output_0_scratch0_array, NULL)

/* Tensor #252 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_8_block_block_2_fc2_Conv_output_0_weights, AI_STATIC,
  252, 0x0,
  AI_SHAPE_INIT(4, 40, 1, 1, 144), AI_STRIDE_INIT(4, 4, 160, 23040, 23040),
  1, &_features_features_8_block_block_2_fc2_Conv_output_0_weights_array, NULL)

/* Tensor #253 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_8_block_block_2_scale_activation_HardSigmoid_output_0_output, AI_STATIC,
  253, 0x0,
  AI_SHAPE_INIT(4, 1, 144, 1, 1), AI_STRIDE_INIT(4, 4, 4, 576, 576),
  1, &_features_features_8_block_block_2_scale_activation_HardSigmoid_output_0_output_array, NULL)

/* Tensor #254 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_8_block_block_3_block_3_0_Conv_output_0_bias, AI_STATIC,
  254, 0x0,
  AI_SHAPE_INIT(4, 1, 48, 1, 1), AI_STRIDE_INIT(4, 4, 4, 192, 192),
  1, &_features_features_8_block_block_3_block_3_0_Conv_output_0_bias_array, NULL)

/* Tensor #255 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_8_block_block_3_block_3_0_Conv_output_0_output, AI_STATIC,
  255, 0x0,
  AI_SHAPE_INIT(4, 1, 48, 43, 2), AI_STRIDE_INIT(4, 4, 4, 192, 8256),
  1, &_features_features_8_block_block_3_block_3_0_Conv_output_0_output_array, NULL)

/* Tensor #256 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_8_block_block_3_block_3_0_Conv_output_0_scratch0, AI_STATIC,
  256, 0x0,
  AI_SHAPE_INIT(4, 1, 144, 1, 1), AI_STRIDE_INIT(4, 4, 4, 576, 576),
  1, &_features_features_8_block_block_3_block_3_0_Conv_output_0_scratch0_array, NULL)

/* Tensor #257 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_8_block_block_3_block_3_0_Conv_output_0_weights, AI_STATIC,
  257, 0x0,
  AI_SHAPE_INIT(4, 144, 1, 1, 48), AI_STRIDE_INIT(4, 4, 576, 27648, 27648),
  1, &_features_features_8_block_block_3_block_3_0_Conv_output_0_weights_array, NULL)

/* Tensor #258 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_9_block_block_0_block_0_0_Conv_output_0_bias, AI_STATIC,
  258, 0x0,
  AI_SHAPE_INIT(4, 1, 288, 1, 1), AI_STRIDE_INIT(4, 4, 4, 1152, 1152),
  1, &_features_features_9_block_block_0_block_0_0_Conv_output_0_bias_array, NULL)

/* Tensor #259 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_9_block_block_0_block_0_0_Conv_output_0_output, AI_STATIC,
  259, 0x0,
  AI_SHAPE_INIT(4, 1, 288, 43, 2), AI_STRIDE_INIT(4, 4, 4, 1152, 49536),
  1, &_features_features_9_block_block_0_block_0_0_Conv_output_0_output_array, NULL)

/* Tensor #260 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_9_block_block_0_block_0_0_Conv_output_0_scratch0, AI_STATIC,
  260, 0x0,
  AI_SHAPE_INIT(4, 1, 48, 1, 1), AI_STRIDE_INIT(4, 4, 4, 192, 192),
  1, &_features_features_9_block_block_0_block_0_0_Conv_output_0_scratch0_array, NULL)

/* Tensor #261 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_9_block_block_0_block_0_0_Conv_output_0_weights, AI_STATIC,
  261, 0x0,
  AI_SHAPE_INIT(4, 48, 1, 1, 288), AI_STRIDE_INIT(4, 4, 192, 55296, 55296),
  1, &_features_features_9_block_block_0_block_0_0_Conv_output_0_weights_array, NULL)

/* Tensor #262 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_9_block_block_0_block_0_2_HardSigmoid_output_0_output, AI_STATIC,
  262, 0x0,
  AI_SHAPE_INIT(4, 1, 288, 43, 2), AI_STRIDE_INIT(4, 4, 4, 1152, 49536),
  1, &_features_features_9_block_block_0_block_0_2_HardSigmoid_output_0_output_array, NULL)

/* Tensor #263 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_9_block_block_0_block_0_2_Mul_output_0_output, AI_STATIC,
  263, 0x0,
  AI_SHAPE_INIT(4, 1, 288, 43, 2), AI_STRIDE_INIT(4, 4, 4, 1152, 49536),
  1, &_features_features_9_block_block_0_block_0_2_Mul_output_0_output_array, NULL)

/* Tensor #264 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_9_block_block_1_block_1_0_Conv_output_0_bias, AI_STATIC,
  264, 0x0,
  AI_SHAPE_INIT(4, 1, 288, 1, 1), AI_STRIDE_INIT(4, 4, 4, 1152, 1152),
  1, &_features_features_9_block_block_1_block_1_0_Conv_output_0_bias_array, NULL)

/* Tensor #265 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_9_block_block_1_block_1_0_Conv_output_0_output, AI_STATIC,
  265, 0x0,
  AI_SHAPE_INIT(4, 1, 288, 22, 1), AI_STRIDE_INIT(4, 4, 4, 1152, 25344),
  1, &_features_features_9_block_block_1_block_1_0_Conv_output_0_output_array, NULL)

/* Tensor #266 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_9_block_block_1_block_1_0_Conv_output_0_weights, AI_STATIC,
  266, 0x0,
  AI_SHAPE_INIT(4, 1, 5, 5, 288), AI_STRIDE_INIT(4, 1, 288, 288, 288),
  1, &_features_features_9_block_block_1_block_1_0_Conv_output_0_weights_array, NULL)

/* Tensor #267 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_9_block_block_1_block_1_2_HardSigmoid_output_0_output, AI_STATIC,
  267, 0x0,
  AI_SHAPE_INIT(4, 1, 288, 22, 1), AI_STRIDE_INIT(4, 4, 4, 1152, 25344),
  1, &_features_features_9_block_block_1_block_1_2_HardSigmoid_output_0_output_array, NULL)

/* Tensor #268 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_9_block_block_1_block_1_2_Mul_output_0_output, AI_STATIC,
  268, 0x0,
  AI_SHAPE_INIT(4, 1, 288, 22, 1), AI_STRIDE_INIT(4, 4, 4, 1152, 25344),
  1, &_features_features_9_block_block_1_block_1_2_Mul_output_0_output_array, NULL)

/* Tensor #269 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_9_block_block_2_Mul_output_0_output, AI_STATIC,
  269, 0x0,
  AI_SHAPE_INIT(4, 1, 288, 22, 1), AI_STRIDE_INIT(4, 4, 4, 1152, 25344),
  1, &_features_features_9_block_block_2_Mul_output_0_output_array, NULL)

/* Tensor #270 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_9_block_block_2_activation_Relu_output_0_output, AI_STATIC,
  270, 0x0,
  AI_SHAPE_INIT(4, 1, 72, 1, 1), AI_STRIDE_INIT(4, 4, 4, 288, 288),
  1, &_features_features_9_block_block_2_activation_Relu_output_0_output_array, NULL)

/* Tensor #271 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_9_block_block_2_avgpool_GlobalAveragePool_output_0_output, AI_STATIC,
  271, 0x0,
  AI_SHAPE_INIT(4, 1, 288, 1, 1), AI_STRIDE_INIT(4, 4, 4, 1152, 1152),
  1, &_features_features_9_block_block_2_avgpool_GlobalAveragePool_output_0_output_array, NULL)

/* Tensor #272 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_9_block_block_2_fc1_Conv_output_0_bias, AI_STATIC,
  272, 0x0,
  AI_SHAPE_INIT(4, 1, 72, 1, 1), AI_STRIDE_INIT(4, 4, 4, 288, 288),
  1, &_features_features_9_block_block_2_fc1_Conv_output_0_bias_array, NULL)

/* Tensor #273 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_9_block_block_2_fc1_Conv_output_0_output, AI_STATIC,
  273, 0x0,
  AI_SHAPE_INIT(4, 1, 72, 1, 1), AI_STRIDE_INIT(4, 4, 4, 288, 288),
  1, &_features_features_9_block_block_2_fc1_Conv_output_0_output_array, NULL)

/* Tensor #274 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_9_block_block_2_fc1_Conv_output_0_scratch0, AI_STATIC,
  274, 0x0,
  AI_SHAPE_INIT(4, 1, 288, 1, 1), AI_STRIDE_INIT(4, 4, 4, 1152, 1152),
  1, &_features_features_9_block_block_2_fc1_Conv_output_0_scratch0_array, NULL)

/* Tensor #275 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_9_block_block_2_fc1_Conv_output_0_weights, AI_STATIC,
  275, 0x0,
  AI_SHAPE_INIT(4, 288, 1, 1, 72), AI_STRIDE_INIT(4, 4, 1152, 82944, 82944),
  1, &_features_features_9_block_block_2_fc1_Conv_output_0_weights_array, NULL)

/* Tensor #276 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_9_block_block_2_fc2_Conv_output_0_bias, AI_STATIC,
  276, 0x0,
  AI_SHAPE_INIT(4, 1, 288, 1, 1), AI_STRIDE_INIT(4, 4, 4, 1152, 1152),
  1, &_features_features_9_block_block_2_fc2_Conv_output_0_bias_array, NULL)

/* Tensor #277 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_9_block_block_2_fc2_Conv_output_0_output, AI_STATIC,
  277, 0x0,
  AI_SHAPE_INIT(4, 1, 288, 1, 1), AI_STRIDE_INIT(4, 4, 4, 1152, 1152),
  1, &_features_features_9_block_block_2_fc2_Conv_output_0_output_array, NULL)

/* Tensor #278 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_9_block_block_2_fc2_Conv_output_0_scratch0, AI_STATIC,
  278, 0x0,
  AI_SHAPE_INIT(4, 1, 72, 1, 1), AI_STRIDE_INIT(4, 4, 4, 288, 288),
  1, &_features_features_9_block_block_2_fc2_Conv_output_0_scratch0_array, NULL)

/* Tensor #279 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_9_block_block_2_fc2_Conv_output_0_weights, AI_STATIC,
  279, 0x0,
  AI_SHAPE_INIT(4, 72, 1, 1, 288), AI_STRIDE_INIT(4, 4, 288, 82944, 82944),
  1, &_features_features_9_block_block_2_fc2_Conv_output_0_weights_array, NULL)

/* Tensor #280 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_9_block_block_2_scale_activation_HardSigmoid_output_0_output, AI_STATIC,
  280, 0x0,
  AI_SHAPE_INIT(4, 1, 288, 1, 1), AI_STRIDE_INIT(4, 4, 4, 1152, 1152),
  1, &_features_features_9_block_block_2_scale_activation_HardSigmoid_output_0_output_array, NULL)

/* Tensor #281 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_9_block_block_3_block_3_0_Conv_output_0_bias, AI_STATIC,
  281, 0x0,
  AI_SHAPE_INIT(4, 1, 96, 1, 1), AI_STRIDE_INIT(4, 4, 4, 384, 384),
  1, &_features_features_9_block_block_3_block_3_0_Conv_output_0_bias_array, NULL)

/* Tensor #282 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_9_block_block_3_block_3_0_Conv_output_0_output, AI_STATIC,
  282, 0x0,
  AI_SHAPE_INIT(4, 1, 96, 22, 1), AI_STRIDE_INIT(4, 4, 4, 384, 8448),
  1, &_features_features_9_block_block_3_block_3_0_Conv_output_0_output_array, NULL)

/* Tensor #283 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_9_block_block_3_block_3_0_Conv_output_0_scratch0, AI_STATIC,
  283, 0x0,
  AI_SHAPE_INIT(4, 1, 288, 1, 1), AI_STRIDE_INIT(4, 4, 4, 1152, 1152),
  1, &_features_features_9_block_block_3_block_3_0_Conv_output_0_scratch0_array, NULL)

/* Tensor #284 */
AI_TENSOR_OBJ_DECLARE(
  _features_features_9_block_block_3_block_3_0_Conv_output_0_weights, AI_STATIC,
  284, 0x0,
  AI_SHAPE_INIT(4, 288, 1, 1, 96), AI_STRIDE_INIT(4, 4, 1152, 110592, 110592),
  1, &_features_features_9_block_block_3_block_3_0_Conv_output_0_weights_array, NULL)

/* Tensor #285 */
AI_TENSOR_OBJ_DECLARE(
  input_output, AI_STATIC,
  285, 0x0,
  AI_SHAPE_INIT(4, 1, 1, 688, 30), AI_STRIDE_INIT(4, 4, 4, 4, 2752),
  1, &input_output_array, NULL)

/* Tensor #286 */
AI_TENSOR_OBJ_DECLARE(
  output_bias, AI_STATIC,
  286, 0x0,
  AI_SHAPE_INIT(4, 1, 7, 1, 1), AI_STRIDE_INIT(4, 4, 4, 28, 28),
  1, &output_bias_array, NULL)

/* Tensor #287 */
AI_TENSOR_OBJ_DECLARE(
  output_output, AI_STATIC,
  287, 0x0,
  AI_SHAPE_INIT(4, 1, 7, 1, 1), AI_STRIDE_INIT(4, 4, 4, 28, 28),
  1, &output_output_array, NULL)

/* Tensor #288 */
AI_TENSOR_OBJ_DECLARE(
  output_weights, AI_STATIC,
  288, 0x0,
  AI_SHAPE_INIT(4, 256, 7, 1, 1), AI_STRIDE_INIT(4, 4, 1024, 7168, 7168),
  1, &output_weights_array, NULL)



/**  Layer declarations section  **********************************************/


AI_TENSOR_CHAIN_OBJ_DECLARE(
  output_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_classifier_classifier_1_Relu_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &output_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 2, &output_weights, &output_bias),
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  output_layer, 140,
  DENSE_TYPE, 0x0, NULL,
  dense, forward_dense,
  &output_chain,
  NULL, &output_layer, AI_STATIC, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  _classifier_classifier_1_Relu_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_classifier_classifier_0_Gemm_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_classifier_classifier_1_Relu_output_0_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  _classifier_classifier_1_Relu_output_0_layer, 139,
  NL_TYPE, 0x0, NULL,
  nl, forward_relu,
  &_classifier_classifier_1_Relu_output_0_chain,
  NULL, &output_layer, AI_STATIC, 
  .nl_params = NULL, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  _classifier_classifier_0_Gemm_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_avgpool_GlobalAveragePool_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_classifier_classifier_0_Gemm_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 2, &_classifier_classifier_0_Gemm_output_0_weights, &_classifier_classifier_0_Gemm_output_0_bias),
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  _classifier_classifier_0_Gemm_output_0_layer, 138,
  DENSE_TYPE, 0x0, NULL,
  dense, forward_dense,
  &_classifier_classifier_0_Gemm_output_0_chain,
  NULL, &_classifier_classifier_1_Relu_output_0_layer, AI_STATIC, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  _avgpool_GlobalAveragePool_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_12_features_12_2_Mul_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_avgpool_GlobalAveragePool_output_0_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  _avgpool_GlobalAveragePool_output_0_layer, 136,
  POOL_TYPE, 0x0, NULL,
  pool, forward_ap,
  &_avgpool_GlobalAveragePool_output_0_chain,
  NULL, &_classifier_classifier_0_Gemm_output_0_layer, AI_STATIC, 
  .pool_size = AI_SHAPE_2D_INIT(22, 1), 
  .pool_stride = AI_SHAPE_2D_INIT(22, 1), 
  .pool_pad = AI_SHAPE_INIT(4, 0, 0, 0, 0), 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  _features_features_12_features_12_2_Mul_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 2, &_features_features_12_features_12_0_Conv_output_0_output, &_features_features_12_features_12_2_HardSigmoid_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_12_features_12_2_Mul_output_0_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  _features_features_12_features_12_2_Mul_output_0_layer, 135,
  ELTWISE_TYPE, 0x0, NULL,
  eltwise, forward_eltwise,
  &_features_features_12_features_12_2_Mul_output_0_chain,
  NULL, &_avgpool_GlobalAveragePool_output_0_layer, AI_STATIC, 
  .operation = ai_mul_f32, 
  .buffer_operation = ai_mul_buffer_f32, 
)


AI_STATIC_CONST ai_float _features_features_12_features_12_2_HardSigmoid_output_0_nl_params_data[] = { 0.1666666716337204, 0.5 };
AI_ARRAY_OBJ_DECLARE(
    _features_features_12_features_12_2_HardSigmoid_output_0_nl_params, AI_ARRAY_FORMAT_FLOAT,
    _features_features_12_features_12_2_HardSigmoid_output_0_nl_params_data, _features_features_12_features_12_2_HardSigmoid_output_0_nl_params_data, 2, AI_STATIC_CONST)
AI_TENSOR_CHAIN_OBJ_DECLARE(
  _features_features_12_features_12_2_HardSigmoid_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_12_features_12_0_Conv_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_12_features_12_2_HardSigmoid_output_0_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  _features_features_12_features_12_2_HardSigmoid_output_0_layer, 134,
  NL_TYPE, 0x0, NULL,
  nl, forward_hard_sigmoid,
  &_features_features_12_features_12_2_HardSigmoid_output_0_chain,
  NULL, &_features_features_12_features_12_2_Mul_output_0_layer, AI_STATIC, 
  .nl_params = &_features_features_12_features_12_2_HardSigmoid_output_0_nl_params, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  _features_features_12_features_12_0_Conv_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_11_Add_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_12_features_12_0_Conv_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 3, &_features_features_12_features_12_0_Conv_output_0_weights, &_features_features_12_features_12_0_Conv_output_0_bias, NULL),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 2, &_features_features_12_features_12_0_Conv_output_0_scratch0, NULL)
)

AI_LAYER_OBJ_DECLARE(
  _features_features_12_features_12_0_Conv_output_0_layer, 133,
  CONV2D_TYPE, 0x0, NULL,
  conv2d, forward_conv2d_if32of32wf32,
  &_features_features_12_features_12_0_Conv_output_0_chain,
  NULL, &_features_features_12_features_12_2_HardSigmoid_output_0_layer, AI_STATIC, 
  .groups = 1, 
  .filter_stride = AI_SHAPE_2D_INIT(1, 1), 
  .dilation = AI_SHAPE_2D_INIT(1, 1), 
  .filter_pad = AI_SHAPE_INIT(4, 0, 0, 0, 0), 
  .in_ch_format = AI_LAYER_FORMAT_CHANNEL_LAST_VALID, 
  .out_ch_format = AI_LAYER_FORMAT_CHANNEL_LAST_VALID, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  _features_features_11_Add_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 2, &_features_features_11_block_block_3_block_3_0_Conv_output_0_output, &_features_features_10_Add_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_11_Add_output_0_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  _features_features_11_Add_output_0_layer, 132,
  ELTWISE_TYPE, 0x0, NULL,
  eltwise, forward_eltwise,
  &_features_features_11_Add_output_0_chain,
  NULL, &_features_features_12_features_12_0_Conv_output_0_layer, AI_STATIC, 
  .operation = ai_sum_f32, 
  .buffer_operation = ai_sum_buffer_f32, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  _features_features_11_block_block_3_block_3_0_Conv_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_11_block_block_2_Mul_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_11_block_block_3_block_3_0_Conv_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 3, &_features_features_11_block_block_3_block_3_0_Conv_output_0_weights, &_features_features_11_block_block_3_block_3_0_Conv_output_0_bias, NULL),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 2, &_features_features_11_block_block_3_block_3_0_Conv_output_0_scratch0, NULL)
)

AI_LAYER_OBJ_DECLARE(
  _features_features_11_block_block_3_block_3_0_Conv_output_0_layer, 131,
  CONV2D_TYPE, 0x0, NULL,
  conv2d, forward_conv2d_if32of32wf32,
  &_features_features_11_block_block_3_block_3_0_Conv_output_0_chain,
  NULL, &_features_features_11_Add_output_0_layer, AI_STATIC, 
  .groups = 1, 
  .filter_stride = AI_SHAPE_2D_INIT(1, 1), 
  .dilation = AI_SHAPE_2D_INIT(1, 1), 
  .filter_pad = AI_SHAPE_INIT(4, 0, 0, 0, 0), 
  .in_ch_format = AI_LAYER_FORMAT_CHANNEL_LAST_VALID, 
  .out_ch_format = AI_LAYER_FORMAT_CHANNEL_LAST_VALID, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  _features_features_11_block_block_2_Mul_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 2, &_features_features_11_block_block_2_scale_activation_HardSigmoid_output_0_output, &_features_features_11_block_block_1_block_1_2_Mul_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_11_block_block_2_Mul_output_0_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  _features_features_11_block_block_2_Mul_output_0_layer, 130,
  ELTWISE_TYPE, 0x0, NULL,
  eltwise, forward_eltwise,
  &_features_features_11_block_block_2_Mul_output_0_chain,
  NULL, &_features_features_11_block_block_3_block_3_0_Conv_output_0_layer, AI_STATIC, 
  .operation = ai_mul_f32, 
  .buffer_operation = ai_mul_buffer_f32, 
)


AI_STATIC_CONST ai_float _features_features_11_block_block_2_scale_activation_HardSigmoid_output_0_nl_params_data[] = { 0.1666666716337204, 0.5 };
AI_ARRAY_OBJ_DECLARE(
    _features_features_11_block_block_2_scale_activation_HardSigmoid_output_0_nl_params, AI_ARRAY_FORMAT_FLOAT,
    _features_features_11_block_block_2_scale_activation_HardSigmoid_output_0_nl_params_data, _features_features_11_block_block_2_scale_activation_HardSigmoid_output_0_nl_params_data, 2, AI_STATIC_CONST)
AI_TENSOR_CHAIN_OBJ_DECLARE(
  _features_features_11_block_block_2_scale_activation_HardSigmoid_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_11_block_block_2_fc2_Conv_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_11_block_block_2_scale_activation_HardSigmoid_output_0_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  _features_features_11_block_block_2_scale_activation_HardSigmoid_output_0_layer, 129,
  NL_TYPE, 0x0, NULL,
  nl, forward_hard_sigmoid,
  &_features_features_11_block_block_2_scale_activation_HardSigmoid_output_0_chain,
  NULL, &_features_features_11_block_block_2_Mul_output_0_layer, AI_STATIC, 
  .nl_params = &_features_features_11_block_block_2_scale_activation_HardSigmoid_output_0_nl_params, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  _features_features_11_block_block_2_fc2_Conv_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_11_block_block_2_activation_Relu_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_11_block_block_2_fc2_Conv_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 3, &_features_features_11_block_block_2_fc2_Conv_output_0_weights, &_features_features_11_block_block_2_fc2_Conv_output_0_bias, NULL),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 2, &_features_features_11_block_block_2_fc2_Conv_output_0_scratch0, NULL)
)

AI_LAYER_OBJ_DECLARE(
  _features_features_11_block_block_2_fc2_Conv_output_0_layer, 128,
  CONV2D_TYPE, 0x0, NULL,
  conv2d, forward_conv2d_if32of32wf32,
  &_features_features_11_block_block_2_fc2_Conv_output_0_chain,
  NULL, &_features_features_11_block_block_2_scale_activation_HardSigmoid_output_0_layer, AI_STATIC, 
  .groups = 1, 
  .filter_stride = AI_SHAPE_2D_INIT(1, 1), 
  .dilation = AI_SHAPE_2D_INIT(1, 1), 
  .filter_pad = AI_SHAPE_INIT(4, 0, 0, 0, 0), 
  .in_ch_format = AI_LAYER_FORMAT_CHANNEL_LAST_VALID, 
  .out_ch_format = AI_LAYER_FORMAT_CHANNEL_LAST_VALID, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  _features_features_11_block_block_2_activation_Relu_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_11_block_block_2_fc1_Conv_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_11_block_block_2_activation_Relu_output_0_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  _features_features_11_block_block_2_activation_Relu_output_0_layer, 127,
  NL_TYPE, 0x0, NULL,
  nl, forward_relu,
  &_features_features_11_block_block_2_activation_Relu_output_0_chain,
  NULL, &_features_features_11_block_block_2_fc2_Conv_output_0_layer, AI_STATIC, 
  .nl_params = NULL, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  _features_features_11_block_block_2_fc1_Conv_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_11_block_block_2_avgpool_GlobalAveragePool_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_11_block_block_2_fc1_Conv_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 3, &_features_features_11_block_block_2_fc1_Conv_output_0_weights, &_features_features_11_block_block_2_fc1_Conv_output_0_bias, NULL),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 2, &_features_features_11_block_block_2_fc1_Conv_output_0_scratch0, NULL)
)

AI_LAYER_OBJ_DECLARE(
  _features_features_11_block_block_2_fc1_Conv_output_0_layer, 126,
  CONV2D_TYPE, 0x0, NULL,
  conv2d, forward_conv2d_if32of32wf32,
  &_features_features_11_block_block_2_fc1_Conv_output_0_chain,
  NULL, &_features_features_11_block_block_2_activation_Relu_output_0_layer, AI_STATIC, 
  .groups = 1, 
  .filter_stride = AI_SHAPE_2D_INIT(1, 1), 
  .dilation = AI_SHAPE_2D_INIT(1, 1), 
  .filter_pad = AI_SHAPE_INIT(4, 0, 0, 0, 0), 
  .in_ch_format = AI_LAYER_FORMAT_CHANNEL_LAST_VALID, 
  .out_ch_format = AI_LAYER_FORMAT_CHANNEL_LAST_VALID, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  _features_features_11_block_block_2_avgpool_GlobalAveragePool_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_11_block_block_1_block_1_2_Mul_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_11_block_block_2_avgpool_GlobalAveragePool_output_0_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  _features_features_11_block_block_2_avgpool_GlobalAveragePool_output_0_layer, 125,
  POOL_TYPE, 0x0, NULL,
  pool, forward_ap,
  &_features_features_11_block_block_2_avgpool_GlobalAveragePool_output_0_chain,
  NULL, &_features_features_11_block_block_2_fc1_Conv_output_0_layer, AI_STATIC, 
  .pool_size = AI_SHAPE_2D_INIT(22, 1), 
  .pool_stride = AI_SHAPE_2D_INIT(22, 1), 
  .pool_pad = AI_SHAPE_INIT(4, 0, 0, 0, 0), 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  _features_features_11_block_block_1_block_1_2_Mul_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 2, &_features_features_11_block_block_1_block_1_0_Conv_output_0_output, &_features_features_11_block_block_1_block_1_2_HardSigmoid_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_11_block_block_1_block_1_2_Mul_output_0_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  _features_features_11_block_block_1_block_1_2_Mul_output_0_layer, 124,
  ELTWISE_TYPE, 0x0, NULL,
  eltwise, forward_eltwise,
  &_features_features_11_block_block_1_block_1_2_Mul_output_0_chain,
  NULL, &_features_features_11_block_block_2_avgpool_GlobalAveragePool_output_0_layer, AI_STATIC, 
  .operation = ai_mul_f32, 
  .buffer_operation = ai_mul_buffer_f32, 
)


AI_STATIC_CONST ai_float _features_features_11_block_block_1_block_1_2_HardSigmoid_output_0_nl_params_data[] = { 0.1666666716337204, 0.5 };
AI_ARRAY_OBJ_DECLARE(
    _features_features_11_block_block_1_block_1_2_HardSigmoid_output_0_nl_params, AI_ARRAY_FORMAT_FLOAT,
    _features_features_11_block_block_1_block_1_2_HardSigmoid_output_0_nl_params_data, _features_features_11_block_block_1_block_1_2_HardSigmoid_output_0_nl_params_data, 2, AI_STATIC_CONST)
AI_TENSOR_CHAIN_OBJ_DECLARE(
  _features_features_11_block_block_1_block_1_2_HardSigmoid_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_11_block_block_1_block_1_0_Conv_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_11_block_block_1_block_1_2_HardSigmoid_output_0_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  _features_features_11_block_block_1_block_1_2_HardSigmoid_output_0_layer, 123,
  NL_TYPE, 0x0, NULL,
  nl, forward_hard_sigmoid,
  &_features_features_11_block_block_1_block_1_2_HardSigmoid_output_0_chain,
  NULL, &_features_features_11_block_block_1_block_1_2_Mul_output_0_layer, AI_STATIC, 
  .nl_params = &_features_features_11_block_block_1_block_1_2_HardSigmoid_output_0_nl_params, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  _features_features_11_block_block_1_block_1_0_Conv_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_11_block_block_0_block_0_2_Mul_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_11_block_block_1_block_1_0_Conv_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 3, &_features_features_11_block_block_1_block_1_0_Conv_output_0_weights, &_features_features_11_block_block_1_block_1_0_Conv_output_0_bias, NULL),
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  _features_features_11_block_block_1_block_1_0_Conv_output_0_layer, 122,
  CONV2D_TYPE, 0x0, NULL,
  conv2d, forward_dw_if32of32wf32,
  &_features_features_11_block_block_1_block_1_0_Conv_output_0_chain,
  NULL, &_features_features_11_block_block_1_block_1_2_HardSigmoid_output_0_layer, AI_STATIC, 
  .groups = 576, 
  .filter_stride = AI_SHAPE_2D_INIT(1, 1), 
  .dilation = AI_SHAPE_2D_INIT(1, 1), 
  .filter_pad = AI_SHAPE_INIT(4, 2, 2, 2, 2), 
  .in_ch_format = AI_LAYER_FORMAT_CHANNEL_LAST_SAME, 
  .out_ch_format = AI_LAYER_FORMAT_CHANNEL_LAST_VALID, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  _features_features_11_block_block_0_block_0_2_Mul_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 2, &_features_features_11_block_block_0_block_0_0_Conv_output_0_output, &_features_features_11_block_block_0_block_0_2_HardSigmoid_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_11_block_block_0_block_0_2_Mul_output_0_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  _features_features_11_block_block_0_block_0_2_Mul_output_0_layer, 121,
  ELTWISE_TYPE, 0x0, NULL,
  eltwise, forward_eltwise,
  &_features_features_11_block_block_0_block_0_2_Mul_output_0_chain,
  NULL, &_features_features_11_block_block_1_block_1_0_Conv_output_0_layer, AI_STATIC, 
  .operation = ai_mul_f32, 
  .buffer_operation = ai_mul_buffer_f32, 
)


AI_STATIC_CONST ai_float _features_features_11_block_block_0_block_0_2_HardSigmoid_output_0_nl_params_data[] = { 0.1666666716337204, 0.5 };
AI_ARRAY_OBJ_DECLARE(
    _features_features_11_block_block_0_block_0_2_HardSigmoid_output_0_nl_params, AI_ARRAY_FORMAT_FLOAT,
    _features_features_11_block_block_0_block_0_2_HardSigmoid_output_0_nl_params_data, _features_features_11_block_block_0_block_0_2_HardSigmoid_output_0_nl_params_data, 2, AI_STATIC_CONST)
AI_TENSOR_CHAIN_OBJ_DECLARE(
  _features_features_11_block_block_0_block_0_2_HardSigmoid_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_11_block_block_0_block_0_0_Conv_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_11_block_block_0_block_0_2_HardSigmoid_output_0_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  _features_features_11_block_block_0_block_0_2_HardSigmoid_output_0_layer, 120,
  NL_TYPE, 0x0, NULL,
  nl, forward_hard_sigmoid,
  &_features_features_11_block_block_0_block_0_2_HardSigmoid_output_0_chain,
  NULL, &_features_features_11_block_block_0_block_0_2_Mul_output_0_layer, AI_STATIC, 
  .nl_params = &_features_features_11_block_block_0_block_0_2_HardSigmoid_output_0_nl_params, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  _features_features_11_block_block_0_block_0_0_Conv_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_10_Add_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_11_block_block_0_block_0_0_Conv_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 3, &_features_features_11_block_block_0_block_0_0_Conv_output_0_weights, &_features_features_11_block_block_0_block_0_0_Conv_output_0_bias, NULL),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 2, &_features_features_11_block_block_0_block_0_0_Conv_output_0_scratch0, NULL)
)

AI_LAYER_OBJ_DECLARE(
  _features_features_11_block_block_0_block_0_0_Conv_output_0_layer, 119,
  CONV2D_TYPE, 0x0, NULL,
  conv2d, forward_conv2d_if32of32wf32,
  &_features_features_11_block_block_0_block_0_0_Conv_output_0_chain,
  NULL, &_features_features_11_block_block_0_block_0_2_HardSigmoid_output_0_layer, AI_STATIC, 
  .groups = 1, 
  .filter_stride = AI_SHAPE_2D_INIT(1, 1), 
  .dilation = AI_SHAPE_2D_INIT(1, 1), 
  .filter_pad = AI_SHAPE_INIT(4, 0, 0, 0, 0), 
  .in_ch_format = AI_LAYER_FORMAT_CHANNEL_LAST_VALID, 
  .out_ch_format = AI_LAYER_FORMAT_CHANNEL_LAST_VALID, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  _features_features_10_Add_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 2, &_features_features_10_block_block_3_block_3_0_Conv_output_0_output, &_features_features_9_block_block_3_block_3_0_Conv_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_10_Add_output_0_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  _features_features_10_Add_output_0_layer, 118,
  ELTWISE_TYPE, 0x0, NULL,
  eltwise, forward_eltwise,
  &_features_features_10_Add_output_0_chain,
  NULL, &_features_features_11_block_block_0_block_0_0_Conv_output_0_layer, AI_STATIC, 
  .operation = ai_sum_f32, 
  .buffer_operation = ai_sum_buffer_f32, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  _features_features_10_block_block_3_block_3_0_Conv_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_10_block_block_2_Mul_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_10_block_block_3_block_3_0_Conv_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 3, &_features_features_10_block_block_3_block_3_0_Conv_output_0_weights, &_features_features_10_block_block_3_block_3_0_Conv_output_0_bias, NULL),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 2, &_features_features_10_block_block_3_block_3_0_Conv_output_0_scratch0, NULL)
)

AI_LAYER_OBJ_DECLARE(
  _features_features_10_block_block_3_block_3_0_Conv_output_0_layer, 117,
  CONV2D_TYPE, 0x0, NULL,
  conv2d, forward_conv2d_if32of32wf32,
  &_features_features_10_block_block_3_block_3_0_Conv_output_0_chain,
  NULL, &_features_features_10_Add_output_0_layer, AI_STATIC, 
  .groups = 1, 
  .filter_stride = AI_SHAPE_2D_INIT(1, 1), 
  .dilation = AI_SHAPE_2D_INIT(1, 1), 
  .filter_pad = AI_SHAPE_INIT(4, 0, 0, 0, 0), 
  .in_ch_format = AI_LAYER_FORMAT_CHANNEL_LAST_VALID, 
  .out_ch_format = AI_LAYER_FORMAT_CHANNEL_LAST_VALID, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  _features_features_10_block_block_2_Mul_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 2, &_features_features_10_block_block_2_scale_activation_HardSigmoid_output_0_output, &_features_features_10_block_block_1_block_1_2_Mul_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_10_block_block_2_Mul_output_0_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  _features_features_10_block_block_2_Mul_output_0_layer, 116,
  ELTWISE_TYPE, 0x0, NULL,
  eltwise, forward_eltwise,
  &_features_features_10_block_block_2_Mul_output_0_chain,
  NULL, &_features_features_10_block_block_3_block_3_0_Conv_output_0_layer, AI_STATIC, 
  .operation = ai_mul_f32, 
  .buffer_operation = ai_mul_buffer_f32, 
)


AI_STATIC_CONST ai_float _features_features_10_block_block_2_scale_activation_HardSigmoid_output_0_nl_params_data[] = { 0.1666666716337204, 0.5 };
AI_ARRAY_OBJ_DECLARE(
    _features_features_10_block_block_2_scale_activation_HardSigmoid_output_0_nl_params, AI_ARRAY_FORMAT_FLOAT,
    _features_features_10_block_block_2_scale_activation_HardSigmoid_output_0_nl_params_data, _features_features_10_block_block_2_scale_activation_HardSigmoid_output_0_nl_params_data, 2, AI_STATIC_CONST)
AI_TENSOR_CHAIN_OBJ_DECLARE(
  _features_features_10_block_block_2_scale_activation_HardSigmoid_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_10_block_block_2_fc2_Conv_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_10_block_block_2_scale_activation_HardSigmoid_output_0_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  _features_features_10_block_block_2_scale_activation_HardSigmoid_output_0_layer, 115,
  NL_TYPE, 0x0, NULL,
  nl, forward_hard_sigmoid,
  &_features_features_10_block_block_2_scale_activation_HardSigmoid_output_0_chain,
  NULL, &_features_features_10_block_block_2_Mul_output_0_layer, AI_STATIC, 
  .nl_params = &_features_features_10_block_block_2_scale_activation_HardSigmoid_output_0_nl_params, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  _features_features_10_block_block_2_fc2_Conv_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_10_block_block_2_activation_Relu_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_10_block_block_2_fc2_Conv_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 3, &_features_features_10_block_block_2_fc2_Conv_output_0_weights, &_features_features_10_block_block_2_fc2_Conv_output_0_bias, NULL),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 2, &_features_features_10_block_block_2_fc2_Conv_output_0_scratch0, NULL)
)

AI_LAYER_OBJ_DECLARE(
  _features_features_10_block_block_2_fc2_Conv_output_0_layer, 114,
  CONV2D_TYPE, 0x0, NULL,
  conv2d, forward_conv2d_if32of32wf32,
  &_features_features_10_block_block_2_fc2_Conv_output_0_chain,
  NULL, &_features_features_10_block_block_2_scale_activation_HardSigmoid_output_0_layer, AI_STATIC, 
  .groups = 1, 
  .filter_stride = AI_SHAPE_2D_INIT(1, 1), 
  .dilation = AI_SHAPE_2D_INIT(1, 1), 
  .filter_pad = AI_SHAPE_INIT(4, 0, 0, 0, 0), 
  .in_ch_format = AI_LAYER_FORMAT_CHANNEL_LAST_VALID, 
  .out_ch_format = AI_LAYER_FORMAT_CHANNEL_LAST_VALID, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  _features_features_10_block_block_2_activation_Relu_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_10_block_block_2_fc1_Conv_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_10_block_block_2_activation_Relu_output_0_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  _features_features_10_block_block_2_activation_Relu_output_0_layer, 113,
  NL_TYPE, 0x0, NULL,
  nl, forward_relu,
  &_features_features_10_block_block_2_activation_Relu_output_0_chain,
  NULL, &_features_features_10_block_block_2_fc2_Conv_output_0_layer, AI_STATIC, 
  .nl_params = NULL, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  _features_features_10_block_block_2_fc1_Conv_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_10_block_block_2_avgpool_GlobalAveragePool_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_10_block_block_2_fc1_Conv_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 3, &_features_features_10_block_block_2_fc1_Conv_output_0_weights, &_features_features_10_block_block_2_fc1_Conv_output_0_bias, NULL),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 2, &_features_features_10_block_block_2_fc1_Conv_output_0_scratch0, NULL)
)

AI_LAYER_OBJ_DECLARE(
  _features_features_10_block_block_2_fc1_Conv_output_0_layer, 112,
  CONV2D_TYPE, 0x0, NULL,
  conv2d, forward_conv2d_if32of32wf32,
  &_features_features_10_block_block_2_fc1_Conv_output_0_chain,
  NULL, &_features_features_10_block_block_2_activation_Relu_output_0_layer, AI_STATIC, 
  .groups = 1, 
  .filter_stride = AI_SHAPE_2D_INIT(1, 1), 
  .dilation = AI_SHAPE_2D_INIT(1, 1), 
  .filter_pad = AI_SHAPE_INIT(4, 0, 0, 0, 0), 
  .in_ch_format = AI_LAYER_FORMAT_CHANNEL_LAST_VALID, 
  .out_ch_format = AI_LAYER_FORMAT_CHANNEL_LAST_VALID, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  _features_features_10_block_block_2_avgpool_GlobalAveragePool_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_10_block_block_1_block_1_2_Mul_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_10_block_block_2_avgpool_GlobalAveragePool_output_0_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  _features_features_10_block_block_2_avgpool_GlobalAveragePool_output_0_layer, 111,
  POOL_TYPE, 0x0, NULL,
  pool, forward_ap,
  &_features_features_10_block_block_2_avgpool_GlobalAveragePool_output_0_chain,
  NULL, &_features_features_10_block_block_2_fc1_Conv_output_0_layer, AI_STATIC, 
  .pool_size = AI_SHAPE_2D_INIT(22, 1), 
  .pool_stride = AI_SHAPE_2D_INIT(22, 1), 
  .pool_pad = AI_SHAPE_INIT(4, 0, 0, 0, 0), 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  _features_features_10_block_block_1_block_1_2_Mul_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 2, &_features_features_10_block_block_1_block_1_0_Conv_output_0_output, &_features_features_10_block_block_1_block_1_2_HardSigmoid_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_10_block_block_1_block_1_2_Mul_output_0_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  _features_features_10_block_block_1_block_1_2_Mul_output_0_layer, 110,
  ELTWISE_TYPE, 0x0, NULL,
  eltwise, forward_eltwise,
  &_features_features_10_block_block_1_block_1_2_Mul_output_0_chain,
  NULL, &_features_features_10_block_block_2_avgpool_GlobalAveragePool_output_0_layer, AI_STATIC, 
  .operation = ai_mul_f32, 
  .buffer_operation = ai_mul_buffer_f32, 
)


AI_STATIC_CONST ai_float _features_features_10_block_block_1_block_1_2_HardSigmoid_output_0_nl_params_data[] = { 0.1666666716337204, 0.5 };
AI_ARRAY_OBJ_DECLARE(
    _features_features_10_block_block_1_block_1_2_HardSigmoid_output_0_nl_params, AI_ARRAY_FORMAT_FLOAT,
    _features_features_10_block_block_1_block_1_2_HardSigmoid_output_0_nl_params_data, _features_features_10_block_block_1_block_1_2_HardSigmoid_output_0_nl_params_data, 2, AI_STATIC_CONST)
AI_TENSOR_CHAIN_OBJ_DECLARE(
  _features_features_10_block_block_1_block_1_2_HardSigmoid_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_10_block_block_1_block_1_0_Conv_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_10_block_block_1_block_1_2_HardSigmoid_output_0_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  _features_features_10_block_block_1_block_1_2_HardSigmoid_output_0_layer, 109,
  NL_TYPE, 0x0, NULL,
  nl, forward_hard_sigmoid,
  &_features_features_10_block_block_1_block_1_2_HardSigmoid_output_0_chain,
  NULL, &_features_features_10_block_block_1_block_1_2_Mul_output_0_layer, AI_STATIC, 
  .nl_params = &_features_features_10_block_block_1_block_1_2_HardSigmoid_output_0_nl_params, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  _features_features_10_block_block_1_block_1_0_Conv_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_10_block_block_0_block_0_2_Mul_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_10_block_block_1_block_1_0_Conv_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 3, &_features_features_10_block_block_1_block_1_0_Conv_output_0_weights, &_features_features_10_block_block_1_block_1_0_Conv_output_0_bias, NULL),
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  _features_features_10_block_block_1_block_1_0_Conv_output_0_layer, 108,
  CONV2D_TYPE, 0x0, NULL,
  conv2d, forward_dw_if32of32wf32,
  &_features_features_10_block_block_1_block_1_0_Conv_output_0_chain,
  NULL, &_features_features_10_block_block_1_block_1_2_HardSigmoid_output_0_layer, AI_STATIC, 
  .groups = 576, 
  .filter_stride = AI_SHAPE_2D_INIT(1, 1), 
  .dilation = AI_SHAPE_2D_INIT(1, 1), 
  .filter_pad = AI_SHAPE_INIT(4, 2, 2, 2, 2), 
  .in_ch_format = AI_LAYER_FORMAT_CHANNEL_LAST_SAME, 
  .out_ch_format = AI_LAYER_FORMAT_CHANNEL_LAST_VALID, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  _features_features_10_block_block_0_block_0_2_Mul_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 2, &_features_features_10_block_block_0_block_0_0_Conv_output_0_output, &_features_features_10_block_block_0_block_0_2_HardSigmoid_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_10_block_block_0_block_0_2_Mul_output_0_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  _features_features_10_block_block_0_block_0_2_Mul_output_0_layer, 107,
  ELTWISE_TYPE, 0x0, NULL,
  eltwise, forward_eltwise,
  &_features_features_10_block_block_0_block_0_2_Mul_output_0_chain,
  NULL, &_features_features_10_block_block_1_block_1_0_Conv_output_0_layer, AI_STATIC, 
  .operation = ai_mul_f32, 
  .buffer_operation = ai_mul_buffer_f32, 
)


AI_STATIC_CONST ai_float _features_features_10_block_block_0_block_0_2_HardSigmoid_output_0_nl_params_data[] = { 0.1666666716337204, 0.5 };
AI_ARRAY_OBJ_DECLARE(
    _features_features_10_block_block_0_block_0_2_HardSigmoid_output_0_nl_params, AI_ARRAY_FORMAT_FLOAT,
    _features_features_10_block_block_0_block_0_2_HardSigmoid_output_0_nl_params_data, _features_features_10_block_block_0_block_0_2_HardSigmoid_output_0_nl_params_data, 2, AI_STATIC_CONST)
AI_TENSOR_CHAIN_OBJ_DECLARE(
  _features_features_10_block_block_0_block_0_2_HardSigmoid_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_10_block_block_0_block_0_0_Conv_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_10_block_block_0_block_0_2_HardSigmoid_output_0_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  _features_features_10_block_block_0_block_0_2_HardSigmoid_output_0_layer, 106,
  NL_TYPE, 0x0, NULL,
  nl, forward_hard_sigmoid,
  &_features_features_10_block_block_0_block_0_2_HardSigmoid_output_0_chain,
  NULL, &_features_features_10_block_block_0_block_0_2_Mul_output_0_layer, AI_STATIC, 
  .nl_params = &_features_features_10_block_block_0_block_0_2_HardSigmoid_output_0_nl_params, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  _features_features_10_block_block_0_block_0_0_Conv_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_9_block_block_3_block_3_0_Conv_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_10_block_block_0_block_0_0_Conv_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 3, &_features_features_10_block_block_0_block_0_0_Conv_output_0_weights, &_features_features_10_block_block_0_block_0_0_Conv_output_0_bias, NULL),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 2, &_features_features_10_block_block_0_block_0_0_Conv_output_0_scratch0, NULL)
)

AI_LAYER_OBJ_DECLARE(
  _features_features_10_block_block_0_block_0_0_Conv_output_0_layer, 105,
  CONV2D_TYPE, 0x0, NULL,
  conv2d, forward_conv2d_if32of32wf32,
  &_features_features_10_block_block_0_block_0_0_Conv_output_0_chain,
  NULL, &_features_features_10_block_block_0_block_0_2_HardSigmoid_output_0_layer, AI_STATIC, 
  .groups = 1, 
  .filter_stride = AI_SHAPE_2D_INIT(1, 1), 
  .dilation = AI_SHAPE_2D_INIT(1, 1), 
  .filter_pad = AI_SHAPE_INIT(4, 0, 0, 0, 0), 
  .in_ch_format = AI_LAYER_FORMAT_CHANNEL_LAST_VALID, 
  .out_ch_format = AI_LAYER_FORMAT_CHANNEL_LAST_VALID, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  _features_features_9_block_block_3_block_3_0_Conv_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_9_block_block_2_Mul_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_9_block_block_3_block_3_0_Conv_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 3, &_features_features_9_block_block_3_block_3_0_Conv_output_0_weights, &_features_features_9_block_block_3_block_3_0_Conv_output_0_bias, NULL),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 2, &_features_features_9_block_block_3_block_3_0_Conv_output_0_scratch0, NULL)
)

AI_LAYER_OBJ_DECLARE(
  _features_features_9_block_block_3_block_3_0_Conv_output_0_layer, 104,
  CONV2D_TYPE, 0x0, NULL,
  conv2d, forward_conv2d_if32of32wf32,
  &_features_features_9_block_block_3_block_3_0_Conv_output_0_chain,
  NULL, &_features_features_10_block_block_0_block_0_0_Conv_output_0_layer, AI_STATIC, 
  .groups = 1, 
  .filter_stride = AI_SHAPE_2D_INIT(1, 1), 
  .dilation = AI_SHAPE_2D_INIT(1, 1), 
  .filter_pad = AI_SHAPE_INIT(4, 0, 0, 0, 0), 
  .in_ch_format = AI_LAYER_FORMAT_CHANNEL_LAST_VALID, 
  .out_ch_format = AI_LAYER_FORMAT_CHANNEL_LAST_VALID, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  _features_features_9_block_block_2_Mul_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 2, &_features_features_9_block_block_2_scale_activation_HardSigmoid_output_0_output, &_features_features_9_block_block_1_block_1_2_Mul_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_9_block_block_2_Mul_output_0_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  _features_features_9_block_block_2_Mul_output_0_layer, 103,
  ELTWISE_TYPE, 0x0, NULL,
  eltwise, forward_eltwise,
  &_features_features_9_block_block_2_Mul_output_0_chain,
  NULL, &_features_features_9_block_block_3_block_3_0_Conv_output_0_layer, AI_STATIC, 
  .operation = ai_mul_f32, 
  .buffer_operation = ai_mul_buffer_f32, 
)


AI_STATIC_CONST ai_float _features_features_9_block_block_2_scale_activation_HardSigmoid_output_0_nl_params_data[] = { 0.1666666716337204, 0.5 };
AI_ARRAY_OBJ_DECLARE(
    _features_features_9_block_block_2_scale_activation_HardSigmoid_output_0_nl_params, AI_ARRAY_FORMAT_FLOAT,
    _features_features_9_block_block_2_scale_activation_HardSigmoid_output_0_nl_params_data, _features_features_9_block_block_2_scale_activation_HardSigmoid_output_0_nl_params_data, 2, AI_STATIC_CONST)
AI_TENSOR_CHAIN_OBJ_DECLARE(
  _features_features_9_block_block_2_scale_activation_HardSigmoid_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_9_block_block_2_fc2_Conv_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_9_block_block_2_scale_activation_HardSigmoid_output_0_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  _features_features_9_block_block_2_scale_activation_HardSigmoid_output_0_layer, 102,
  NL_TYPE, 0x0, NULL,
  nl, forward_hard_sigmoid,
  &_features_features_9_block_block_2_scale_activation_HardSigmoid_output_0_chain,
  NULL, &_features_features_9_block_block_2_Mul_output_0_layer, AI_STATIC, 
  .nl_params = &_features_features_9_block_block_2_scale_activation_HardSigmoid_output_0_nl_params, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  _features_features_9_block_block_2_fc2_Conv_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_9_block_block_2_activation_Relu_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_9_block_block_2_fc2_Conv_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 3, &_features_features_9_block_block_2_fc2_Conv_output_0_weights, &_features_features_9_block_block_2_fc2_Conv_output_0_bias, NULL),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 2, &_features_features_9_block_block_2_fc2_Conv_output_0_scratch0, NULL)
)

AI_LAYER_OBJ_DECLARE(
  _features_features_9_block_block_2_fc2_Conv_output_0_layer, 101,
  CONV2D_TYPE, 0x0, NULL,
  conv2d, forward_conv2d_if32of32wf32,
  &_features_features_9_block_block_2_fc2_Conv_output_0_chain,
  NULL, &_features_features_9_block_block_2_scale_activation_HardSigmoid_output_0_layer, AI_STATIC, 
  .groups = 1, 
  .filter_stride = AI_SHAPE_2D_INIT(1, 1), 
  .dilation = AI_SHAPE_2D_INIT(1, 1), 
  .filter_pad = AI_SHAPE_INIT(4, 0, 0, 0, 0), 
  .in_ch_format = AI_LAYER_FORMAT_CHANNEL_LAST_VALID, 
  .out_ch_format = AI_LAYER_FORMAT_CHANNEL_LAST_VALID, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  _features_features_9_block_block_2_activation_Relu_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_9_block_block_2_fc1_Conv_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_9_block_block_2_activation_Relu_output_0_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  _features_features_9_block_block_2_activation_Relu_output_0_layer, 100,
  NL_TYPE, 0x0, NULL,
  nl, forward_relu,
  &_features_features_9_block_block_2_activation_Relu_output_0_chain,
  NULL, &_features_features_9_block_block_2_fc2_Conv_output_0_layer, AI_STATIC, 
  .nl_params = NULL, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  _features_features_9_block_block_2_fc1_Conv_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_9_block_block_2_avgpool_GlobalAveragePool_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_9_block_block_2_fc1_Conv_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 3, &_features_features_9_block_block_2_fc1_Conv_output_0_weights, &_features_features_9_block_block_2_fc1_Conv_output_0_bias, NULL),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 2, &_features_features_9_block_block_2_fc1_Conv_output_0_scratch0, NULL)
)

AI_LAYER_OBJ_DECLARE(
  _features_features_9_block_block_2_fc1_Conv_output_0_layer, 99,
  CONV2D_TYPE, 0x0, NULL,
  conv2d, forward_conv2d_if32of32wf32,
  &_features_features_9_block_block_2_fc1_Conv_output_0_chain,
  NULL, &_features_features_9_block_block_2_activation_Relu_output_0_layer, AI_STATIC, 
  .groups = 1, 
  .filter_stride = AI_SHAPE_2D_INIT(1, 1), 
  .dilation = AI_SHAPE_2D_INIT(1, 1), 
  .filter_pad = AI_SHAPE_INIT(4, 0, 0, 0, 0), 
  .in_ch_format = AI_LAYER_FORMAT_CHANNEL_LAST_VALID, 
  .out_ch_format = AI_LAYER_FORMAT_CHANNEL_LAST_VALID, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  _features_features_9_block_block_2_avgpool_GlobalAveragePool_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_9_block_block_1_block_1_2_Mul_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_9_block_block_2_avgpool_GlobalAveragePool_output_0_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  _features_features_9_block_block_2_avgpool_GlobalAveragePool_output_0_layer, 98,
  POOL_TYPE, 0x0, NULL,
  pool, forward_ap,
  &_features_features_9_block_block_2_avgpool_GlobalAveragePool_output_0_chain,
  NULL, &_features_features_9_block_block_2_fc1_Conv_output_0_layer, AI_STATIC, 
  .pool_size = AI_SHAPE_2D_INIT(22, 1), 
  .pool_stride = AI_SHAPE_2D_INIT(22, 1), 
  .pool_pad = AI_SHAPE_INIT(4, 0, 0, 0, 0), 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  _features_features_9_block_block_1_block_1_2_Mul_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 2, &_features_features_9_block_block_1_block_1_0_Conv_output_0_output, &_features_features_9_block_block_1_block_1_2_HardSigmoid_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_9_block_block_1_block_1_2_Mul_output_0_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  _features_features_9_block_block_1_block_1_2_Mul_output_0_layer, 97,
  ELTWISE_TYPE, 0x0, NULL,
  eltwise, forward_eltwise,
  &_features_features_9_block_block_1_block_1_2_Mul_output_0_chain,
  NULL, &_features_features_9_block_block_2_avgpool_GlobalAveragePool_output_0_layer, AI_STATIC, 
  .operation = ai_mul_f32, 
  .buffer_operation = ai_mul_buffer_f32, 
)


AI_STATIC_CONST ai_float _features_features_9_block_block_1_block_1_2_HardSigmoid_output_0_nl_params_data[] = { 0.1666666716337204, 0.5 };
AI_ARRAY_OBJ_DECLARE(
    _features_features_9_block_block_1_block_1_2_HardSigmoid_output_0_nl_params, AI_ARRAY_FORMAT_FLOAT,
    _features_features_9_block_block_1_block_1_2_HardSigmoid_output_0_nl_params_data, _features_features_9_block_block_1_block_1_2_HardSigmoid_output_0_nl_params_data, 2, AI_STATIC_CONST)
AI_TENSOR_CHAIN_OBJ_DECLARE(
  _features_features_9_block_block_1_block_1_2_HardSigmoid_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_9_block_block_1_block_1_0_Conv_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_9_block_block_1_block_1_2_HardSigmoid_output_0_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  _features_features_9_block_block_1_block_1_2_HardSigmoid_output_0_layer, 96,
  NL_TYPE, 0x0, NULL,
  nl, forward_hard_sigmoid,
  &_features_features_9_block_block_1_block_1_2_HardSigmoid_output_0_chain,
  NULL, &_features_features_9_block_block_1_block_1_2_Mul_output_0_layer, AI_STATIC, 
  .nl_params = &_features_features_9_block_block_1_block_1_2_HardSigmoid_output_0_nl_params, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  _features_features_9_block_block_1_block_1_0_Conv_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_9_block_block_0_block_0_2_Mul_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_9_block_block_1_block_1_0_Conv_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 3, &_features_features_9_block_block_1_block_1_0_Conv_output_0_weights, &_features_features_9_block_block_1_block_1_0_Conv_output_0_bias, NULL),
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  _features_features_9_block_block_1_block_1_0_Conv_output_0_layer, 95,
  CONV2D_TYPE, 0x0, NULL,
  conv2d, forward_dw_if32of32wf32,
  &_features_features_9_block_block_1_block_1_0_Conv_output_0_chain,
  NULL, &_features_features_9_block_block_1_block_1_2_HardSigmoid_output_0_layer, AI_STATIC, 
  .groups = 288, 
  .filter_stride = AI_SHAPE_2D_INIT(2, 2), 
  .dilation = AI_SHAPE_2D_INIT(1, 1), 
  .filter_pad = AI_SHAPE_INIT(4, 2, 2, 2, 2), 
  .in_ch_format = AI_LAYER_FORMAT_CHANNEL_LAST_SAME, 
  .out_ch_format = AI_LAYER_FORMAT_CHANNEL_LAST_VALID, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  _features_features_9_block_block_0_block_0_2_Mul_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 2, &_features_features_9_block_block_0_block_0_0_Conv_output_0_output, &_features_features_9_block_block_0_block_0_2_HardSigmoid_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_9_block_block_0_block_0_2_Mul_output_0_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  _features_features_9_block_block_0_block_0_2_Mul_output_0_layer, 94,
  ELTWISE_TYPE, 0x0, NULL,
  eltwise, forward_eltwise,
  &_features_features_9_block_block_0_block_0_2_Mul_output_0_chain,
  NULL, &_features_features_9_block_block_1_block_1_0_Conv_output_0_layer, AI_STATIC, 
  .operation = ai_mul_f32, 
  .buffer_operation = ai_mul_buffer_f32, 
)


AI_STATIC_CONST ai_float _features_features_9_block_block_0_block_0_2_HardSigmoid_output_0_nl_params_data[] = { 0.1666666716337204, 0.5 };
AI_ARRAY_OBJ_DECLARE(
    _features_features_9_block_block_0_block_0_2_HardSigmoid_output_0_nl_params, AI_ARRAY_FORMAT_FLOAT,
    _features_features_9_block_block_0_block_0_2_HardSigmoid_output_0_nl_params_data, _features_features_9_block_block_0_block_0_2_HardSigmoid_output_0_nl_params_data, 2, AI_STATIC_CONST)
AI_TENSOR_CHAIN_OBJ_DECLARE(
  _features_features_9_block_block_0_block_0_2_HardSigmoid_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_9_block_block_0_block_0_0_Conv_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_9_block_block_0_block_0_2_HardSigmoid_output_0_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  _features_features_9_block_block_0_block_0_2_HardSigmoid_output_0_layer, 93,
  NL_TYPE, 0x0, NULL,
  nl, forward_hard_sigmoid,
  &_features_features_9_block_block_0_block_0_2_HardSigmoid_output_0_chain,
  NULL, &_features_features_9_block_block_0_block_0_2_Mul_output_0_layer, AI_STATIC, 
  .nl_params = &_features_features_9_block_block_0_block_0_2_HardSigmoid_output_0_nl_params, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  _features_features_9_block_block_0_block_0_0_Conv_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_8_Add_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_9_block_block_0_block_0_0_Conv_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 3, &_features_features_9_block_block_0_block_0_0_Conv_output_0_weights, &_features_features_9_block_block_0_block_0_0_Conv_output_0_bias, NULL),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 2, &_features_features_9_block_block_0_block_0_0_Conv_output_0_scratch0, NULL)
)

AI_LAYER_OBJ_DECLARE(
  _features_features_9_block_block_0_block_0_0_Conv_output_0_layer, 92,
  CONV2D_TYPE, 0x0, NULL,
  conv2d, forward_conv2d_if32of32wf32,
  &_features_features_9_block_block_0_block_0_0_Conv_output_0_chain,
  NULL, &_features_features_9_block_block_0_block_0_2_HardSigmoid_output_0_layer, AI_STATIC, 
  .groups = 1, 
  .filter_stride = AI_SHAPE_2D_INIT(1, 1), 
  .dilation = AI_SHAPE_2D_INIT(1, 1), 
  .filter_pad = AI_SHAPE_INIT(4, 0, 0, 0, 0), 
  .in_ch_format = AI_LAYER_FORMAT_CHANNEL_LAST_VALID, 
  .out_ch_format = AI_LAYER_FORMAT_CHANNEL_LAST_VALID, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  _features_features_8_Add_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 2, &_features_features_8_block_block_3_block_3_0_Conv_output_0_output, &_features_features_7_block_block_3_block_3_0_Conv_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_8_Add_output_0_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  _features_features_8_Add_output_0_layer, 91,
  ELTWISE_TYPE, 0x0, NULL,
  eltwise, forward_eltwise,
  &_features_features_8_Add_output_0_chain,
  NULL, &_features_features_9_block_block_0_block_0_0_Conv_output_0_layer, AI_STATIC, 
  .operation = ai_sum_f32, 
  .buffer_operation = ai_sum_buffer_f32, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  _features_features_8_block_block_3_block_3_0_Conv_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_8_block_block_2_Mul_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_8_block_block_3_block_3_0_Conv_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 3, &_features_features_8_block_block_3_block_3_0_Conv_output_0_weights, &_features_features_8_block_block_3_block_3_0_Conv_output_0_bias, NULL),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 2, &_features_features_8_block_block_3_block_3_0_Conv_output_0_scratch0, NULL)
)

AI_LAYER_OBJ_DECLARE(
  _features_features_8_block_block_3_block_3_0_Conv_output_0_layer, 90,
  CONV2D_TYPE, 0x0, NULL,
  conv2d, forward_conv2d_if32of32wf32,
  &_features_features_8_block_block_3_block_3_0_Conv_output_0_chain,
  NULL, &_features_features_8_Add_output_0_layer, AI_STATIC, 
  .groups = 1, 
  .filter_stride = AI_SHAPE_2D_INIT(1, 1), 
  .dilation = AI_SHAPE_2D_INIT(1, 1), 
  .filter_pad = AI_SHAPE_INIT(4, 0, 0, 0, 0), 
  .in_ch_format = AI_LAYER_FORMAT_CHANNEL_LAST_VALID, 
  .out_ch_format = AI_LAYER_FORMAT_CHANNEL_LAST_VALID, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  _features_features_8_block_block_2_Mul_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 2, &_features_features_8_block_block_2_scale_activation_HardSigmoid_output_0_output, &_features_features_8_block_block_1_block_1_2_Mul_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_8_block_block_2_Mul_output_0_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  _features_features_8_block_block_2_Mul_output_0_layer, 89,
  ELTWISE_TYPE, 0x0, NULL,
  eltwise, forward_eltwise,
  &_features_features_8_block_block_2_Mul_output_0_chain,
  NULL, &_features_features_8_block_block_3_block_3_0_Conv_output_0_layer, AI_STATIC, 
  .operation = ai_mul_f32, 
  .buffer_operation = ai_mul_buffer_f32, 
)


AI_STATIC_CONST ai_float _features_features_8_block_block_2_scale_activation_HardSigmoid_output_0_nl_params_data[] = { 0.1666666716337204, 0.5 };
AI_ARRAY_OBJ_DECLARE(
    _features_features_8_block_block_2_scale_activation_HardSigmoid_output_0_nl_params, AI_ARRAY_FORMAT_FLOAT,
    _features_features_8_block_block_2_scale_activation_HardSigmoid_output_0_nl_params_data, _features_features_8_block_block_2_scale_activation_HardSigmoid_output_0_nl_params_data, 2, AI_STATIC_CONST)
AI_TENSOR_CHAIN_OBJ_DECLARE(
  _features_features_8_block_block_2_scale_activation_HardSigmoid_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_8_block_block_2_fc2_Conv_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_8_block_block_2_scale_activation_HardSigmoid_output_0_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  _features_features_8_block_block_2_scale_activation_HardSigmoid_output_0_layer, 88,
  NL_TYPE, 0x0, NULL,
  nl, forward_hard_sigmoid,
  &_features_features_8_block_block_2_scale_activation_HardSigmoid_output_0_chain,
  NULL, &_features_features_8_block_block_2_Mul_output_0_layer, AI_STATIC, 
  .nl_params = &_features_features_8_block_block_2_scale_activation_HardSigmoid_output_0_nl_params, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  _features_features_8_block_block_2_fc2_Conv_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_8_block_block_2_activation_Relu_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_8_block_block_2_fc2_Conv_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 3, &_features_features_8_block_block_2_fc2_Conv_output_0_weights, &_features_features_8_block_block_2_fc2_Conv_output_0_bias, NULL),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 2, &_features_features_8_block_block_2_fc2_Conv_output_0_scratch0, NULL)
)

AI_LAYER_OBJ_DECLARE(
  _features_features_8_block_block_2_fc2_Conv_output_0_layer, 87,
  CONV2D_TYPE, 0x0, NULL,
  conv2d, forward_conv2d_if32of32wf32,
  &_features_features_8_block_block_2_fc2_Conv_output_0_chain,
  NULL, &_features_features_8_block_block_2_scale_activation_HardSigmoid_output_0_layer, AI_STATIC, 
  .groups = 1, 
  .filter_stride = AI_SHAPE_2D_INIT(1, 1), 
  .dilation = AI_SHAPE_2D_INIT(1, 1), 
  .filter_pad = AI_SHAPE_INIT(4, 0, 0, 0, 0), 
  .in_ch_format = AI_LAYER_FORMAT_CHANNEL_LAST_VALID, 
  .out_ch_format = AI_LAYER_FORMAT_CHANNEL_LAST_VALID, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  _features_features_8_block_block_2_activation_Relu_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_8_block_block_2_fc1_Conv_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_8_block_block_2_activation_Relu_output_0_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  _features_features_8_block_block_2_activation_Relu_output_0_layer, 86,
  NL_TYPE, 0x0, NULL,
  nl, forward_relu,
  &_features_features_8_block_block_2_activation_Relu_output_0_chain,
  NULL, &_features_features_8_block_block_2_fc2_Conv_output_0_layer, AI_STATIC, 
  .nl_params = NULL, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  _features_features_8_block_block_2_fc1_Conv_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_8_block_block_2_avgpool_GlobalAveragePool_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_8_block_block_2_fc1_Conv_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 3, &_features_features_8_block_block_2_fc1_Conv_output_0_weights, &_features_features_8_block_block_2_fc1_Conv_output_0_bias, NULL),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 2, &_features_features_8_block_block_2_fc1_Conv_output_0_scratch0, NULL)
)

AI_LAYER_OBJ_DECLARE(
  _features_features_8_block_block_2_fc1_Conv_output_0_layer, 85,
  CONV2D_TYPE, 0x0, NULL,
  conv2d, forward_conv2d_if32of32wf32,
  &_features_features_8_block_block_2_fc1_Conv_output_0_chain,
  NULL, &_features_features_8_block_block_2_activation_Relu_output_0_layer, AI_STATIC, 
  .groups = 1, 
  .filter_stride = AI_SHAPE_2D_INIT(1, 1), 
  .dilation = AI_SHAPE_2D_INIT(1, 1), 
  .filter_pad = AI_SHAPE_INIT(4, 0, 0, 0, 0), 
  .in_ch_format = AI_LAYER_FORMAT_CHANNEL_LAST_VALID, 
  .out_ch_format = AI_LAYER_FORMAT_CHANNEL_LAST_VALID, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  _features_features_8_block_block_2_avgpool_GlobalAveragePool_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_8_block_block_1_block_1_2_Mul_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_8_block_block_2_avgpool_GlobalAveragePool_output_0_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  _features_features_8_block_block_2_avgpool_GlobalAveragePool_output_0_layer, 84,
  POOL_TYPE, 0x0, NULL,
  pool, forward_ap,
  &_features_features_8_block_block_2_avgpool_GlobalAveragePool_output_0_chain,
  NULL, &_features_features_8_block_block_2_fc1_Conv_output_0_layer, AI_STATIC, 
  .pool_size = AI_SHAPE_2D_INIT(43, 2), 
  .pool_stride = AI_SHAPE_2D_INIT(43, 2), 
  .pool_pad = AI_SHAPE_INIT(4, 0, 0, 0, 0), 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  _features_features_8_block_block_1_block_1_2_Mul_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 2, &_features_features_8_block_block_1_block_1_0_Conv_output_0_output, &_features_features_8_block_block_1_block_1_2_HardSigmoid_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_8_block_block_1_block_1_2_Mul_output_0_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  _features_features_8_block_block_1_block_1_2_Mul_output_0_layer, 83,
  ELTWISE_TYPE, 0x0, NULL,
  eltwise, forward_eltwise,
  &_features_features_8_block_block_1_block_1_2_Mul_output_0_chain,
  NULL, &_features_features_8_block_block_2_avgpool_GlobalAveragePool_output_0_layer, AI_STATIC, 
  .operation = ai_mul_f32, 
  .buffer_operation = ai_mul_buffer_f32, 
)


AI_STATIC_CONST ai_float _features_features_8_block_block_1_block_1_2_HardSigmoid_output_0_nl_params_data[] = { 0.1666666716337204, 0.5 };
AI_ARRAY_OBJ_DECLARE(
    _features_features_8_block_block_1_block_1_2_HardSigmoid_output_0_nl_params, AI_ARRAY_FORMAT_FLOAT,
    _features_features_8_block_block_1_block_1_2_HardSigmoid_output_0_nl_params_data, _features_features_8_block_block_1_block_1_2_HardSigmoid_output_0_nl_params_data, 2, AI_STATIC_CONST)
AI_TENSOR_CHAIN_OBJ_DECLARE(
  _features_features_8_block_block_1_block_1_2_HardSigmoid_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_8_block_block_1_block_1_0_Conv_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_8_block_block_1_block_1_2_HardSigmoid_output_0_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  _features_features_8_block_block_1_block_1_2_HardSigmoid_output_0_layer, 82,
  NL_TYPE, 0x0, NULL,
  nl, forward_hard_sigmoid,
  &_features_features_8_block_block_1_block_1_2_HardSigmoid_output_0_chain,
  NULL, &_features_features_8_block_block_1_block_1_2_Mul_output_0_layer, AI_STATIC, 
  .nl_params = &_features_features_8_block_block_1_block_1_2_HardSigmoid_output_0_nl_params, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  _features_features_8_block_block_1_block_1_0_Conv_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_8_block_block_0_block_0_2_Mul_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_8_block_block_1_block_1_0_Conv_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 3, &_features_features_8_block_block_1_block_1_0_Conv_output_0_weights, &_features_features_8_block_block_1_block_1_0_Conv_output_0_bias, NULL),
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  _features_features_8_block_block_1_block_1_0_Conv_output_0_layer, 81,
  CONV2D_TYPE, 0x0, NULL,
  conv2d, forward_dw_if32of32wf32,
  &_features_features_8_block_block_1_block_1_0_Conv_output_0_chain,
  NULL, &_features_features_8_block_block_1_block_1_2_HardSigmoid_output_0_layer, AI_STATIC, 
  .groups = 144, 
  .filter_stride = AI_SHAPE_2D_INIT(1, 1), 
  .dilation = AI_SHAPE_2D_INIT(1, 1), 
  .filter_pad = AI_SHAPE_INIT(4, 2, 2, 2, 2), 
  .in_ch_format = AI_LAYER_FORMAT_CHANNEL_LAST_SAME, 
  .out_ch_format = AI_LAYER_FORMAT_CHANNEL_LAST_VALID, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  _features_features_8_block_block_0_block_0_2_Mul_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 2, &_features_features_8_block_block_0_block_0_0_Conv_output_0_output, &_features_features_8_block_block_0_block_0_2_HardSigmoid_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_8_block_block_0_block_0_2_Mul_output_0_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  _features_features_8_block_block_0_block_0_2_Mul_output_0_layer, 80,
  ELTWISE_TYPE, 0x0, NULL,
  eltwise, forward_eltwise,
  &_features_features_8_block_block_0_block_0_2_Mul_output_0_chain,
  NULL, &_features_features_8_block_block_1_block_1_0_Conv_output_0_layer, AI_STATIC, 
  .operation = ai_mul_f32, 
  .buffer_operation = ai_mul_buffer_f32, 
)


AI_STATIC_CONST ai_float _features_features_8_block_block_0_block_0_2_HardSigmoid_output_0_nl_params_data[] = { 0.1666666716337204, 0.5 };
AI_ARRAY_OBJ_DECLARE(
    _features_features_8_block_block_0_block_0_2_HardSigmoid_output_0_nl_params, AI_ARRAY_FORMAT_FLOAT,
    _features_features_8_block_block_0_block_0_2_HardSigmoid_output_0_nl_params_data, _features_features_8_block_block_0_block_0_2_HardSigmoid_output_0_nl_params_data, 2, AI_STATIC_CONST)
AI_TENSOR_CHAIN_OBJ_DECLARE(
  _features_features_8_block_block_0_block_0_2_HardSigmoid_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_8_block_block_0_block_0_0_Conv_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_8_block_block_0_block_0_2_HardSigmoid_output_0_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  _features_features_8_block_block_0_block_0_2_HardSigmoid_output_0_layer, 79,
  NL_TYPE, 0x0, NULL,
  nl, forward_hard_sigmoid,
  &_features_features_8_block_block_0_block_0_2_HardSigmoid_output_0_chain,
  NULL, &_features_features_8_block_block_0_block_0_2_Mul_output_0_layer, AI_STATIC, 
  .nl_params = &_features_features_8_block_block_0_block_0_2_HardSigmoid_output_0_nl_params, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  _features_features_8_block_block_0_block_0_0_Conv_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_7_block_block_3_block_3_0_Conv_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_8_block_block_0_block_0_0_Conv_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 3, &_features_features_8_block_block_0_block_0_0_Conv_output_0_weights, &_features_features_8_block_block_0_block_0_0_Conv_output_0_bias, NULL),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 2, &_features_features_8_block_block_0_block_0_0_Conv_output_0_scratch0, NULL)
)

AI_LAYER_OBJ_DECLARE(
  _features_features_8_block_block_0_block_0_0_Conv_output_0_layer, 78,
  CONV2D_TYPE, 0x0, NULL,
  conv2d, forward_conv2d_if32of32wf32,
  &_features_features_8_block_block_0_block_0_0_Conv_output_0_chain,
  NULL, &_features_features_8_block_block_0_block_0_2_HardSigmoid_output_0_layer, AI_STATIC, 
  .groups = 1, 
  .filter_stride = AI_SHAPE_2D_INIT(1, 1), 
  .dilation = AI_SHAPE_2D_INIT(1, 1), 
  .filter_pad = AI_SHAPE_INIT(4, 0, 0, 0, 0), 
  .in_ch_format = AI_LAYER_FORMAT_CHANNEL_LAST_VALID, 
  .out_ch_format = AI_LAYER_FORMAT_CHANNEL_LAST_VALID, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  _features_features_7_block_block_3_block_3_0_Conv_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_7_block_block_2_Mul_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_7_block_block_3_block_3_0_Conv_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 3, &_features_features_7_block_block_3_block_3_0_Conv_output_0_weights, &_features_features_7_block_block_3_block_3_0_Conv_output_0_bias, NULL),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 2, &_features_features_7_block_block_3_block_3_0_Conv_output_0_scratch0, NULL)
)

AI_LAYER_OBJ_DECLARE(
  _features_features_7_block_block_3_block_3_0_Conv_output_0_layer, 77,
  CONV2D_TYPE, 0x0, NULL,
  conv2d, forward_conv2d_if32of32wf32,
  &_features_features_7_block_block_3_block_3_0_Conv_output_0_chain,
  NULL, &_features_features_8_block_block_0_block_0_0_Conv_output_0_layer, AI_STATIC, 
  .groups = 1, 
  .filter_stride = AI_SHAPE_2D_INIT(1, 1), 
  .dilation = AI_SHAPE_2D_INIT(1, 1), 
  .filter_pad = AI_SHAPE_INIT(4, 0, 0, 0, 0), 
  .in_ch_format = AI_LAYER_FORMAT_CHANNEL_LAST_VALID, 
  .out_ch_format = AI_LAYER_FORMAT_CHANNEL_LAST_VALID, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  _features_features_7_block_block_2_Mul_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 2, &_features_features_7_block_block_2_scale_activation_HardSigmoid_output_0_output, &_features_features_7_block_block_1_block_1_2_Mul_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_7_block_block_2_Mul_output_0_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  _features_features_7_block_block_2_Mul_output_0_layer, 76,
  ELTWISE_TYPE, 0x0, NULL,
  eltwise, forward_eltwise,
  &_features_features_7_block_block_2_Mul_output_0_chain,
  NULL, &_features_features_7_block_block_3_block_3_0_Conv_output_0_layer, AI_STATIC, 
  .operation = ai_mul_f32, 
  .buffer_operation = ai_mul_buffer_f32, 
)


AI_STATIC_CONST ai_float _features_features_7_block_block_2_scale_activation_HardSigmoid_output_0_nl_params_data[] = { 0.1666666716337204, 0.5 };
AI_ARRAY_OBJ_DECLARE(
    _features_features_7_block_block_2_scale_activation_HardSigmoid_output_0_nl_params, AI_ARRAY_FORMAT_FLOAT,
    _features_features_7_block_block_2_scale_activation_HardSigmoid_output_0_nl_params_data, _features_features_7_block_block_2_scale_activation_HardSigmoid_output_0_nl_params_data, 2, AI_STATIC_CONST)
AI_TENSOR_CHAIN_OBJ_DECLARE(
  _features_features_7_block_block_2_scale_activation_HardSigmoid_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_7_block_block_2_fc2_Conv_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_7_block_block_2_scale_activation_HardSigmoid_output_0_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  _features_features_7_block_block_2_scale_activation_HardSigmoid_output_0_layer, 75,
  NL_TYPE, 0x0, NULL,
  nl, forward_hard_sigmoid,
  &_features_features_7_block_block_2_scale_activation_HardSigmoid_output_0_chain,
  NULL, &_features_features_7_block_block_2_Mul_output_0_layer, AI_STATIC, 
  .nl_params = &_features_features_7_block_block_2_scale_activation_HardSigmoid_output_0_nl_params, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  _features_features_7_block_block_2_fc2_Conv_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_7_block_block_2_activation_Relu_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_7_block_block_2_fc2_Conv_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 3, &_features_features_7_block_block_2_fc2_Conv_output_0_weights, &_features_features_7_block_block_2_fc2_Conv_output_0_bias, NULL),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 2, &_features_features_7_block_block_2_fc2_Conv_output_0_scratch0, NULL)
)

AI_LAYER_OBJ_DECLARE(
  _features_features_7_block_block_2_fc2_Conv_output_0_layer, 74,
  CONV2D_TYPE, 0x0, NULL,
  conv2d, forward_conv2d_if32of32wf32,
  &_features_features_7_block_block_2_fc2_Conv_output_0_chain,
  NULL, &_features_features_7_block_block_2_scale_activation_HardSigmoid_output_0_layer, AI_STATIC, 
  .groups = 1, 
  .filter_stride = AI_SHAPE_2D_INIT(1, 1), 
  .dilation = AI_SHAPE_2D_INIT(1, 1), 
  .filter_pad = AI_SHAPE_INIT(4, 0, 0, 0, 0), 
  .in_ch_format = AI_LAYER_FORMAT_CHANNEL_LAST_VALID, 
  .out_ch_format = AI_LAYER_FORMAT_CHANNEL_LAST_VALID, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  _features_features_7_block_block_2_activation_Relu_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_7_block_block_2_fc1_Conv_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_7_block_block_2_activation_Relu_output_0_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  _features_features_7_block_block_2_activation_Relu_output_0_layer, 73,
  NL_TYPE, 0x0, NULL,
  nl, forward_relu,
  &_features_features_7_block_block_2_activation_Relu_output_0_chain,
  NULL, &_features_features_7_block_block_2_fc2_Conv_output_0_layer, AI_STATIC, 
  .nl_params = NULL, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  _features_features_7_block_block_2_fc1_Conv_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_7_block_block_2_avgpool_GlobalAveragePool_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_7_block_block_2_fc1_Conv_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 3, &_features_features_7_block_block_2_fc1_Conv_output_0_weights, &_features_features_7_block_block_2_fc1_Conv_output_0_bias, NULL),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 2, &_features_features_7_block_block_2_fc1_Conv_output_0_scratch0, NULL)
)

AI_LAYER_OBJ_DECLARE(
  _features_features_7_block_block_2_fc1_Conv_output_0_layer, 72,
  CONV2D_TYPE, 0x0, NULL,
  conv2d, forward_conv2d_if32of32wf32,
  &_features_features_7_block_block_2_fc1_Conv_output_0_chain,
  NULL, &_features_features_7_block_block_2_activation_Relu_output_0_layer, AI_STATIC, 
  .groups = 1, 
  .filter_stride = AI_SHAPE_2D_INIT(1, 1), 
  .dilation = AI_SHAPE_2D_INIT(1, 1), 
  .filter_pad = AI_SHAPE_INIT(4, 0, 0, 0, 0), 
  .in_ch_format = AI_LAYER_FORMAT_CHANNEL_LAST_VALID, 
  .out_ch_format = AI_LAYER_FORMAT_CHANNEL_LAST_VALID, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  _features_features_7_block_block_2_avgpool_GlobalAveragePool_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_7_block_block_1_block_1_2_Mul_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_7_block_block_2_avgpool_GlobalAveragePool_output_0_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  _features_features_7_block_block_2_avgpool_GlobalAveragePool_output_0_layer, 71,
  POOL_TYPE, 0x0, NULL,
  pool, forward_ap,
  &_features_features_7_block_block_2_avgpool_GlobalAveragePool_output_0_chain,
  NULL, &_features_features_7_block_block_2_fc1_Conv_output_0_layer, AI_STATIC, 
  .pool_size = AI_SHAPE_2D_INIT(43, 2), 
  .pool_stride = AI_SHAPE_2D_INIT(43, 2), 
  .pool_pad = AI_SHAPE_INIT(4, 0, 0, 0, 0), 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  _features_features_7_block_block_1_block_1_2_Mul_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 2, &_features_features_7_block_block_1_block_1_0_Conv_output_0_output, &_features_features_7_block_block_1_block_1_2_HardSigmoid_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_7_block_block_1_block_1_2_Mul_output_0_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  _features_features_7_block_block_1_block_1_2_Mul_output_0_layer, 70,
  ELTWISE_TYPE, 0x0, NULL,
  eltwise, forward_eltwise,
  &_features_features_7_block_block_1_block_1_2_Mul_output_0_chain,
  NULL, &_features_features_7_block_block_2_avgpool_GlobalAveragePool_output_0_layer, AI_STATIC, 
  .operation = ai_mul_f32, 
  .buffer_operation = ai_mul_buffer_f32, 
)


AI_STATIC_CONST ai_float _features_features_7_block_block_1_block_1_2_HardSigmoid_output_0_nl_params_data[] = { 0.1666666716337204, 0.5 };
AI_ARRAY_OBJ_DECLARE(
    _features_features_7_block_block_1_block_1_2_HardSigmoid_output_0_nl_params, AI_ARRAY_FORMAT_FLOAT,
    _features_features_7_block_block_1_block_1_2_HardSigmoid_output_0_nl_params_data, _features_features_7_block_block_1_block_1_2_HardSigmoid_output_0_nl_params_data, 2, AI_STATIC_CONST)
AI_TENSOR_CHAIN_OBJ_DECLARE(
  _features_features_7_block_block_1_block_1_2_HardSigmoid_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_7_block_block_1_block_1_0_Conv_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_7_block_block_1_block_1_2_HardSigmoid_output_0_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  _features_features_7_block_block_1_block_1_2_HardSigmoid_output_0_layer, 69,
  NL_TYPE, 0x0, NULL,
  nl, forward_hard_sigmoid,
  &_features_features_7_block_block_1_block_1_2_HardSigmoid_output_0_chain,
  NULL, &_features_features_7_block_block_1_block_1_2_Mul_output_0_layer, AI_STATIC, 
  .nl_params = &_features_features_7_block_block_1_block_1_2_HardSigmoid_output_0_nl_params, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  _features_features_7_block_block_1_block_1_0_Conv_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_7_block_block_0_block_0_2_Mul_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_7_block_block_1_block_1_0_Conv_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 3, &_features_features_7_block_block_1_block_1_0_Conv_output_0_weights, &_features_features_7_block_block_1_block_1_0_Conv_output_0_bias, NULL),
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  _features_features_7_block_block_1_block_1_0_Conv_output_0_layer, 68,
  CONV2D_TYPE, 0x0, NULL,
  conv2d, forward_dw_if32of32wf32,
  &_features_features_7_block_block_1_block_1_0_Conv_output_0_chain,
  NULL, &_features_features_7_block_block_1_block_1_2_HardSigmoid_output_0_layer, AI_STATIC, 
  .groups = 120, 
  .filter_stride = AI_SHAPE_2D_INIT(1, 1), 
  .dilation = AI_SHAPE_2D_INIT(1, 1), 
  .filter_pad = AI_SHAPE_INIT(4, 2, 2, 2, 2), 
  .in_ch_format = AI_LAYER_FORMAT_CHANNEL_LAST_SAME, 
  .out_ch_format = AI_LAYER_FORMAT_CHANNEL_LAST_VALID, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  _features_features_7_block_block_0_block_0_2_Mul_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 2, &_features_features_7_block_block_0_block_0_0_Conv_output_0_output, &_features_features_7_block_block_0_block_0_2_HardSigmoid_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_7_block_block_0_block_0_2_Mul_output_0_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  _features_features_7_block_block_0_block_0_2_Mul_output_0_layer, 67,
  ELTWISE_TYPE, 0x0, NULL,
  eltwise, forward_eltwise,
  &_features_features_7_block_block_0_block_0_2_Mul_output_0_chain,
  NULL, &_features_features_7_block_block_1_block_1_0_Conv_output_0_layer, AI_STATIC, 
  .operation = ai_mul_f32, 
  .buffer_operation = ai_mul_buffer_f32, 
)


AI_STATIC_CONST ai_float _features_features_7_block_block_0_block_0_2_HardSigmoid_output_0_nl_params_data[] = { 0.1666666716337204, 0.5 };
AI_ARRAY_OBJ_DECLARE(
    _features_features_7_block_block_0_block_0_2_HardSigmoid_output_0_nl_params, AI_ARRAY_FORMAT_FLOAT,
    _features_features_7_block_block_0_block_0_2_HardSigmoid_output_0_nl_params_data, _features_features_7_block_block_0_block_0_2_HardSigmoid_output_0_nl_params_data, 2, AI_STATIC_CONST)
AI_TENSOR_CHAIN_OBJ_DECLARE(
  _features_features_7_block_block_0_block_0_2_HardSigmoid_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_7_block_block_0_block_0_0_Conv_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_7_block_block_0_block_0_2_HardSigmoid_output_0_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  _features_features_7_block_block_0_block_0_2_HardSigmoid_output_0_layer, 66,
  NL_TYPE, 0x0, NULL,
  nl, forward_hard_sigmoid,
  &_features_features_7_block_block_0_block_0_2_HardSigmoid_output_0_chain,
  NULL, &_features_features_7_block_block_0_block_0_2_Mul_output_0_layer, AI_STATIC, 
  .nl_params = &_features_features_7_block_block_0_block_0_2_HardSigmoid_output_0_nl_params, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  _features_features_7_block_block_0_block_0_0_Conv_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_6_Add_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_7_block_block_0_block_0_0_Conv_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 3, &_features_features_7_block_block_0_block_0_0_Conv_output_0_weights, &_features_features_7_block_block_0_block_0_0_Conv_output_0_bias, NULL),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 2, &_features_features_7_block_block_0_block_0_0_Conv_output_0_scratch0, NULL)
)

AI_LAYER_OBJ_DECLARE(
  _features_features_7_block_block_0_block_0_0_Conv_output_0_layer, 65,
  CONV2D_TYPE, 0x0, NULL,
  conv2d, forward_conv2d_if32of32wf32,
  &_features_features_7_block_block_0_block_0_0_Conv_output_0_chain,
  NULL, &_features_features_7_block_block_0_block_0_2_HardSigmoid_output_0_layer, AI_STATIC, 
  .groups = 1, 
  .filter_stride = AI_SHAPE_2D_INIT(1, 1), 
  .dilation = AI_SHAPE_2D_INIT(1, 1), 
  .filter_pad = AI_SHAPE_INIT(4, 0, 0, 0, 0), 
  .in_ch_format = AI_LAYER_FORMAT_CHANNEL_LAST_VALID, 
  .out_ch_format = AI_LAYER_FORMAT_CHANNEL_LAST_VALID, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  _features_features_6_Add_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 2, &_features_features_6_block_block_3_block_3_0_Conv_output_0_output, &_features_features_5_Add_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_6_Add_output_0_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  _features_features_6_Add_output_0_layer, 64,
  ELTWISE_TYPE, 0x0, NULL,
  eltwise, forward_eltwise,
  &_features_features_6_Add_output_0_chain,
  NULL, &_features_features_7_block_block_0_block_0_0_Conv_output_0_layer, AI_STATIC, 
  .operation = ai_sum_f32, 
  .buffer_operation = ai_sum_buffer_f32, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  _features_features_6_block_block_3_block_3_0_Conv_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_6_block_block_2_Mul_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_6_block_block_3_block_3_0_Conv_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 3, &_features_features_6_block_block_3_block_3_0_Conv_output_0_weights, &_features_features_6_block_block_3_block_3_0_Conv_output_0_bias, NULL),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 2, &_features_features_6_block_block_3_block_3_0_Conv_output_0_scratch0, NULL)
)

AI_LAYER_OBJ_DECLARE(
  _features_features_6_block_block_3_block_3_0_Conv_output_0_layer, 63,
  CONV2D_TYPE, 0x0, NULL,
  conv2d, forward_conv2d_if32of32wf32,
  &_features_features_6_block_block_3_block_3_0_Conv_output_0_chain,
  NULL, &_features_features_6_Add_output_0_layer, AI_STATIC, 
  .groups = 1, 
  .filter_stride = AI_SHAPE_2D_INIT(1, 1), 
  .dilation = AI_SHAPE_2D_INIT(1, 1), 
  .filter_pad = AI_SHAPE_INIT(4, 0, 0, 0, 0), 
  .in_ch_format = AI_LAYER_FORMAT_CHANNEL_LAST_VALID, 
  .out_ch_format = AI_LAYER_FORMAT_CHANNEL_LAST_VALID, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  _features_features_6_block_block_2_Mul_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 2, &_features_features_6_block_block_2_scale_activation_HardSigmoid_output_0_output, &_features_features_6_block_block_1_block_1_2_Mul_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_6_block_block_2_Mul_output_0_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  _features_features_6_block_block_2_Mul_output_0_layer, 62,
  ELTWISE_TYPE, 0x0, NULL,
  eltwise, forward_eltwise,
  &_features_features_6_block_block_2_Mul_output_0_chain,
  NULL, &_features_features_6_block_block_3_block_3_0_Conv_output_0_layer, AI_STATIC, 
  .operation = ai_mul_f32, 
  .buffer_operation = ai_mul_buffer_f32, 
)


AI_STATIC_CONST ai_float _features_features_6_block_block_2_scale_activation_HardSigmoid_output_0_nl_params_data[] = { 0.1666666716337204, 0.5 };
AI_ARRAY_OBJ_DECLARE(
    _features_features_6_block_block_2_scale_activation_HardSigmoid_output_0_nl_params, AI_ARRAY_FORMAT_FLOAT,
    _features_features_6_block_block_2_scale_activation_HardSigmoid_output_0_nl_params_data, _features_features_6_block_block_2_scale_activation_HardSigmoid_output_0_nl_params_data, 2, AI_STATIC_CONST)
AI_TENSOR_CHAIN_OBJ_DECLARE(
  _features_features_6_block_block_2_scale_activation_HardSigmoid_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_6_block_block_2_fc2_Conv_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_6_block_block_2_scale_activation_HardSigmoid_output_0_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  _features_features_6_block_block_2_scale_activation_HardSigmoid_output_0_layer, 61,
  NL_TYPE, 0x0, NULL,
  nl, forward_hard_sigmoid,
  &_features_features_6_block_block_2_scale_activation_HardSigmoid_output_0_chain,
  NULL, &_features_features_6_block_block_2_Mul_output_0_layer, AI_STATIC, 
  .nl_params = &_features_features_6_block_block_2_scale_activation_HardSigmoid_output_0_nl_params, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  _features_features_6_block_block_2_fc2_Conv_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_6_block_block_2_activation_Relu_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_6_block_block_2_fc2_Conv_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 3, &_features_features_6_block_block_2_fc2_Conv_output_0_weights, &_features_features_6_block_block_2_fc2_Conv_output_0_bias, NULL),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 2, &_features_features_6_block_block_2_fc2_Conv_output_0_scratch0, NULL)
)

AI_LAYER_OBJ_DECLARE(
  _features_features_6_block_block_2_fc2_Conv_output_0_layer, 60,
  CONV2D_TYPE, 0x0, NULL,
  conv2d, forward_conv2d_if32of32wf32,
  &_features_features_6_block_block_2_fc2_Conv_output_0_chain,
  NULL, &_features_features_6_block_block_2_scale_activation_HardSigmoid_output_0_layer, AI_STATIC, 
  .groups = 1, 
  .filter_stride = AI_SHAPE_2D_INIT(1, 1), 
  .dilation = AI_SHAPE_2D_INIT(1, 1), 
  .filter_pad = AI_SHAPE_INIT(4, 0, 0, 0, 0), 
  .in_ch_format = AI_LAYER_FORMAT_CHANNEL_LAST_VALID, 
  .out_ch_format = AI_LAYER_FORMAT_CHANNEL_LAST_VALID, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  _features_features_6_block_block_2_activation_Relu_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_6_block_block_2_fc1_Conv_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_6_block_block_2_activation_Relu_output_0_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  _features_features_6_block_block_2_activation_Relu_output_0_layer, 59,
  NL_TYPE, 0x0, NULL,
  nl, forward_relu,
  &_features_features_6_block_block_2_activation_Relu_output_0_chain,
  NULL, &_features_features_6_block_block_2_fc2_Conv_output_0_layer, AI_STATIC, 
  .nl_params = NULL, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  _features_features_6_block_block_2_fc1_Conv_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_6_block_block_2_avgpool_GlobalAveragePool_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_6_block_block_2_fc1_Conv_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 3, &_features_features_6_block_block_2_fc1_Conv_output_0_weights, &_features_features_6_block_block_2_fc1_Conv_output_0_bias, NULL),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 2, &_features_features_6_block_block_2_fc1_Conv_output_0_scratch0, NULL)
)

AI_LAYER_OBJ_DECLARE(
  _features_features_6_block_block_2_fc1_Conv_output_0_layer, 58,
  CONV2D_TYPE, 0x0, NULL,
  conv2d, forward_conv2d_if32of32wf32,
  &_features_features_6_block_block_2_fc1_Conv_output_0_chain,
  NULL, &_features_features_6_block_block_2_activation_Relu_output_0_layer, AI_STATIC, 
  .groups = 1, 
  .filter_stride = AI_SHAPE_2D_INIT(1, 1), 
  .dilation = AI_SHAPE_2D_INIT(1, 1), 
  .filter_pad = AI_SHAPE_INIT(4, 0, 0, 0, 0), 
  .in_ch_format = AI_LAYER_FORMAT_CHANNEL_LAST_VALID, 
  .out_ch_format = AI_LAYER_FORMAT_CHANNEL_LAST_VALID, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  _features_features_6_block_block_2_avgpool_GlobalAveragePool_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_6_block_block_1_block_1_2_Mul_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_6_block_block_2_avgpool_GlobalAveragePool_output_0_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  _features_features_6_block_block_2_avgpool_GlobalAveragePool_output_0_layer, 57,
  POOL_TYPE, 0x0, NULL,
  pool, forward_ap,
  &_features_features_6_block_block_2_avgpool_GlobalAveragePool_output_0_chain,
  NULL, &_features_features_6_block_block_2_fc1_Conv_output_0_layer, AI_STATIC, 
  .pool_size = AI_SHAPE_2D_INIT(43, 2), 
  .pool_stride = AI_SHAPE_2D_INIT(43, 2), 
  .pool_pad = AI_SHAPE_INIT(4, 0, 0, 0, 0), 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  _features_features_6_block_block_1_block_1_2_Mul_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 2, &_features_features_6_block_block_1_block_1_0_Conv_output_0_output, &_features_features_6_block_block_1_block_1_2_HardSigmoid_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_6_block_block_1_block_1_2_Mul_output_0_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  _features_features_6_block_block_1_block_1_2_Mul_output_0_layer, 56,
  ELTWISE_TYPE, 0x0, NULL,
  eltwise, forward_eltwise,
  &_features_features_6_block_block_1_block_1_2_Mul_output_0_chain,
  NULL, &_features_features_6_block_block_2_avgpool_GlobalAveragePool_output_0_layer, AI_STATIC, 
  .operation = ai_mul_f32, 
  .buffer_operation = ai_mul_buffer_f32, 
)


AI_STATIC_CONST ai_float _features_features_6_block_block_1_block_1_2_HardSigmoid_output_0_nl_params_data[] = { 0.1666666716337204, 0.5 };
AI_ARRAY_OBJ_DECLARE(
    _features_features_6_block_block_1_block_1_2_HardSigmoid_output_0_nl_params, AI_ARRAY_FORMAT_FLOAT,
    _features_features_6_block_block_1_block_1_2_HardSigmoid_output_0_nl_params_data, _features_features_6_block_block_1_block_1_2_HardSigmoid_output_0_nl_params_data, 2, AI_STATIC_CONST)
AI_TENSOR_CHAIN_OBJ_DECLARE(
  _features_features_6_block_block_1_block_1_2_HardSigmoid_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_6_block_block_1_block_1_0_Conv_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_6_block_block_1_block_1_2_HardSigmoid_output_0_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  _features_features_6_block_block_1_block_1_2_HardSigmoid_output_0_layer, 55,
  NL_TYPE, 0x0, NULL,
  nl, forward_hard_sigmoid,
  &_features_features_6_block_block_1_block_1_2_HardSigmoid_output_0_chain,
  NULL, &_features_features_6_block_block_1_block_1_2_Mul_output_0_layer, AI_STATIC, 
  .nl_params = &_features_features_6_block_block_1_block_1_2_HardSigmoid_output_0_nl_params, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  _features_features_6_block_block_1_block_1_0_Conv_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_6_block_block_0_block_0_2_Mul_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_6_block_block_1_block_1_0_Conv_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 3, &_features_features_6_block_block_1_block_1_0_Conv_output_0_weights, &_features_features_6_block_block_1_block_1_0_Conv_output_0_bias, NULL),
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  _features_features_6_block_block_1_block_1_0_Conv_output_0_layer, 54,
  CONV2D_TYPE, 0x0, NULL,
  conv2d, forward_dw_if32of32wf32,
  &_features_features_6_block_block_1_block_1_0_Conv_output_0_chain,
  NULL, &_features_features_6_block_block_1_block_1_2_HardSigmoid_output_0_layer, AI_STATIC, 
  .groups = 240, 
  .filter_stride = AI_SHAPE_2D_INIT(1, 1), 
  .dilation = AI_SHAPE_2D_INIT(1, 1), 
  .filter_pad = AI_SHAPE_INIT(4, 2, 2, 2, 2), 
  .in_ch_format = AI_LAYER_FORMAT_CHANNEL_LAST_SAME, 
  .out_ch_format = AI_LAYER_FORMAT_CHANNEL_LAST_VALID, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  _features_features_6_block_block_0_block_0_2_Mul_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 2, &_features_features_6_block_block_0_block_0_0_Conv_output_0_output, &_features_features_6_block_block_0_block_0_2_HardSigmoid_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_6_block_block_0_block_0_2_Mul_output_0_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  _features_features_6_block_block_0_block_0_2_Mul_output_0_layer, 53,
  ELTWISE_TYPE, 0x0, NULL,
  eltwise, forward_eltwise,
  &_features_features_6_block_block_0_block_0_2_Mul_output_0_chain,
  NULL, &_features_features_6_block_block_1_block_1_0_Conv_output_0_layer, AI_STATIC, 
  .operation = ai_mul_f32, 
  .buffer_operation = ai_mul_buffer_f32, 
)


AI_STATIC_CONST ai_float _features_features_6_block_block_0_block_0_2_HardSigmoid_output_0_nl_params_data[] = { 0.1666666716337204, 0.5 };
AI_ARRAY_OBJ_DECLARE(
    _features_features_6_block_block_0_block_0_2_HardSigmoid_output_0_nl_params, AI_ARRAY_FORMAT_FLOAT,
    _features_features_6_block_block_0_block_0_2_HardSigmoid_output_0_nl_params_data, _features_features_6_block_block_0_block_0_2_HardSigmoid_output_0_nl_params_data, 2, AI_STATIC_CONST)
AI_TENSOR_CHAIN_OBJ_DECLARE(
  _features_features_6_block_block_0_block_0_2_HardSigmoid_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_6_block_block_0_block_0_0_Conv_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_6_block_block_0_block_0_2_HardSigmoid_output_0_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  _features_features_6_block_block_0_block_0_2_HardSigmoid_output_0_layer, 52,
  NL_TYPE, 0x0, NULL,
  nl, forward_hard_sigmoid,
  &_features_features_6_block_block_0_block_0_2_HardSigmoid_output_0_chain,
  NULL, &_features_features_6_block_block_0_block_0_2_Mul_output_0_layer, AI_STATIC, 
  .nl_params = &_features_features_6_block_block_0_block_0_2_HardSigmoid_output_0_nl_params, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  _features_features_6_block_block_0_block_0_0_Conv_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_5_Add_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_6_block_block_0_block_0_0_Conv_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 3, &_features_features_6_block_block_0_block_0_0_Conv_output_0_weights, &_features_features_6_block_block_0_block_0_0_Conv_output_0_bias, NULL),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 2, &_features_features_6_block_block_0_block_0_0_Conv_output_0_scratch0, NULL)
)

AI_LAYER_OBJ_DECLARE(
  _features_features_6_block_block_0_block_0_0_Conv_output_0_layer, 51,
  CONV2D_TYPE, 0x0, NULL,
  conv2d, forward_conv2d_if32of32wf32,
  &_features_features_6_block_block_0_block_0_0_Conv_output_0_chain,
  NULL, &_features_features_6_block_block_0_block_0_2_HardSigmoid_output_0_layer, AI_STATIC, 
  .groups = 1, 
  .filter_stride = AI_SHAPE_2D_INIT(1, 1), 
  .dilation = AI_SHAPE_2D_INIT(1, 1), 
  .filter_pad = AI_SHAPE_INIT(4, 0, 0, 0, 0), 
  .in_ch_format = AI_LAYER_FORMAT_CHANNEL_LAST_VALID, 
  .out_ch_format = AI_LAYER_FORMAT_CHANNEL_LAST_VALID, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  _features_features_5_Add_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 2, &_features_features_5_block_block_3_block_3_0_Conv_output_0_output, &_features_features_4_block_block_3_block_3_0_Conv_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_5_Add_output_0_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  _features_features_5_Add_output_0_layer, 50,
  ELTWISE_TYPE, 0x0, NULL,
  eltwise, forward_eltwise,
  &_features_features_5_Add_output_0_chain,
  NULL, &_features_features_6_block_block_0_block_0_0_Conv_output_0_layer, AI_STATIC, 
  .operation = ai_sum_f32, 
  .buffer_operation = ai_sum_buffer_f32, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  _features_features_5_block_block_3_block_3_0_Conv_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_5_block_block_2_Mul_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_5_block_block_3_block_3_0_Conv_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 3, &_features_features_5_block_block_3_block_3_0_Conv_output_0_weights, &_features_features_5_block_block_3_block_3_0_Conv_output_0_bias, NULL),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 2, &_features_features_5_block_block_3_block_3_0_Conv_output_0_scratch0, NULL)
)

AI_LAYER_OBJ_DECLARE(
  _features_features_5_block_block_3_block_3_0_Conv_output_0_layer, 49,
  CONV2D_TYPE, 0x0, NULL,
  conv2d, forward_conv2d_if32of32wf32,
  &_features_features_5_block_block_3_block_3_0_Conv_output_0_chain,
  NULL, &_features_features_5_Add_output_0_layer, AI_STATIC, 
  .groups = 1, 
  .filter_stride = AI_SHAPE_2D_INIT(1, 1), 
  .dilation = AI_SHAPE_2D_INIT(1, 1), 
  .filter_pad = AI_SHAPE_INIT(4, 0, 0, 0, 0), 
  .in_ch_format = AI_LAYER_FORMAT_CHANNEL_LAST_VALID, 
  .out_ch_format = AI_LAYER_FORMAT_CHANNEL_LAST_VALID, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  _features_features_5_block_block_2_Mul_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 2, &_features_features_5_block_block_2_scale_activation_HardSigmoid_output_0_output, &_features_features_5_block_block_1_block_1_2_Mul_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_5_block_block_2_Mul_output_0_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  _features_features_5_block_block_2_Mul_output_0_layer, 48,
  ELTWISE_TYPE, 0x0, NULL,
  eltwise, forward_eltwise,
  &_features_features_5_block_block_2_Mul_output_0_chain,
  NULL, &_features_features_5_block_block_3_block_3_0_Conv_output_0_layer, AI_STATIC, 
  .operation = ai_mul_f32, 
  .buffer_operation = ai_mul_buffer_f32, 
)


AI_STATIC_CONST ai_float _features_features_5_block_block_2_scale_activation_HardSigmoid_output_0_nl_params_data[] = { 0.1666666716337204, 0.5 };
AI_ARRAY_OBJ_DECLARE(
    _features_features_5_block_block_2_scale_activation_HardSigmoid_output_0_nl_params, AI_ARRAY_FORMAT_FLOAT,
    _features_features_5_block_block_2_scale_activation_HardSigmoid_output_0_nl_params_data, _features_features_5_block_block_2_scale_activation_HardSigmoid_output_0_nl_params_data, 2, AI_STATIC_CONST)
AI_TENSOR_CHAIN_OBJ_DECLARE(
  _features_features_5_block_block_2_scale_activation_HardSigmoid_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_5_block_block_2_fc2_Conv_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_5_block_block_2_scale_activation_HardSigmoid_output_0_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  _features_features_5_block_block_2_scale_activation_HardSigmoid_output_0_layer, 47,
  NL_TYPE, 0x0, NULL,
  nl, forward_hard_sigmoid,
  &_features_features_5_block_block_2_scale_activation_HardSigmoid_output_0_chain,
  NULL, &_features_features_5_block_block_2_Mul_output_0_layer, AI_STATIC, 
  .nl_params = &_features_features_5_block_block_2_scale_activation_HardSigmoid_output_0_nl_params, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  _features_features_5_block_block_2_fc2_Conv_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_5_block_block_2_activation_Relu_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_5_block_block_2_fc2_Conv_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 3, &_features_features_5_block_block_2_fc2_Conv_output_0_weights, &_features_features_5_block_block_2_fc2_Conv_output_0_bias, NULL),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 2, &_features_features_5_block_block_2_fc2_Conv_output_0_scratch0, NULL)
)

AI_LAYER_OBJ_DECLARE(
  _features_features_5_block_block_2_fc2_Conv_output_0_layer, 46,
  CONV2D_TYPE, 0x0, NULL,
  conv2d, forward_conv2d_if32of32wf32,
  &_features_features_5_block_block_2_fc2_Conv_output_0_chain,
  NULL, &_features_features_5_block_block_2_scale_activation_HardSigmoid_output_0_layer, AI_STATIC, 
  .groups = 1, 
  .filter_stride = AI_SHAPE_2D_INIT(1, 1), 
  .dilation = AI_SHAPE_2D_INIT(1, 1), 
  .filter_pad = AI_SHAPE_INIT(4, 0, 0, 0, 0), 
  .in_ch_format = AI_LAYER_FORMAT_CHANNEL_LAST_VALID, 
  .out_ch_format = AI_LAYER_FORMAT_CHANNEL_LAST_VALID, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  _features_features_5_block_block_2_activation_Relu_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_5_block_block_2_fc1_Conv_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_5_block_block_2_activation_Relu_output_0_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  _features_features_5_block_block_2_activation_Relu_output_0_layer, 45,
  NL_TYPE, 0x0, NULL,
  nl, forward_relu,
  &_features_features_5_block_block_2_activation_Relu_output_0_chain,
  NULL, &_features_features_5_block_block_2_fc2_Conv_output_0_layer, AI_STATIC, 
  .nl_params = NULL, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  _features_features_5_block_block_2_fc1_Conv_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_5_block_block_2_avgpool_GlobalAveragePool_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_5_block_block_2_fc1_Conv_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 3, &_features_features_5_block_block_2_fc1_Conv_output_0_weights, &_features_features_5_block_block_2_fc1_Conv_output_0_bias, NULL),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 2, &_features_features_5_block_block_2_fc1_Conv_output_0_scratch0, NULL)
)

AI_LAYER_OBJ_DECLARE(
  _features_features_5_block_block_2_fc1_Conv_output_0_layer, 44,
  CONV2D_TYPE, 0x0, NULL,
  conv2d, forward_conv2d_if32of32wf32,
  &_features_features_5_block_block_2_fc1_Conv_output_0_chain,
  NULL, &_features_features_5_block_block_2_activation_Relu_output_0_layer, AI_STATIC, 
  .groups = 1, 
  .filter_stride = AI_SHAPE_2D_INIT(1, 1), 
  .dilation = AI_SHAPE_2D_INIT(1, 1), 
  .filter_pad = AI_SHAPE_INIT(4, 0, 0, 0, 0), 
  .in_ch_format = AI_LAYER_FORMAT_CHANNEL_LAST_VALID, 
  .out_ch_format = AI_LAYER_FORMAT_CHANNEL_LAST_VALID, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  _features_features_5_block_block_2_avgpool_GlobalAveragePool_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_5_block_block_1_block_1_2_Mul_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_5_block_block_2_avgpool_GlobalAveragePool_output_0_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  _features_features_5_block_block_2_avgpool_GlobalAveragePool_output_0_layer, 43,
  POOL_TYPE, 0x0, NULL,
  pool, forward_ap,
  &_features_features_5_block_block_2_avgpool_GlobalAveragePool_output_0_chain,
  NULL, &_features_features_5_block_block_2_fc1_Conv_output_0_layer, AI_STATIC, 
  .pool_size = AI_SHAPE_2D_INIT(43, 2), 
  .pool_stride = AI_SHAPE_2D_INIT(43, 2), 
  .pool_pad = AI_SHAPE_INIT(4, 0, 0, 0, 0), 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  _features_features_5_block_block_1_block_1_2_Mul_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 2, &_features_features_5_block_block_1_block_1_0_Conv_output_0_output, &_features_features_5_block_block_1_block_1_2_HardSigmoid_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_5_block_block_1_block_1_2_Mul_output_0_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  _features_features_5_block_block_1_block_1_2_Mul_output_0_layer, 42,
  ELTWISE_TYPE, 0x0, NULL,
  eltwise, forward_eltwise,
  &_features_features_5_block_block_1_block_1_2_Mul_output_0_chain,
  NULL, &_features_features_5_block_block_2_avgpool_GlobalAveragePool_output_0_layer, AI_STATIC, 
  .operation = ai_mul_f32, 
  .buffer_operation = ai_mul_buffer_f32, 
)


AI_STATIC_CONST ai_float _features_features_5_block_block_1_block_1_2_HardSigmoid_output_0_nl_params_data[] = { 0.1666666716337204, 0.5 };
AI_ARRAY_OBJ_DECLARE(
    _features_features_5_block_block_1_block_1_2_HardSigmoid_output_0_nl_params, AI_ARRAY_FORMAT_FLOAT,
    _features_features_5_block_block_1_block_1_2_HardSigmoid_output_0_nl_params_data, _features_features_5_block_block_1_block_1_2_HardSigmoid_output_0_nl_params_data, 2, AI_STATIC_CONST)
AI_TENSOR_CHAIN_OBJ_DECLARE(
  _features_features_5_block_block_1_block_1_2_HardSigmoid_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_5_block_block_1_block_1_0_Conv_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_5_block_block_1_block_1_2_HardSigmoid_output_0_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  _features_features_5_block_block_1_block_1_2_HardSigmoid_output_0_layer, 41,
  NL_TYPE, 0x0, NULL,
  nl, forward_hard_sigmoid,
  &_features_features_5_block_block_1_block_1_2_HardSigmoid_output_0_chain,
  NULL, &_features_features_5_block_block_1_block_1_2_Mul_output_0_layer, AI_STATIC, 
  .nl_params = &_features_features_5_block_block_1_block_1_2_HardSigmoid_output_0_nl_params, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  _features_features_5_block_block_1_block_1_0_Conv_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_5_block_block_0_block_0_2_Mul_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_5_block_block_1_block_1_0_Conv_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 3, &_features_features_5_block_block_1_block_1_0_Conv_output_0_weights, &_features_features_5_block_block_1_block_1_0_Conv_output_0_bias, NULL),
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  _features_features_5_block_block_1_block_1_0_Conv_output_0_layer, 40,
  CONV2D_TYPE, 0x0, NULL,
  conv2d, forward_dw_if32of32wf32,
  &_features_features_5_block_block_1_block_1_0_Conv_output_0_chain,
  NULL, &_features_features_5_block_block_1_block_1_2_HardSigmoid_output_0_layer, AI_STATIC, 
  .groups = 240, 
  .filter_stride = AI_SHAPE_2D_INIT(1, 1), 
  .dilation = AI_SHAPE_2D_INIT(1, 1), 
  .filter_pad = AI_SHAPE_INIT(4, 2, 2, 2, 2), 
  .in_ch_format = AI_LAYER_FORMAT_CHANNEL_LAST_SAME, 
  .out_ch_format = AI_LAYER_FORMAT_CHANNEL_LAST_VALID, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  _features_features_5_block_block_0_block_0_2_Mul_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 2, &_features_features_5_block_block_0_block_0_0_Conv_output_0_output, &_features_features_5_block_block_0_block_0_2_HardSigmoid_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_5_block_block_0_block_0_2_Mul_output_0_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  _features_features_5_block_block_0_block_0_2_Mul_output_0_layer, 39,
  ELTWISE_TYPE, 0x0, NULL,
  eltwise, forward_eltwise,
  &_features_features_5_block_block_0_block_0_2_Mul_output_0_chain,
  NULL, &_features_features_5_block_block_1_block_1_0_Conv_output_0_layer, AI_STATIC, 
  .operation = ai_mul_f32, 
  .buffer_operation = ai_mul_buffer_f32, 
)


AI_STATIC_CONST ai_float _features_features_5_block_block_0_block_0_2_HardSigmoid_output_0_nl_params_data[] = { 0.1666666716337204, 0.5 };
AI_ARRAY_OBJ_DECLARE(
    _features_features_5_block_block_0_block_0_2_HardSigmoid_output_0_nl_params, AI_ARRAY_FORMAT_FLOAT,
    _features_features_5_block_block_0_block_0_2_HardSigmoid_output_0_nl_params_data, _features_features_5_block_block_0_block_0_2_HardSigmoid_output_0_nl_params_data, 2, AI_STATIC_CONST)
AI_TENSOR_CHAIN_OBJ_DECLARE(
  _features_features_5_block_block_0_block_0_2_HardSigmoid_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_5_block_block_0_block_0_0_Conv_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_5_block_block_0_block_0_2_HardSigmoid_output_0_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  _features_features_5_block_block_0_block_0_2_HardSigmoid_output_0_layer, 38,
  NL_TYPE, 0x0, NULL,
  nl, forward_hard_sigmoid,
  &_features_features_5_block_block_0_block_0_2_HardSigmoid_output_0_chain,
  NULL, &_features_features_5_block_block_0_block_0_2_Mul_output_0_layer, AI_STATIC, 
  .nl_params = &_features_features_5_block_block_0_block_0_2_HardSigmoid_output_0_nl_params, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  _features_features_5_block_block_0_block_0_0_Conv_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_4_block_block_3_block_3_0_Conv_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_5_block_block_0_block_0_0_Conv_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 3, &_features_features_5_block_block_0_block_0_0_Conv_output_0_weights, &_features_features_5_block_block_0_block_0_0_Conv_output_0_bias, NULL),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 2, &_features_features_5_block_block_0_block_0_0_Conv_output_0_scratch0, NULL)
)

AI_LAYER_OBJ_DECLARE(
  _features_features_5_block_block_0_block_0_0_Conv_output_0_layer, 37,
  CONV2D_TYPE, 0x0, NULL,
  conv2d, forward_conv2d_if32of32wf32,
  &_features_features_5_block_block_0_block_0_0_Conv_output_0_chain,
  NULL, &_features_features_5_block_block_0_block_0_2_HardSigmoid_output_0_layer, AI_STATIC, 
  .groups = 1, 
  .filter_stride = AI_SHAPE_2D_INIT(1, 1), 
  .dilation = AI_SHAPE_2D_INIT(1, 1), 
  .filter_pad = AI_SHAPE_INIT(4, 0, 0, 0, 0), 
  .in_ch_format = AI_LAYER_FORMAT_CHANNEL_LAST_VALID, 
  .out_ch_format = AI_LAYER_FORMAT_CHANNEL_LAST_VALID, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  _features_features_4_block_block_3_block_3_0_Conv_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_4_block_block_2_Mul_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_4_block_block_3_block_3_0_Conv_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 3, &_features_features_4_block_block_3_block_3_0_Conv_output_0_weights, &_features_features_4_block_block_3_block_3_0_Conv_output_0_bias, NULL),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 2, &_features_features_4_block_block_3_block_3_0_Conv_output_0_scratch0, NULL)
)

AI_LAYER_OBJ_DECLARE(
  _features_features_4_block_block_3_block_3_0_Conv_output_0_layer, 36,
  CONV2D_TYPE, 0x0, NULL,
  conv2d, forward_conv2d_if32of32wf32,
  &_features_features_4_block_block_3_block_3_0_Conv_output_0_chain,
  NULL, &_features_features_5_block_block_0_block_0_0_Conv_output_0_layer, AI_STATIC, 
  .groups = 1, 
  .filter_stride = AI_SHAPE_2D_INIT(1, 1), 
  .dilation = AI_SHAPE_2D_INIT(1, 1), 
  .filter_pad = AI_SHAPE_INIT(4, 0, 0, 0, 0), 
  .in_ch_format = AI_LAYER_FORMAT_CHANNEL_LAST_VALID, 
  .out_ch_format = AI_LAYER_FORMAT_CHANNEL_LAST_VALID, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  _features_features_4_block_block_2_Mul_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 2, &_features_features_4_block_block_2_scale_activation_HardSigmoid_output_0_output, &_features_features_4_block_block_1_block_1_2_Mul_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_4_block_block_2_Mul_output_0_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  _features_features_4_block_block_2_Mul_output_0_layer, 35,
  ELTWISE_TYPE, 0x0, NULL,
  eltwise, forward_eltwise,
  &_features_features_4_block_block_2_Mul_output_0_chain,
  NULL, &_features_features_4_block_block_3_block_3_0_Conv_output_0_layer, AI_STATIC, 
  .operation = ai_mul_f32, 
  .buffer_operation = ai_mul_buffer_f32, 
)


AI_STATIC_CONST ai_float _features_features_4_block_block_2_scale_activation_HardSigmoid_output_0_nl_params_data[] = { 0.1666666716337204, 0.5 };
AI_ARRAY_OBJ_DECLARE(
    _features_features_4_block_block_2_scale_activation_HardSigmoid_output_0_nl_params, AI_ARRAY_FORMAT_FLOAT,
    _features_features_4_block_block_2_scale_activation_HardSigmoid_output_0_nl_params_data, _features_features_4_block_block_2_scale_activation_HardSigmoid_output_0_nl_params_data, 2, AI_STATIC_CONST)
AI_TENSOR_CHAIN_OBJ_DECLARE(
  _features_features_4_block_block_2_scale_activation_HardSigmoid_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_4_block_block_2_fc2_Conv_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_4_block_block_2_scale_activation_HardSigmoid_output_0_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  _features_features_4_block_block_2_scale_activation_HardSigmoid_output_0_layer, 34,
  NL_TYPE, 0x0, NULL,
  nl, forward_hard_sigmoid,
  &_features_features_4_block_block_2_scale_activation_HardSigmoid_output_0_chain,
  NULL, &_features_features_4_block_block_2_Mul_output_0_layer, AI_STATIC, 
  .nl_params = &_features_features_4_block_block_2_scale_activation_HardSigmoid_output_0_nl_params, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  _features_features_4_block_block_2_fc2_Conv_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_4_block_block_2_activation_Relu_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_4_block_block_2_fc2_Conv_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 3, &_features_features_4_block_block_2_fc2_Conv_output_0_weights, &_features_features_4_block_block_2_fc2_Conv_output_0_bias, NULL),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 2, &_features_features_4_block_block_2_fc2_Conv_output_0_scratch0, NULL)
)

AI_LAYER_OBJ_DECLARE(
  _features_features_4_block_block_2_fc2_Conv_output_0_layer, 33,
  CONV2D_TYPE, 0x0, NULL,
  conv2d, forward_conv2d_if32of32wf32,
  &_features_features_4_block_block_2_fc2_Conv_output_0_chain,
  NULL, &_features_features_4_block_block_2_scale_activation_HardSigmoid_output_0_layer, AI_STATIC, 
  .groups = 1, 
  .filter_stride = AI_SHAPE_2D_INIT(1, 1), 
  .dilation = AI_SHAPE_2D_INIT(1, 1), 
  .filter_pad = AI_SHAPE_INIT(4, 0, 0, 0, 0), 
  .in_ch_format = AI_LAYER_FORMAT_CHANNEL_LAST_VALID, 
  .out_ch_format = AI_LAYER_FORMAT_CHANNEL_LAST_VALID, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  _features_features_4_block_block_2_activation_Relu_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_4_block_block_2_fc1_Conv_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_4_block_block_2_activation_Relu_output_0_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  _features_features_4_block_block_2_activation_Relu_output_0_layer, 32,
  NL_TYPE, 0x0, NULL,
  nl, forward_relu,
  &_features_features_4_block_block_2_activation_Relu_output_0_chain,
  NULL, &_features_features_4_block_block_2_fc2_Conv_output_0_layer, AI_STATIC, 
  .nl_params = NULL, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  _features_features_4_block_block_2_fc1_Conv_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_4_block_block_2_avgpool_GlobalAveragePool_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_4_block_block_2_fc1_Conv_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 3, &_features_features_4_block_block_2_fc1_Conv_output_0_weights, &_features_features_4_block_block_2_fc1_Conv_output_0_bias, NULL),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 2, &_features_features_4_block_block_2_fc1_Conv_output_0_scratch0, NULL)
)

AI_LAYER_OBJ_DECLARE(
  _features_features_4_block_block_2_fc1_Conv_output_0_layer, 31,
  CONV2D_TYPE, 0x0, NULL,
  conv2d, forward_conv2d_if32of32wf32,
  &_features_features_4_block_block_2_fc1_Conv_output_0_chain,
  NULL, &_features_features_4_block_block_2_activation_Relu_output_0_layer, AI_STATIC, 
  .groups = 1, 
  .filter_stride = AI_SHAPE_2D_INIT(1, 1), 
  .dilation = AI_SHAPE_2D_INIT(1, 1), 
  .filter_pad = AI_SHAPE_INIT(4, 0, 0, 0, 0), 
  .in_ch_format = AI_LAYER_FORMAT_CHANNEL_LAST_VALID, 
  .out_ch_format = AI_LAYER_FORMAT_CHANNEL_LAST_VALID, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  _features_features_4_block_block_2_avgpool_GlobalAveragePool_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_4_block_block_1_block_1_2_Mul_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_4_block_block_2_avgpool_GlobalAveragePool_output_0_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  _features_features_4_block_block_2_avgpool_GlobalAveragePool_output_0_layer, 30,
  POOL_TYPE, 0x0, NULL,
  pool, forward_ap,
  &_features_features_4_block_block_2_avgpool_GlobalAveragePool_output_0_chain,
  NULL, &_features_features_4_block_block_2_fc1_Conv_output_0_layer, AI_STATIC, 
  .pool_size = AI_SHAPE_2D_INIT(43, 2), 
  .pool_stride = AI_SHAPE_2D_INIT(43, 2), 
  .pool_pad = AI_SHAPE_INIT(4, 0, 0, 0, 0), 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  _features_features_4_block_block_1_block_1_2_Mul_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 2, &_features_features_4_block_block_1_block_1_0_Conv_output_0_output, &_features_features_4_block_block_1_block_1_2_HardSigmoid_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_4_block_block_1_block_1_2_Mul_output_0_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  _features_features_4_block_block_1_block_1_2_Mul_output_0_layer, 29,
  ELTWISE_TYPE, 0x0, NULL,
  eltwise, forward_eltwise,
  &_features_features_4_block_block_1_block_1_2_Mul_output_0_chain,
  NULL, &_features_features_4_block_block_2_avgpool_GlobalAveragePool_output_0_layer, AI_STATIC, 
  .operation = ai_mul_f32, 
  .buffer_operation = ai_mul_buffer_f32, 
)


AI_STATIC_CONST ai_float _features_features_4_block_block_1_block_1_2_HardSigmoid_output_0_nl_params_data[] = { 0.1666666716337204, 0.5 };
AI_ARRAY_OBJ_DECLARE(
    _features_features_4_block_block_1_block_1_2_HardSigmoid_output_0_nl_params, AI_ARRAY_FORMAT_FLOAT,
    _features_features_4_block_block_1_block_1_2_HardSigmoid_output_0_nl_params_data, _features_features_4_block_block_1_block_1_2_HardSigmoid_output_0_nl_params_data, 2, AI_STATIC_CONST)
AI_TENSOR_CHAIN_OBJ_DECLARE(
  _features_features_4_block_block_1_block_1_2_HardSigmoid_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_4_block_block_1_block_1_0_Conv_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_4_block_block_1_block_1_2_HardSigmoid_output_0_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  _features_features_4_block_block_1_block_1_2_HardSigmoid_output_0_layer, 28,
  NL_TYPE, 0x0, NULL,
  nl, forward_hard_sigmoid,
  &_features_features_4_block_block_1_block_1_2_HardSigmoid_output_0_chain,
  NULL, &_features_features_4_block_block_1_block_1_2_Mul_output_0_layer, AI_STATIC, 
  .nl_params = &_features_features_4_block_block_1_block_1_2_HardSigmoid_output_0_nl_params, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  _features_features_4_block_block_1_block_1_0_Conv_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_4_block_block_0_block_0_2_Mul_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_4_block_block_1_block_1_0_Conv_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 3, &_features_features_4_block_block_1_block_1_0_Conv_output_0_weights, &_features_features_4_block_block_1_block_1_0_Conv_output_0_bias, NULL),
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  _features_features_4_block_block_1_block_1_0_Conv_output_0_layer, 27,
  CONV2D_TYPE, 0x0, NULL,
  conv2d, forward_dw_if32of32wf32,
  &_features_features_4_block_block_1_block_1_0_Conv_output_0_chain,
  NULL, &_features_features_4_block_block_1_block_1_2_HardSigmoid_output_0_layer, AI_STATIC, 
  .groups = 96, 
  .filter_stride = AI_SHAPE_2D_INIT(2, 2), 
  .dilation = AI_SHAPE_2D_INIT(1, 1), 
  .filter_pad = AI_SHAPE_INIT(4, 2, 2, 2, 2), 
  .in_ch_format = AI_LAYER_FORMAT_CHANNEL_LAST_SAME, 
  .out_ch_format = AI_LAYER_FORMAT_CHANNEL_LAST_VALID, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  _features_features_4_block_block_0_block_0_2_Mul_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 2, &_features_features_4_block_block_0_block_0_0_Conv_output_0_output, &_features_features_4_block_block_0_block_0_2_HardSigmoid_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_4_block_block_0_block_0_2_Mul_output_0_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  _features_features_4_block_block_0_block_0_2_Mul_output_0_layer, 26,
  ELTWISE_TYPE, 0x0, NULL,
  eltwise, forward_eltwise,
  &_features_features_4_block_block_0_block_0_2_Mul_output_0_chain,
  NULL, &_features_features_4_block_block_1_block_1_0_Conv_output_0_layer, AI_STATIC, 
  .operation = ai_mul_f32, 
  .buffer_operation = ai_mul_buffer_f32, 
)


AI_STATIC_CONST ai_float _features_features_4_block_block_0_block_0_2_HardSigmoid_output_0_nl_params_data[] = { 0.1666666716337204, 0.5 };
AI_ARRAY_OBJ_DECLARE(
    _features_features_4_block_block_0_block_0_2_HardSigmoid_output_0_nl_params, AI_ARRAY_FORMAT_FLOAT,
    _features_features_4_block_block_0_block_0_2_HardSigmoid_output_0_nl_params_data, _features_features_4_block_block_0_block_0_2_HardSigmoid_output_0_nl_params_data, 2, AI_STATIC_CONST)
AI_TENSOR_CHAIN_OBJ_DECLARE(
  _features_features_4_block_block_0_block_0_2_HardSigmoid_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_4_block_block_0_block_0_0_Conv_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_4_block_block_0_block_0_2_HardSigmoid_output_0_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  _features_features_4_block_block_0_block_0_2_HardSigmoid_output_0_layer, 25,
  NL_TYPE, 0x0, NULL,
  nl, forward_hard_sigmoid,
  &_features_features_4_block_block_0_block_0_2_HardSigmoid_output_0_chain,
  NULL, &_features_features_4_block_block_0_block_0_2_Mul_output_0_layer, AI_STATIC, 
  .nl_params = &_features_features_4_block_block_0_block_0_2_HardSigmoid_output_0_nl_params, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  _features_features_4_block_block_0_block_0_0_Conv_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_3_Add_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_4_block_block_0_block_0_0_Conv_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 3, &_features_features_4_block_block_0_block_0_0_Conv_output_0_weights, &_features_features_4_block_block_0_block_0_0_Conv_output_0_bias, NULL),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 2, &_features_features_4_block_block_0_block_0_0_Conv_output_0_scratch0, NULL)
)

AI_LAYER_OBJ_DECLARE(
  _features_features_4_block_block_0_block_0_0_Conv_output_0_layer, 24,
  CONV2D_TYPE, 0x0, NULL,
  conv2d, forward_conv2d_if32of32wf32,
  &_features_features_4_block_block_0_block_0_0_Conv_output_0_chain,
  NULL, &_features_features_4_block_block_0_block_0_2_HardSigmoid_output_0_layer, AI_STATIC, 
  .groups = 1, 
  .filter_stride = AI_SHAPE_2D_INIT(1, 1), 
  .dilation = AI_SHAPE_2D_INIT(1, 1), 
  .filter_pad = AI_SHAPE_INIT(4, 0, 0, 0, 0), 
  .in_ch_format = AI_LAYER_FORMAT_CHANNEL_LAST_VALID, 
  .out_ch_format = AI_LAYER_FORMAT_CHANNEL_LAST_VALID, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  _features_features_3_Add_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 2, &_features_features_3_block_block_2_block_2_0_Conv_output_0_output, &_features_features_2_block_block_2_block_2_0_Conv_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_3_Add_output_0_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  _features_features_3_Add_output_0_layer, 23,
  ELTWISE_TYPE, 0x0, NULL,
  eltwise, forward_eltwise,
  &_features_features_3_Add_output_0_chain,
  NULL, &_features_features_4_block_block_0_block_0_0_Conv_output_0_layer, AI_STATIC, 
  .operation = ai_sum_f32, 
  .buffer_operation = ai_sum_buffer_f32, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  _features_features_3_block_block_2_block_2_0_Conv_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_3_block_block_1_block_1_2_Relu_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_3_block_block_2_block_2_0_Conv_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 3, &_features_features_3_block_block_2_block_2_0_Conv_output_0_weights, &_features_features_3_block_block_2_block_2_0_Conv_output_0_bias, NULL),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 2, &_features_features_3_block_block_2_block_2_0_Conv_output_0_scratch0, NULL)
)

AI_LAYER_OBJ_DECLARE(
  _features_features_3_block_block_2_block_2_0_Conv_output_0_layer, 22,
  CONV2D_TYPE, 0x0, NULL,
  conv2d, forward_conv2d_if32of32wf32,
  &_features_features_3_block_block_2_block_2_0_Conv_output_0_chain,
  NULL, &_features_features_3_Add_output_0_layer, AI_STATIC, 
  .groups = 1, 
  .filter_stride = AI_SHAPE_2D_INIT(1, 1), 
  .dilation = AI_SHAPE_2D_INIT(1, 1), 
  .filter_pad = AI_SHAPE_INIT(4, 0, 0, 0, 0), 
  .in_ch_format = AI_LAYER_FORMAT_CHANNEL_LAST_VALID, 
  .out_ch_format = AI_LAYER_FORMAT_CHANNEL_LAST_VALID, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  _features_features_3_block_block_1_block_1_2_Relu_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_3_block_block_1_block_1_0_Conv_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_3_block_block_1_block_1_2_Relu_output_0_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  _features_features_3_block_block_1_block_1_2_Relu_output_0_layer, 21,
  NL_TYPE, 0x0, NULL,
  nl, forward_relu,
  &_features_features_3_block_block_1_block_1_2_Relu_output_0_chain,
  NULL, &_features_features_3_block_block_2_block_2_0_Conv_output_0_layer, AI_STATIC, 
  .nl_params = NULL, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  _features_features_3_block_block_1_block_1_0_Conv_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_3_block_block_0_block_0_2_Relu_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_3_block_block_1_block_1_0_Conv_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 3, &_features_features_3_block_block_1_block_1_0_Conv_output_0_weights, &_features_features_3_block_block_1_block_1_0_Conv_output_0_bias, NULL),
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  _features_features_3_block_block_1_block_1_0_Conv_output_0_layer, 20,
  CONV2D_TYPE, 0x0, NULL,
  conv2d, forward_dw_if32of32wf32,
  &_features_features_3_block_block_1_block_1_0_Conv_output_0_chain,
  NULL, &_features_features_3_block_block_1_block_1_2_Relu_output_0_layer, AI_STATIC, 
  .groups = 88, 
  .filter_stride = AI_SHAPE_2D_INIT(1, 1), 
  .dilation = AI_SHAPE_2D_INIT(1, 1), 
  .filter_pad = AI_SHAPE_INIT(4, 1, 1, 1, 1), 
  .in_ch_format = AI_LAYER_FORMAT_CHANNEL_LAST_SAME, 
  .out_ch_format = AI_LAYER_FORMAT_CHANNEL_LAST_VALID, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  _features_features_3_block_block_0_block_0_2_Relu_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_3_block_block_0_block_0_0_Conv_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_3_block_block_0_block_0_2_Relu_output_0_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  _features_features_3_block_block_0_block_0_2_Relu_output_0_layer, 19,
  NL_TYPE, 0x0, NULL,
  nl, forward_relu,
  &_features_features_3_block_block_0_block_0_2_Relu_output_0_chain,
  NULL, &_features_features_3_block_block_1_block_1_0_Conv_output_0_layer, AI_STATIC, 
  .nl_params = NULL, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  _features_features_3_block_block_0_block_0_0_Conv_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_2_block_block_2_block_2_0_Conv_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_3_block_block_0_block_0_0_Conv_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 3, &_features_features_3_block_block_0_block_0_0_Conv_output_0_weights, &_features_features_3_block_block_0_block_0_0_Conv_output_0_bias, NULL),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 2, &_features_features_3_block_block_0_block_0_0_Conv_output_0_scratch0, NULL)
)

AI_LAYER_OBJ_DECLARE(
  _features_features_3_block_block_0_block_0_0_Conv_output_0_layer, 18,
  CONV2D_TYPE, 0x0, NULL,
  conv2d, forward_conv2d_if32of32wf32,
  &_features_features_3_block_block_0_block_0_0_Conv_output_0_chain,
  NULL, &_features_features_3_block_block_0_block_0_2_Relu_output_0_layer, AI_STATIC, 
  .groups = 1, 
  .filter_stride = AI_SHAPE_2D_INIT(1, 1), 
  .dilation = AI_SHAPE_2D_INIT(1, 1), 
  .filter_pad = AI_SHAPE_INIT(4, 0, 0, 0, 0), 
  .in_ch_format = AI_LAYER_FORMAT_CHANNEL_LAST_VALID, 
  .out_ch_format = AI_LAYER_FORMAT_CHANNEL_LAST_VALID, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  _features_features_2_block_block_2_block_2_0_Conv_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_2_block_block_1_block_1_2_Relu_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_2_block_block_2_block_2_0_Conv_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 3, &_features_features_2_block_block_2_block_2_0_Conv_output_0_weights, &_features_features_2_block_block_2_block_2_0_Conv_output_0_bias, NULL),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 2, &_features_features_2_block_block_2_block_2_0_Conv_output_0_scratch0, NULL)
)

AI_LAYER_OBJ_DECLARE(
  _features_features_2_block_block_2_block_2_0_Conv_output_0_layer, 17,
  CONV2D_TYPE, 0x0, NULL,
  conv2d, forward_conv2d_if32of32wf32,
  &_features_features_2_block_block_2_block_2_0_Conv_output_0_chain,
  NULL, &_features_features_3_block_block_0_block_0_0_Conv_output_0_layer, AI_STATIC, 
  .groups = 1, 
  .filter_stride = AI_SHAPE_2D_INIT(1, 1), 
  .dilation = AI_SHAPE_2D_INIT(1, 1), 
  .filter_pad = AI_SHAPE_INIT(4, 0, 0, 0, 0), 
  .in_ch_format = AI_LAYER_FORMAT_CHANNEL_LAST_VALID, 
  .out_ch_format = AI_LAYER_FORMAT_CHANNEL_LAST_VALID, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  _features_features_2_block_block_1_block_1_2_Relu_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_2_block_block_1_block_1_0_Conv_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_2_block_block_1_block_1_2_Relu_output_0_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  _features_features_2_block_block_1_block_1_2_Relu_output_0_layer, 16,
  NL_TYPE, 0x0, NULL,
  nl, forward_relu,
  &_features_features_2_block_block_1_block_1_2_Relu_output_0_chain,
  NULL, &_features_features_2_block_block_2_block_2_0_Conv_output_0_layer, AI_STATIC, 
  .nl_params = NULL, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  _features_features_2_block_block_1_block_1_0_Conv_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_2_block_block_0_block_0_2_Relu_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_2_block_block_1_block_1_0_Conv_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 3, &_features_features_2_block_block_1_block_1_0_Conv_output_0_weights, &_features_features_2_block_block_1_block_1_0_Conv_output_0_bias, NULL),
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  _features_features_2_block_block_1_block_1_0_Conv_output_0_layer, 15,
  CONV2D_TYPE, 0x0, NULL,
  conv2d, forward_dw_if32of32wf32,
  &_features_features_2_block_block_1_block_1_0_Conv_output_0_chain,
  NULL, &_features_features_2_block_block_1_block_1_2_Relu_output_0_layer, AI_STATIC, 
  .groups = 72, 
  .filter_stride = AI_SHAPE_2D_INIT(2, 2), 
  .dilation = AI_SHAPE_2D_INIT(1, 1), 
  .filter_pad = AI_SHAPE_INIT(4, 1, 1, 1, 1), 
  .in_ch_format = AI_LAYER_FORMAT_CHANNEL_LAST_SAME, 
  .out_ch_format = AI_LAYER_FORMAT_CHANNEL_LAST_VALID, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  _features_features_2_block_block_0_block_0_2_Relu_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_2_block_block_0_block_0_0_Conv_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_2_block_block_0_block_0_2_Relu_output_0_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  _features_features_2_block_block_0_block_0_2_Relu_output_0_layer, 14,
  NL_TYPE, 0x0, NULL,
  nl, forward_relu,
  &_features_features_2_block_block_0_block_0_2_Relu_output_0_chain,
  NULL, &_features_features_2_block_block_1_block_1_0_Conv_output_0_layer, AI_STATIC, 
  .nl_params = NULL, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  _features_features_2_block_block_0_block_0_0_Conv_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_1_block_block_2_block_2_0_Conv_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_2_block_block_0_block_0_0_Conv_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 3, &_features_features_2_block_block_0_block_0_0_Conv_output_0_weights, &_features_features_2_block_block_0_block_0_0_Conv_output_0_bias, NULL),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 2, &_features_features_2_block_block_0_block_0_0_Conv_output_0_scratch0, NULL)
)

AI_LAYER_OBJ_DECLARE(
  _features_features_2_block_block_0_block_0_0_Conv_output_0_layer, 13,
  CONV2D_TYPE, 0x0, NULL,
  conv2d, forward_conv2d_if32of32wf32,
  &_features_features_2_block_block_0_block_0_0_Conv_output_0_chain,
  NULL, &_features_features_2_block_block_0_block_0_2_Relu_output_0_layer, AI_STATIC, 
  .groups = 1, 
  .filter_stride = AI_SHAPE_2D_INIT(1, 1), 
  .dilation = AI_SHAPE_2D_INIT(1, 1), 
  .filter_pad = AI_SHAPE_INIT(4, 0, 0, 0, 0), 
  .in_ch_format = AI_LAYER_FORMAT_CHANNEL_LAST_VALID, 
  .out_ch_format = AI_LAYER_FORMAT_CHANNEL_LAST_VALID, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  _features_features_1_block_block_2_block_2_0_Conv_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_1_block_block_1_Mul_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_1_block_block_2_block_2_0_Conv_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 3, &_features_features_1_block_block_2_block_2_0_Conv_output_0_weights, &_features_features_1_block_block_2_block_2_0_Conv_output_0_bias, NULL),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 2, &_features_features_1_block_block_2_block_2_0_Conv_output_0_scratch0, NULL)
)

AI_LAYER_OBJ_DECLARE(
  _features_features_1_block_block_2_block_2_0_Conv_output_0_layer, 12,
  CONV2D_TYPE, 0x0, NULL,
  conv2d, forward_conv2d_if32of32wf32,
  &_features_features_1_block_block_2_block_2_0_Conv_output_0_chain,
  NULL, &_features_features_2_block_block_0_block_0_0_Conv_output_0_layer, AI_STATIC, 
  .groups = 1, 
  .filter_stride = AI_SHAPE_2D_INIT(1, 1), 
  .dilation = AI_SHAPE_2D_INIT(1, 1), 
  .filter_pad = AI_SHAPE_INIT(4, 0, 0, 0, 0), 
  .in_ch_format = AI_LAYER_FORMAT_CHANNEL_LAST_VALID, 
  .out_ch_format = AI_LAYER_FORMAT_CHANNEL_LAST_VALID, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  _features_features_1_block_block_1_Mul_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 2, &_features_features_1_block_block_1_scale_activation_HardSigmoid_output_0_output, &_features_features_1_block_block_0_block_0_2_Relu_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_1_block_block_1_Mul_output_0_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  _features_features_1_block_block_1_Mul_output_0_layer, 11,
  ELTWISE_TYPE, 0x0, NULL,
  eltwise, forward_eltwise,
  &_features_features_1_block_block_1_Mul_output_0_chain,
  NULL, &_features_features_1_block_block_2_block_2_0_Conv_output_0_layer, AI_STATIC, 
  .operation = ai_mul_f32, 
  .buffer_operation = ai_mul_buffer_f32, 
)


AI_STATIC_CONST ai_float _features_features_1_block_block_1_scale_activation_HardSigmoid_output_0_nl_params_data[] = { 0.1666666716337204, 0.5 };
AI_ARRAY_OBJ_DECLARE(
    _features_features_1_block_block_1_scale_activation_HardSigmoid_output_0_nl_params, AI_ARRAY_FORMAT_FLOAT,
    _features_features_1_block_block_1_scale_activation_HardSigmoid_output_0_nl_params_data, _features_features_1_block_block_1_scale_activation_HardSigmoid_output_0_nl_params_data, 2, AI_STATIC_CONST)
AI_TENSOR_CHAIN_OBJ_DECLARE(
  _features_features_1_block_block_1_scale_activation_HardSigmoid_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_1_block_block_1_fc2_Conv_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_1_block_block_1_scale_activation_HardSigmoid_output_0_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  _features_features_1_block_block_1_scale_activation_HardSigmoid_output_0_layer, 10,
  NL_TYPE, 0x0, NULL,
  nl, forward_hard_sigmoid,
  &_features_features_1_block_block_1_scale_activation_HardSigmoid_output_0_chain,
  NULL, &_features_features_1_block_block_1_Mul_output_0_layer, AI_STATIC, 
  .nl_params = &_features_features_1_block_block_1_scale_activation_HardSigmoid_output_0_nl_params, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  _features_features_1_block_block_1_fc2_Conv_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_1_block_block_1_activation_Relu_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_1_block_block_1_fc2_Conv_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 3, &_features_features_1_block_block_1_fc2_Conv_output_0_weights, &_features_features_1_block_block_1_fc2_Conv_output_0_bias, NULL),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 2, &_features_features_1_block_block_1_fc2_Conv_output_0_scratch0, NULL)
)

AI_LAYER_OBJ_DECLARE(
  _features_features_1_block_block_1_fc2_Conv_output_0_layer, 9,
  CONV2D_TYPE, 0x0, NULL,
  conv2d, forward_conv2d_if32of32wf32,
  &_features_features_1_block_block_1_fc2_Conv_output_0_chain,
  NULL, &_features_features_1_block_block_1_scale_activation_HardSigmoid_output_0_layer, AI_STATIC, 
  .groups = 1, 
  .filter_stride = AI_SHAPE_2D_INIT(1, 1), 
  .dilation = AI_SHAPE_2D_INIT(1, 1), 
  .filter_pad = AI_SHAPE_INIT(4, 0, 0, 0, 0), 
  .in_ch_format = AI_LAYER_FORMAT_CHANNEL_LAST_VALID, 
  .out_ch_format = AI_LAYER_FORMAT_CHANNEL_LAST_VALID, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  _features_features_1_block_block_1_activation_Relu_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_1_block_block_1_fc1_Conv_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_1_block_block_1_activation_Relu_output_0_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  _features_features_1_block_block_1_activation_Relu_output_0_layer, 8,
  NL_TYPE, 0x0, NULL,
  nl, forward_relu,
  &_features_features_1_block_block_1_activation_Relu_output_0_chain,
  NULL, &_features_features_1_block_block_1_fc2_Conv_output_0_layer, AI_STATIC, 
  .nl_params = NULL, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  _features_features_1_block_block_1_fc1_Conv_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_1_block_block_1_avgpool_GlobalAveragePool_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_1_block_block_1_fc1_Conv_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 3, &_features_features_1_block_block_1_fc1_Conv_output_0_weights, &_features_features_1_block_block_1_fc1_Conv_output_0_bias, NULL),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 2, &_features_features_1_block_block_1_fc1_Conv_output_0_scratch0, NULL)
)

AI_LAYER_OBJ_DECLARE(
  _features_features_1_block_block_1_fc1_Conv_output_0_layer, 7,
  CONV2D_TYPE, 0x0, NULL,
  conv2d, forward_conv2d_if32of32wf32,
  &_features_features_1_block_block_1_fc1_Conv_output_0_chain,
  NULL, &_features_features_1_block_block_1_activation_Relu_output_0_layer, AI_STATIC, 
  .groups = 1, 
  .filter_stride = AI_SHAPE_2D_INIT(1, 1), 
  .dilation = AI_SHAPE_2D_INIT(1, 1), 
  .filter_pad = AI_SHAPE_INIT(4, 0, 0, 0, 0), 
  .in_ch_format = AI_LAYER_FORMAT_CHANNEL_LAST_VALID, 
  .out_ch_format = AI_LAYER_FORMAT_CHANNEL_LAST_VALID, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  _features_features_1_block_block_1_avgpool_GlobalAveragePool_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_1_block_block_0_block_0_2_Relu_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_1_block_block_1_avgpool_GlobalAveragePool_output_0_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  _features_features_1_block_block_1_avgpool_GlobalAveragePool_output_0_layer, 6,
  POOL_TYPE, 0x0, NULL,
  pool, forward_ap,
  &_features_features_1_block_block_1_avgpool_GlobalAveragePool_output_0_chain,
  NULL, &_features_features_1_block_block_1_fc1_Conv_output_0_layer, AI_STATIC, 
  .pool_size = AI_SHAPE_2D_INIT(172, 8), 
  .pool_stride = AI_SHAPE_2D_INIT(172, 8), 
  .pool_pad = AI_SHAPE_INIT(4, 0, 0, 0, 0), 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  _features_features_1_block_block_0_block_0_2_Relu_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_1_block_block_0_block_0_0_Conv_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_1_block_block_0_block_0_2_Relu_output_0_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  _features_features_1_block_block_0_block_0_2_Relu_output_0_layer, 5,
  NL_TYPE, 0x0, NULL,
  nl, forward_relu,
  &_features_features_1_block_block_0_block_0_2_Relu_output_0_chain,
  NULL, &_features_features_1_block_block_1_avgpool_GlobalAveragePool_output_0_layer, AI_STATIC, 
  .nl_params = NULL, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  _features_features_1_block_block_0_block_0_0_Conv_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_0_features_0_2_Mul_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_1_block_block_0_block_0_0_Conv_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 3, &_features_features_1_block_block_0_block_0_0_Conv_output_0_weights, &_features_features_1_block_block_0_block_0_0_Conv_output_0_bias, NULL),
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  _features_features_1_block_block_0_block_0_0_Conv_output_0_layer, 4,
  CONV2D_TYPE, 0x0, NULL,
  conv2d, forward_dw_if32of32wf32,
  &_features_features_1_block_block_0_block_0_0_Conv_output_0_chain,
  NULL, &_features_features_1_block_block_0_block_0_2_Relu_output_0_layer, AI_STATIC, 
  .groups = 16, 
  .filter_stride = AI_SHAPE_2D_INIT(2, 2), 
  .dilation = AI_SHAPE_2D_INIT(1, 1), 
  .filter_pad = AI_SHAPE_INIT(4, 1, 1, 1, 1), 
  .in_ch_format = AI_LAYER_FORMAT_CHANNEL_LAST_SAME, 
  .out_ch_format = AI_LAYER_FORMAT_CHANNEL_LAST_VALID, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  _features_features_0_features_0_2_Mul_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 2, &_features_features_0_features_0_0_Conv_output_0_output, &_features_features_0_features_0_2_HardSigmoid_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_0_features_0_2_Mul_output_0_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  _features_features_0_features_0_2_Mul_output_0_layer, 3,
  ELTWISE_TYPE, 0x0, NULL,
  eltwise, forward_eltwise,
  &_features_features_0_features_0_2_Mul_output_0_chain,
  NULL, &_features_features_1_block_block_0_block_0_0_Conv_output_0_layer, AI_STATIC, 
  .operation = ai_mul_f32, 
  .buffer_operation = ai_mul_buffer_f32, 
)


AI_STATIC_CONST ai_float _features_features_0_features_0_2_HardSigmoid_output_0_nl_params_data[] = { 0.1666666716337204, 0.5 };
AI_ARRAY_OBJ_DECLARE(
    _features_features_0_features_0_2_HardSigmoid_output_0_nl_params, AI_ARRAY_FORMAT_FLOAT,
    _features_features_0_features_0_2_HardSigmoid_output_0_nl_params_data, _features_features_0_features_0_2_HardSigmoid_output_0_nl_params_data, 2, AI_STATIC_CONST)
AI_TENSOR_CHAIN_OBJ_DECLARE(
  _features_features_0_features_0_2_HardSigmoid_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_0_features_0_0_Conv_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_0_features_0_2_HardSigmoid_output_0_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  _features_features_0_features_0_2_HardSigmoid_output_0_layer, 2,
  NL_TYPE, 0x0, NULL,
  nl, forward_hard_sigmoid,
  &_features_features_0_features_0_2_HardSigmoid_output_0_chain,
  NULL, &_features_features_0_features_0_2_Mul_output_0_layer, AI_STATIC, 
  .nl_params = &_features_features_0_features_0_2_HardSigmoid_output_0_nl_params, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  _features_features_0_features_0_0_Conv_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &input_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_features_features_0_features_0_0_Conv_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 3, &_features_features_0_features_0_0_Conv_output_0_weights, &_features_features_0_features_0_0_Conv_output_0_bias, NULL),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 2, &_features_features_0_features_0_0_Conv_output_0_scratch0, NULL)
)

AI_LAYER_OBJ_DECLARE(
  _features_features_0_features_0_0_Conv_output_0_layer, 1,
  CONV2D_TYPE, 0x0, NULL,
  conv2d, forward_conv2d_if32of32wf32,
  &_features_features_0_features_0_0_Conv_output_0_chain,
  NULL, &_features_features_0_features_0_2_HardSigmoid_output_0_layer, AI_STATIC, 
  .groups = 1, 
  .filter_stride = AI_SHAPE_2D_INIT(2, 2), 
  .dilation = AI_SHAPE_2D_INIT(1, 1), 
  .filter_pad = AI_SHAPE_INIT(4, 1, 1, 1, 1), 
  .in_ch_format = AI_LAYER_FORMAT_CHANNEL_LAST_SAME, 
  .out_ch_format = AI_LAYER_FORMAT_CHANNEL_LAST_VALID, 
)


#if (AI_TOOLS_API_VERSION < AI_TOOLS_API_VERSION_1_5)

AI_NETWORK_OBJ_DECLARE(
  AI_NET_OBJ_INSTANCE, AI_STATIC,
  AI_BUFFER_INIT(AI_FLAG_NONE,  AI_BUFFER_FORMAT_U8,
    AI_BUFFER_SHAPE_INIT(AI_SHAPE_BCWH, 4, 1, 4280700, 1, 1),
    4280700, NULL, NULL),
  AI_BUFFER_INIT(AI_FLAG_NONE,  AI_BUFFER_FORMAT_U8,
    AI_BUFFER_SHAPE_INIT(AI_SHAPE_BCWH, 4, 1, 790688, 1, 1),
    790688, NULL, NULL),
  AI_TENSOR_LIST_IO_OBJ_INIT(AI_FLAG_NONE, AI_FALLOWER1_IN_NUM, &input_output),
  AI_TENSOR_LIST_IO_OBJ_INIT(AI_FLAG_NONE, AI_FALLOWER1_OUT_NUM, &output_output),
  &_features_features_0_features_0_0_Conv_output_0_layer, 0xb2e07433, NULL)

#else

AI_NETWORK_OBJ_DECLARE(
  AI_NET_OBJ_INSTANCE, AI_STATIC,
  AI_BUFFER_ARRAY_OBJ_INIT_STATIC(
  	AI_FLAG_NONE, 1,
    AI_BUFFER_INIT(AI_FLAG_NONE,  AI_BUFFER_FORMAT_U8,
      AI_BUFFER_SHAPE_INIT(AI_SHAPE_BCWH, 4, 1, 4280700, 1, 1),
      4280700, NULL, NULL)
  ),
  AI_BUFFER_ARRAY_OBJ_INIT_STATIC(
  	AI_FLAG_NONE, 1,
    AI_BUFFER_INIT(AI_FLAG_NONE,  AI_BUFFER_FORMAT_U8,
      AI_BUFFER_SHAPE_INIT(AI_SHAPE_BCWH, 4, 1, 790688, 1, 1),
      790688, NULL, NULL)
  ),
  AI_TENSOR_LIST_IO_OBJ_INIT(AI_FLAG_NONE, AI_FALLOWER1_IN_NUM, &input_output),
  AI_TENSOR_LIST_IO_OBJ_INIT(AI_FLAG_NONE, AI_FALLOWER1_OUT_NUM, &output_output),
  &_features_features_0_features_0_0_Conv_output_0_layer, 0xb2e07433, NULL)

#endif	/*(AI_TOOLS_API_VERSION < AI_TOOLS_API_VERSION_1_5)*/



/******************************************************************************/
AI_DECLARE_STATIC
ai_bool fallower1_configure_activations(
  ai_network* net_ctx, const ai_network_params* params)
{
  AI_ASSERT(net_ctx)

  if (ai_platform_get_activations_map(g_fallower1_activations_map, 1, params)) {
    /* Updating activations (byte) offsets */
    
    _features_features_0_features_0_0_Conv_output_0_scratch0_array.data = AI_PTR(g_fallower1_activations_map[0] + 330204);
    _features_features_0_features_0_0_Conv_output_0_scratch0_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 330204);
    _features_features_0_features_0_0_Conv_output_0_output_array.data = AI_PTR(g_fallower1_activations_map[0] + 330240);
    _features_features_0_features_0_0_Conv_output_0_output_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 330240);
    _features_features_0_features_0_2_HardSigmoid_output_0_output_array.data = AI_PTR(g_fallower1_activations_map[0] + 0);
    _features_features_0_features_0_2_HardSigmoid_output_0_output_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 0);
    _features_features_0_features_0_2_Mul_output_0_output_array.data = AI_PTR(g_fallower1_activations_map[0] + 330240);
    _features_features_0_features_0_2_Mul_output_0_output_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 330240);
    _features_features_1_block_block_0_block_0_0_Conv_output_0_output_array.data = AI_PTR(g_fallower1_activations_map[0] + 0);
    _features_features_1_block_block_0_block_0_0_Conv_output_0_output_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 0);
    _features_features_1_block_block_0_block_0_2_Relu_output_0_output_array.data = AI_PTR(g_fallower1_activations_map[0] + 88064);
    _features_features_1_block_block_0_block_0_2_Relu_output_0_output_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 88064);
    _features_features_1_block_block_1_fc1_Conv_output_0_scratch0_array.data = AI_PTR(g_fallower1_activations_map[0] + 0);
    _features_features_1_block_block_1_fc1_Conv_output_0_scratch0_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 0);
    _features_features_1_block_block_1_fc2_Conv_output_0_scratch0_array.data = AI_PTR(g_fallower1_activations_map[0] + 0);
    _features_features_1_block_block_1_fc2_Conv_output_0_scratch0_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 0);
    _features_features_1_block_block_2_block_2_0_Conv_output_0_scratch0_array.data = AI_PTR(g_fallower1_activations_map[0] + 0);
    _features_features_1_block_block_2_block_2_0_Conv_output_0_scratch0_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 0);
    _features_features_2_block_block_0_block_0_0_Conv_output_0_scratch0_array.data = AI_PTR(g_fallower1_activations_map[0] + 0);
    _features_features_2_block_block_0_block_0_0_Conv_output_0_scratch0_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 0);
    _features_features_2_block_block_0_block_0_0_Conv_output_0_output_array.data = AI_PTR(g_fallower1_activations_map[0] + 264192);
    _features_features_2_block_block_0_block_0_0_Conv_output_0_output_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 264192);
    _features_features_2_block_block_0_block_0_2_Relu_output_0_output_array.data = AI_PTR(g_fallower1_activations_map[0] + 264192);
    _features_features_2_block_block_0_block_0_2_Relu_output_0_output_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 264192);
    _features_features_2_block_block_2_block_2_0_Conv_output_0_scratch0_array.data = AI_PTR(g_fallower1_activations_map[0] + 0);
    _features_features_2_block_block_2_block_2_0_Conv_output_0_scratch0_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 0);
    _features_features_3_block_block_0_block_0_0_Conv_output_0_scratch0_array.data = AI_PTR(g_fallower1_activations_map[0] + 0);
    _features_features_3_block_block_0_block_0_0_Conv_output_0_scratch0_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 0);
    _features_features_3_block_block_0_block_0_0_Conv_output_0_output_array.data = AI_PTR(g_fallower1_activations_map[0] + 96);
    _features_features_3_block_block_0_block_0_0_Conv_output_0_output_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 96);
    _features_features_3_block_block_0_block_0_2_Relu_output_0_output_array.data = AI_PTR(g_fallower1_activations_map[0] + 96);
    _features_features_3_block_block_0_block_0_2_Relu_output_0_output_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 96);
    _features_features_3_block_block_1_block_1_0_Conv_output_0_output_array.data = AI_PTR(g_fallower1_activations_map[0] + 121184);
    _features_features_3_block_block_1_block_1_0_Conv_output_0_output_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 121184);
    _features_features_3_block_block_1_block_1_2_Relu_output_0_output_array.data = AI_PTR(g_fallower1_activations_map[0] + 0);
    _features_features_3_block_block_1_block_1_2_Relu_output_0_output_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 0);
    _features_features_3_block_block_2_block_2_0_Conv_output_0_scratch0_array.data = AI_PTR(g_fallower1_activations_map[0] + 121088);
    _features_features_3_block_block_2_block_2_0_Conv_output_0_scratch0_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 121088);
    _features_features_4_block_block_0_block_0_0_Conv_output_0_scratch0_array.data = AI_PTR(g_fallower1_activations_map[0] + 660384);
    _features_features_4_block_block_0_block_0_0_Conv_output_0_scratch0_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 660384);
    _features_features_4_block_block_0_block_0_0_Conv_output_0_output_array.data = AI_PTR(g_fallower1_activations_map[0] + 0);
    _features_features_4_block_block_0_block_0_0_Conv_output_0_output_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 0);
    _features_features_4_block_block_0_block_0_2_HardSigmoid_output_0_output_array.data = AI_PTR(g_fallower1_activations_map[0] + 528384);
    _features_features_4_block_block_0_block_0_2_HardSigmoid_output_0_output_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 528384);
    _features_features_4_block_block_0_block_0_2_Mul_output_0_output_array.data = AI_PTR(g_fallower1_activations_map[0] + 132096);
    _features_features_4_block_block_0_block_0_2_Mul_output_0_output_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 132096);
    _features_features_4_block_block_2_fc1_Conv_output_0_scratch0_array.data = AI_PTR(g_fallower1_activations_map[0] + 0);
    _features_features_4_block_block_2_fc1_Conv_output_0_scratch0_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 0);
    _features_features_4_block_block_2_fc2_Conv_output_0_scratch0_array.data = AI_PTR(g_fallower1_activations_map[0] + 0);
    _features_features_4_block_block_2_fc2_Conv_output_0_scratch0_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 0);
    _features_features_4_block_block_3_block_3_0_Conv_output_0_scratch0_array.data = AI_PTR(g_fallower1_activations_map[0] + 0);
    _features_features_4_block_block_3_block_3_0_Conv_output_0_scratch0_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 0);
    _features_features_5_block_block_0_block_0_0_Conv_output_0_scratch0_array.data = AI_PTR(g_fallower1_activations_map[0] + 0);
    _features_features_5_block_block_0_block_0_0_Conv_output_0_scratch0_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 0);
    _features_features_5_block_block_0_block_0_2_HardSigmoid_output_0_output_array.data = AI_PTR(g_fallower1_activations_map[0] + 577920);
    _features_features_5_block_block_0_block_0_2_HardSigmoid_output_0_output_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 577920);
    _features_features_5_block_block_1_block_1_0_Conv_output_0_output_array.data = AI_PTR(g_fallower1_activations_map[0] + 0);
    _features_features_5_block_block_1_block_1_0_Conv_output_0_output_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 0);
    _features_features_5_block_block_2_fc1_Conv_output_0_scratch0_array.data = AI_PTR(g_fallower1_activations_map[0] + 0);
    _features_features_5_block_block_2_fc1_Conv_output_0_scratch0_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 0);
    _features_features_5_block_block_2_fc2_Conv_output_0_scratch0_array.data = AI_PTR(g_fallower1_activations_map[0] + 0);
    _features_features_5_block_block_2_fc2_Conv_output_0_scratch0_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 0);
    _features_features_5_block_block_3_block_3_0_Conv_output_0_scratch0_array.data = AI_PTR(g_fallower1_activations_map[0] + 0);
    _features_features_5_block_block_3_block_3_0_Conv_output_0_scratch0_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 0);
    _features_features_6_block_block_0_block_0_0_Conv_output_0_scratch0_array.data = AI_PTR(g_fallower1_activations_map[0] + 0);
    _features_features_6_block_block_0_block_0_0_Conv_output_0_scratch0_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 0);
    _features_features_6_block_block_0_block_0_2_HardSigmoid_output_0_output_array.data = AI_PTR(g_fallower1_activations_map[0] + 0);
    _features_features_6_block_block_0_block_0_2_HardSigmoid_output_0_output_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 0);
    _features_features_6_block_block_1_block_1_0_Conv_output_0_output_array.data = AI_PTR(g_fallower1_activations_map[0] + 0);
    _features_features_6_block_block_1_block_1_0_Conv_output_0_output_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 0);
    _features_features_6_block_block_2_fc1_Conv_output_0_scratch0_array.data = AI_PTR(g_fallower1_activations_map[0] + 0);
    _features_features_6_block_block_2_fc1_Conv_output_0_scratch0_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 0);
    _features_features_6_block_block_2_fc2_Conv_output_0_scratch0_array.data = AI_PTR(g_fallower1_activations_map[0] + 0);
    _features_features_6_block_block_2_fc2_Conv_output_0_scratch0_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 0);
    _features_features_6_block_block_3_block_3_0_Conv_output_0_scratch0_array.data = AI_PTR(g_fallower1_activations_map[0] + 0);
    _features_features_6_block_block_3_block_3_0_Conv_output_0_scratch0_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 0);
    _features_features_7_block_block_0_block_0_0_Conv_output_0_scratch0_array.data = AI_PTR(g_fallower1_activations_map[0] + 0);
    _features_features_7_block_block_0_block_0_0_Conv_output_0_scratch0_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 0);
    _features_features_7_block_block_2_fc1_Conv_output_0_scratch0_array.data = AI_PTR(g_fallower1_activations_map[0] + 0);
    _features_features_7_block_block_2_fc1_Conv_output_0_scratch0_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 0);
    _features_features_7_block_block_2_fc2_Conv_output_0_scratch0_array.data = AI_PTR(g_fallower1_activations_map[0] + 0);
    _features_features_7_block_block_2_fc2_Conv_output_0_scratch0_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 0);
    _features_features_7_block_block_3_block_3_0_Conv_output_0_scratch0_array.data = AI_PTR(g_fallower1_activations_map[0] + 0);
    _features_features_7_block_block_3_block_3_0_Conv_output_0_scratch0_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 0);
    _features_features_8_block_block_0_block_0_0_Conv_output_0_scratch0_array.data = AI_PTR(g_fallower1_activations_map[0] + 0);
    _features_features_8_block_block_0_block_0_0_Conv_output_0_scratch0_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 0);
    _features_features_8_block_block_2_fc1_Conv_output_0_scratch0_array.data = AI_PTR(g_fallower1_activations_map[0] + 0);
    _features_features_8_block_block_2_fc1_Conv_output_0_scratch0_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 0);
    _features_features_8_block_block_2_fc2_Conv_output_0_scratch0_array.data = AI_PTR(g_fallower1_activations_map[0] + 0);
    _features_features_8_block_block_2_fc2_Conv_output_0_scratch0_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 0);
    _features_features_8_block_block_3_block_3_0_Conv_output_0_scratch0_array.data = AI_PTR(g_fallower1_activations_map[0] + 659904);
    _features_features_8_block_block_3_block_3_0_Conv_output_0_scratch0_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 659904);
    _features_features_9_block_block_0_block_0_0_Conv_output_0_scratch0_array.data = AI_PTR(g_fallower1_activations_map[0] + 660288);
    _features_features_9_block_block_0_block_0_0_Conv_output_0_scratch0_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 660288);
    _features_features_9_block_block_0_block_0_2_HardSigmoid_output_0_output_array.data = AI_PTR(g_fallower1_activations_map[0] + 0);
    _features_features_9_block_block_0_block_0_2_HardSigmoid_output_0_output_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 0);
    _features_features_9_block_block_2_fc1_Conv_output_0_scratch0_array.data = AI_PTR(g_fallower1_activations_map[0] + 0);
    _features_features_9_block_block_2_fc1_Conv_output_0_scratch0_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 0);
    _features_features_9_block_block_2_fc2_Conv_output_0_scratch0_array.data = AI_PTR(g_fallower1_activations_map[0] + 0);
    _features_features_9_block_block_2_fc2_Conv_output_0_scratch0_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 0);
    _features_features_9_block_block_3_block_3_0_Conv_output_0_scratch0_array.data = AI_PTR(g_fallower1_activations_map[0] + 0);
    _features_features_9_block_block_3_block_3_0_Conv_output_0_scratch0_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 0);
    _features_features_10_block_block_0_block_0_0_Conv_output_0_scratch0_array.data = AI_PTR(g_fallower1_activations_map[0] + 0);
    _features_features_10_block_block_0_block_0_0_Conv_output_0_scratch0_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 0);
    _features_features_10_block_block_2_fc1_Conv_output_0_scratch0_array.data = AI_PTR(g_fallower1_activations_map[0] + 0);
    _features_features_10_block_block_2_fc1_Conv_output_0_scratch0_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 0);
    _features_features_10_block_block_2_fc2_Conv_output_0_scratch0_array.data = AI_PTR(g_fallower1_activations_map[0] + 0);
    _features_features_10_block_block_2_fc2_Conv_output_0_scratch0_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 0);
    _features_features_10_block_block_3_block_3_0_Conv_output_0_scratch0_array.data = AI_PTR(g_fallower1_activations_map[0] + 0);
    _features_features_10_block_block_3_block_3_0_Conv_output_0_scratch0_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 0);
    _features_features_11_block_block_0_block_0_0_Conv_output_0_scratch0_array.data = AI_PTR(g_fallower1_activations_map[0] + 0);
    _features_features_11_block_block_0_block_0_0_Conv_output_0_scratch0_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 0);
    _features_features_11_block_block_2_fc1_Conv_output_0_scratch0_array.data = AI_PTR(g_fallower1_activations_map[0] + 0);
    _features_features_11_block_block_2_fc1_Conv_output_0_scratch0_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 0);
    _features_features_11_block_block_2_fc2_Conv_output_0_scratch0_array.data = AI_PTR(g_fallower1_activations_map[0] + 0);
    _features_features_11_block_block_2_fc2_Conv_output_0_scratch0_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 0);
    _features_features_11_block_block_3_block_3_0_Conv_output_0_scratch0_array.data = AI_PTR(g_fallower1_activations_map[0] + 0);
    _features_features_11_block_block_3_block_3_0_Conv_output_0_scratch0_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 0);
    _features_features_12_features_12_0_Conv_output_0_scratch0_array.data = AI_PTR(g_fallower1_activations_map[0] + 0);
    _features_features_12_features_12_0_Conv_output_0_scratch0_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 0);
    input_output_array.data = AI_PTR(g_fallower1_activations_map[0] + 693632);
    input_output_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 693632);
    _features_features_1_block_block_1_avgpool_GlobalAveragePool_output_0_output_array.data = AI_PTR(g_fallower1_activations_map[0] + 693632);
    _features_features_1_block_block_1_avgpool_GlobalAveragePool_output_0_output_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 693632);
    _features_features_1_block_block_1_fc1_Conv_output_0_output_array.data = AI_PTR(g_fallower1_activations_map[0] + 693696);
    _features_features_1_block_block_1_fc1_Conv_output_0_output_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 693696);
    _features_features_1_block_block_1_activation_Relu_output_0_output_array.data = AI_PTR(g_fallower1_activations_map[0] + 693632);
    _features_features_1_block_block_1_activation_Relu_output_0_output_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 693632);
    _features_features_1_block_block_1_fc2_Conv_output_0_output_array.data = AI_PTR(g_fallower1_activations_map[0] + 693664);
    _features_features_1_block_block_1_fc2_Conv_output_0_output_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 693664);
    _features_features_1_block_block_1_scale_activation_HardSigmoid_output_0_output_array.data = AI_PTR(g_fallower1_activations_map[0] + 693728);
    _features_features_1_block_block_1_scale_activation_HardSigmoid_output_0_output_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 693728);
    _features_features_1_block_block_1_Mul_output_0_output_array.data = AI_PTR(g_fallower1_activations_map[0] + 693792);
    _features_features_1_block_block_1_Mul_output_0_output_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 693792);
    _features_features_1_block_block_2_block_2_0_Conv_output_0_output_array.data = AI_PTR(g_fallower1_activations_map[0] + 682784);
    _features_features_1_block_block_2_block_2_0_Conv_output_0_output_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 682784);
    _features_features_2_block_block_1_block_1_0_Conv_output_0_output_array.data = AI_PTR(g_fallower1_activations_map[0] + 682784);
    _features_features_2_block_block_1_block_1_0_Conv_output_0_output_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 682784);
    _features_features_2_block_block_1_block_1_2_Relu_output_0_output_array.data = AI_PTR(g_fallower1_activations_map[0] + 682784);
    _features_features_2_block_block_1_block_1_2_Relu_output_0_output_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 682784);
    _features_features_2_block_block_2_block_2_0_Conv_output_0_output_array.data = AI_PTR(g_fallower1_activations_map[0] + 674528);
    _features_features_2_block_block_2_block_2_0_Conv_output_0_output_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 674528);
    _features_features_3_block_block_2_block_2_0_Conv_output_0_output_array.data = AI_PTR(g_fallower1_activations_map[0] + 707552);
    _features_features_3_block_block_2_block_2_0_Conv_output_0_output_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 707552);
    _features_features_3_Add_output_0_output_array.data = AI_PTR(g_fallower1_activations_map[0] + 748832);
    _features_features_3_Add_output_0_output_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 748832);
    _features_features_4_block_block_1_block_1_0_Conv_output_0_output_array.data = AI_PTR(g_fallower1_activations_map[0] + 674528);
    _features_features_4_block_block_1_block_1_0_Conv_output_0_output_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 674528);
    _features_features_4_block_block_1_block_1_2_HardSigmoid_output_0_output_array.data = AI_PTR(g_fallower1_activations_map[0] + 707552);
    _features_features_4_block_block_1_block_1_2_HardSigmoid_output_0_output_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 707552);
    _features_features_4_block_block_1_block_1_2_Mul_output_0_output_array.data = AI_PTR(g_fallower1_activations_map[0] + 740576);
    _features_features_4_block_block_1_block_1_2_Mul_output_0_output_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 740576);
    _features_features_4_block_block_2_avgpool_GlobalAveragePool_output_0_output_array.data = AI_PTR(g_fallower1_activations_map[0] + 674528);
    _features_features_4_block_block_2_avgpool_GlobalAveragePool_output_0_output_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 674528);
    _features_features_4_block_block_2_fc1_Conv_output_0_output_array.data = AI_PTR(g_fallower1_activations_map[0] + 674912);
    _features_features_4_block_block_2_fc1_Conv_output_0_output_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 674912);
    _features_features_4_block_block_2_activation_Relu_output_0_output_array.data = AI_PTR(g_fallower1_activations_map[0] + 674528);
    _features_features_4_block_block_2_activation_Relu_output_0_output_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 674528);
    _features_features_4_block_block_2_fc2_Conv_output_0_output_array.data = AI_PTR(g_fallower1_activations_map[0] + 674624);
    _features_features_4_block_block_2_fc2_Conv_output_0_output_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 674624);
    _features_features_4_block_block_2_scale_activation_HardSigmoid_output_0_output_array.data = AI_PTR(g_fallower1_activations_map[0] + 675008);
    _features_features_4_block_block_2_scale_activation_HardSigmoid_output_0_output_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 675008);
    _features_features_4_block_block_2_Mul_output_0_output_array.data = AI_PTR(g_fallower1_activations_map[0] + 675392);
    _features_features_4_block_block_2_Mul_output_0_output_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 675392);
    _features_features_4_block_block_3_block_3_0_Conv_output_0_output_array.data = AI_PTR(g_fallower1_activations_map[0] + 768096);
    _features_features_4_block_block_3_block_3_0_Conv_output_0_output_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 768096);
    _features_features_5_block_block_0_block_0_0_Conv_output_0_output_array.data = AI_PTR(g_fallower1_activations_map[0] + 674528);
    _features_features_5_block_block_0_block_0_0_Conv_output_0_output_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 674528);
    _features_features_5_block_block_0_block_0_2_Mul_output_0_output_array.data = AI_PTR(g_fallower1_activations_map[0] + 674528);
    _features_features_5_block_block_0_block_0_2_Mul_output_0_output_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 674528);
    _features_features_5_block_block_1_block_1_2_HardSigmoid_output_0_output_array.data = AI_PTR(g_fallower1_activations_map[0] + 674528);
    _features_features_5_block_block_1_block_1_2_HardSigmoid_output_0_output_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 674528);
    _features_features_5_block_block_1_block_1_2_Mul_output_0_output_array.data = AI_PTR(g_fallower1_activations_map[0] + 674528);
    _features_features_5_block_block_1_block_1_2_Mul_output_0_output_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 674528);
    _features_features_5_block_block_2_avgpool_GlobalAveragePool_output_0_output_array.data = AI_PTR(g_fallower1_activations_map[0] + 757088);
    _features_features_5_block_block_2_avgpool_GlobalAveragePool_output_0_output_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 757088);
    _features_features_5_block_block_2_fc1_Conv_output_0_output_array.data = AI_PTR(g_fallower1_activations_map[0] + 758048);
    _features_features_5_block_block_2_fc1_Conv_output_0_output_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 758048);
    _features_features_5_block_block_2_activation_Relu_output_0_output_array.data = AI_PTR(g_fallower1_activations_map[0] + 757088);
    _features_features_5_block_block_2_activation_Relu_output_0_output_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 757088);
    _features_features_5_block_block_2_fc2_Conv_output_0_output_array.data = AI_PTR(g_fallower1_activations_map[0] + 757344);
    _features_features_5_block_block_2_fc2_Conv_output_0_output_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 757344);
    _features_features_5_block_block_2_scale_activation_HardSigmoid_output_0_output_array.data = AI_PTR(g_fallower1_activations_map[0] + 758304);
    _features_features_5_block_block_2_scale_activation_HardSigmoid_output_0_output_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 758304);
    _features_features_5_block_block_2_Mul_output_0_output_array.data = AI_PTR(g_fallower1_activations_map[0] + 674528);
    _features_features_5_block_block_2_Mul_output_0_output_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 674528);
    _features_features_5_block_block_3_block_3_0_Conv_output_0_output_array.data = AI_PTR(g_fallower1_activations_map[0] + 667648);
    _features_features_5_block_block_3_block_3_0_Conv_output_0_output_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 667648);
    _features_features_5_Add_output_0_output_array.data = AI_PTR(g_fallower1_activations_map[0] + 667648);
    _features_features_5_Add_output_0_output_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 667648);
    _features_features_6_block_block_0_block_0_0_Conv_output_0_output_array.data = AI_PTR(g_fallower1_activations_map[0] + 681408);
    _features_features_6_block_block_0_block_0_0_Conv_output_0_output_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 681408);
    _features_features_6_block_block_0_block_0_2_Mul_output_0_output_array.data = AI_PTR(g_fallower1_activations_map[0] + 681408);
    _features_features_6_block_block_0_block_0_2_Mul_output_0_output_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 681408);
    _features_features_6_block_block_1_block_1_2_HardSigmoid_output_0_output_array.data = AI_PTR(g_fallower1_activations_map[0] + 681408);
    _features_features_6_block_block_1_block_1_2_HardSigmoid_output_0_output_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 681408);
    _features_features_6_block_block_1_block_1_2_Mul_output_0_output_array.data = AI_PTR(g_fallower1_activations_map[0] + 681408);
    _features_features_6_block_block_1_block_1_2_Mul_output_0_output_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 681408);
    _features_features_6_block_block_2_avgpool_GlobalAveragePool_output_0_output_array.data = AI_PTR(g_fallower1_activations_map[0] + 763968);
    _features_features_6_block_block_2_avgpool_GlobalAveragePool_output_0_output_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 763968);
    _features_features_6_block_block_2_fc1_Conv_output_0_output_array.data = AI_PTR(g_fallower1_activations_map[0] + 764928);
    _features_features_6_block_block_2_fc1_Conv_output_0_output_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 764928);
    _features_features_6_block_block_2_activation_Relu_output_0_output_array.data = AI_PTR(g_fallower1_activations_map[0] + 763968);
    _features_features_6_block_block_2_activation_Relu_output_0_output_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 763968);
    _features_features_6_block_block_2_fc2_Conv_output_0_output_array.data = AI_PTR(g_fallower1_activations_map[0] + 764224);
    _features_features_6_block_block_2_fc2_Conv_output_0_output_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 764224);
    _features_features_6_block_block_2_scale_activation_HardSigmoid_output_0_output_array.data = AI_PTR(g_fallower1_activations_map[0] + 765184);
    _features_features_6_block_block_2_scale_activation_HardSigmoid_output_0_output_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 765184);
    _features_features_6_block_block_2_Mul_output_0_output_array.data = AI_PTR(g_fallower1_activations_map[0] + 681408);
    _features_features_6_block_block_2_Mul_output_0_output_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 681408);
    _features_features_6_block_block_3_block_3_0_Conv_output_0_output_array.data = AI_PTR(g_fallower1_activations_map[0] + 763968);
    _features_features_6_block_block_3_block_3_0_Conv_output_0_output_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 763968);
    _features_features_6_Add_output_0_output_array.data = AI_PTR(g_fallower1_activations_map[0] + 681408);
    _features_features_6_Add_output_0_output_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 681408);
    _features_features_7_block_block_0_block_0_0_Conv_output_0_output_array.data = AI_PTR(g_fallower1_activations_map[0] + 695168);
    _features_features_7_block_block_0_block_0_0_Conv_output_0_output_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 695168);
    _features_features_7_block_block_0_block_0_2_HardSigmoid_output_0_output_array.data = AI_PTR(g_fallower1_activations_map[0] + 736448);
    _features_features_7_block_block_0_block_0_2_HardSigmoid_output_0_output_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 736448);
    _features_features_7_block_block_0_block_0_2_Mul_output_0_output_array.data = AI_PTR(g_fallower1_activations_map[0] + 695168);
    _features_features_7_block_block_0_block_0_2_Mul_output_0_output_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 695168);
    _features_features_7_block_block_1_block_1_0_Conv_output_0_output_array.data = AI_PTR(g_fallower1_activations_map[0] + 736448);
    _features_features_7_block_block_1_block_1_0_Conv_output_0_output_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 736448);
    _features_features_7_block_block_1_block_1_2_HardSigmoid_output_0_output_array.data = AI_PTR(g_fallower1_activations_map[0] + 667648);
    _features_features_7_block_block_1_block_1_2_HardSigmoid_output_0_output_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 667648);
    _features_features_7_block_block_1_block_1_2_Mul_output_0_output_array.data = AI_PTR(g_fallower1_activations_map[0] + 736448);
    _features_features_7_block_block_1_block_1_2_Mul_output_0_output_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 736448);
    _features_features_7_block_block_2_avgpool_GlobalAveragePool_output_0_output_array.data = AI_PTR(g_fallower1_activations_map[0] + 667648);
    _features_features_7_block_block_2_avgpool_GlobalAveragePool_output_0_output_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 667648);
    _features_features_7_block_block_2_fc1_Conv_output_0_output_array.data = AI_PTR(g_fallower1_activations_map[0] + 668128);
    _features_features_7_block_block_2_fc1_Conv_output_0_output_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 668128);
    _features_features_7_block_block_2_activation_Relu_output_0_output_array.data = AI_PTR(g_fallower1_activations_map[0] + 667648);
    _features_features_7_block_block_2_activation_Relu_output_0_output_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 667648);
    _features_features_7_block_block_2_fc2_Conv_output_0_output_array.data = AI_PTR(g_fallower1_activations_map[0] + 667776);
    _features_features_7_block_block_2_fc2_Conv_output_0_output_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 667776);
    _features_features_7_block_block_2_scale_activation_HardSigmoid_output_0_output_array.data = AI_PTR(g_fallower1_activations_map[0] + 668256);
    _features_features_7_block_block_2_scale_activation_HardSigmoid_output_0_output_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 668256);
    _features_features_7_block_block_2_Mul_output_0_output_array.data = AI_PTR(g_fallower1_activations_map[0] + 668736);
    _features_features_7_block_block_2_Mul_output_0_output_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 668736);
    _features_features_7_block_block_3_block_3_0_Conv_output_0_output_array.data = AI_PTR(g_fallower1_activations_map[0] + 710016);
    _features_features_7_block_block_3_block_3_0_Conv_output_0_output_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 710016);
    _features_features_8_block_block_0_block_0_0_Conv_output_0_output_array.data = AI_PTR(g_fallower1_activations_map[0] + 726528);
    _features_features_8_block_block_0_block_0_0_Conv_output_0_output_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 726528);
    _features_features_8_block_block_0_block_0_2_HardSigmoid_output_0_output_array.data = AI_PTR(g_fallower1_activations_map[0] + 660480);
    _features_features_8_block_block_0_block_0_2_HardSigmoid_output_0_output_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 660480);
    _features_features_8_block_block_0_block_0_2_Mul_output_0_output_array.data = AI_PTR(g_fallower1_activations_map[0] + 726528);
    _features_features_8_block_block_0_block_0_2_Mul_output_0_output_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 726528);
    _features_features_8_block_block_1_block_1_0_Conv_output_0_output_array.data = AI_PTR(g_fallower1_activations_map[0] + 660480);
    _features_features_8_block_block_1_block_1_0_Conv_output_0_output_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 660480);
    _features_features_8_block_block_1_block_1_2_HardSigmoid_output_0_output_array.data = AI_PTR(g_fallower1_activations_map[0] + 726528);
    _features_features_8_block_block_1_block_1_2_HardSigmoid_output_0_output_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 726528);
    _features_features_8_block_block_1_block_1_2_Mul_output_0_output_array.data = AI_PTR(g_fallower1_activations_map[0] + 660480);
    _features_features_8_block_block_1_block_1_2_Mul_output_0_output_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 660480);
    _features_features_8_block_block_2_avgpool_GlobalAveragePool_output_0_output_array.data = AI_PTR(g_fallower1_activations_map[0] + 726528);
    _features_features_8_block_block_2_avgpool_GlobalAveragePool_output_0_output_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 726528);
    _features_features_8_block_block_2_fc1_Conv_output_0_output_array.data = AI_PTR(g_fallower1_activations_map[0] + 727104);
    _features_features_8_block_block_2_fc1_Conv_output_0_output_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 727104);
    _features_features_8_block_block_2_activation_Relu_output_0_output_array.data = AI_PTR(g_fallower1_activations_map[0] + 726528);
    _features_features_8_block_block_2_activation_Relu_output_0_output_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 726528);
    _features_features_8_block_block_2_fc2_Conv_output_0_output_array.data = AI_PTR(g_fallower1_activations_map[0] + 726688);
    _features_features_8_block_block_2_fc2_Conv_output_0_output_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 726688);
    _features_features_8_block_block_2_scale_activation_HardSigmoid_output_0_output_array.data = AI_PTR(g_fallower1_activations_map[0] + 727264);
    _features_features_8_block_block_2_scale_activation_HardSigmoid_output_0_output_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 727264);
    _features_features_8_block_block_2_Mul_output_0_output_array.data = AI_PTR(g_fallower1_activations_map[0] + 660480);
    _features_features_8_block_block_2_Mul_output_0_output_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 660480);
    _features_features_8_block_block_3_block_3_0_Conv_output_0_output_array.data = AI_PTR(g_fallower1_activations_map[0] + 765344);
    _features_features_8_block_block_3_block_3_0_Conv_output_0_output_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 765344);
    _features_features_8_Add_output_0_output_array.data = AI_PTR(g_fallower1_activations_map[0] + 765344);
    _features_features_8_Add_output_0_output_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 765344);
    _features_features_9_block_block_0_block_0_0_Conv_output_0_output_array.data = AI_PTR(g_fallower1_activations_map[0] + 666272);
    _features_features_9_block_block_0_block_0_0_Conv_output_0_output_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 666272);
    _features_features_9_block_block_0_block_0_2_Mul_output_0_output_array.data = AI_PTR(g_fallower1_activations_map[0] + 666272);
    _features_features_9_block_block_0_block_0_2_Mul_output_0_output_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 666272);
    _features_features_9_block_block_1_block_1_0_Conv_output_0_output_array.data = AI_PTR(g_fallower1_activations_map[0] + 765344);
    _features_features_9_block_block_1_block_1_0_Conv_output_0_output_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 765344);
    _features_features_9_block_block_1_block_1_2_HardSigmoid_output_0_output_array.data = AI_PTR(g_fallower1_activations_map[0] + 660480);
    _features_features_9_block_block_1_block_1_2_HardSigmoid_output_0_output_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 660480);
    _features_features_9_block_block_1_block_1_2_Mul_output_0_output_array.data = AI_PTR(g_fallower1_activations_map[0] + 685824);
    _features_features_9_block_block_1_block_1_2_Mul_output_0_output_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 685824);
    _features_features_9_block_block_2_avgpool_GlobalAveragePool_output_0_output_array.data = AI_PTR(g_fallower1_activations_map[0] + 660480);
    _features_features_9_block_block_2_avgpool_GlobalAveragePool_output_0_output_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 660480);
    _features_features_9_block_block_2_fc1_Conv_output_0_output_array.data = AI_PTR(g_fallower1_activations_map[0] + 661632);
    _features_features_9_block_block_2_fc1_Conv_output_0_output_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 661632);
    _features_features_9_block_block_2_activation_Relu_output_0_output_array.data = AI_PTR(g_fallower1_activations_map[0] + 660480);
    _features_features_9_block_block_2_activation_Relu_output_0_output_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 660480);
    _features_features_9_block_block_2_fc2_Conv_output_0_output_array.data = AI_PTR(g_fallower1_activations_map[0] + 660768);
    _features_features_9_block_block_2_fc2_Conv_output_0_output_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 660768);
    _features_features_9_block_block_2_scale_activation_HardSigmoid_output_0_output_array.data = AI_PTR(g_fallower1_activations_map[0] + 661920);
    _features_features_9_block_block_2_scale_activation_HardSigmoid_output_0_output_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 661920);
    _features_features_9_block_block_2_Mul_output_0_output_array.data = AI_PTR(g_fallower1_activations_map[0] + 711168);
    _features_features_9_block_block_2_Mul_output_0_output_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 711168);
    _features_features_9_block_block_3_block_3_0_Conv_output_0_output_array.data = AI_PTR(g_fallower1_activations_map[0] + 660480);
    _features_features_9_block_block_3_block_3_0_Conv_output_0_output_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 660480);
    _features_features_10_block_block_0_block_0_0_Conv_output_0_output_array.data = AI_PTR(g_fallower1_activations_map[0] + 668928);
    _features_features_10_block_block_0_block_0_0_Conv_output_0_output_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 668928);
    _features_features_10_block_block_0_block_0_2_HardSigmoid_output_0_output_array.data = AI_PTR(g_fallower1_activations_map[0] + 719616);
    _features_features_10_block_block_0_block_0_2_HardSigmoid_output_0_output_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 719616);
    _features_features_10_block_block_0_block_0_2_Mul_output_0_output_array.data = AI_PTR(g_fallower1_activations_map[0] + 668928);
    _features_features_10_block_block_0_block_0_2_Mul_output_0_output_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 668928);
    _features_features_10_block_block_1_block_1_0_Conv_output_0_output_array.data = AI_PTR(g_fallower1_activations_map[0] + 719616);
    _features_features_10_block_block_1_block_1_0_Conv_output_0_output_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 719616);
    _features_features_10_block_block_1_block_1_2_HardSigmoid_output_0_output_array.data = AI_PTR(g_fallower1_activations_map[0] + 668928);
    _features_features_10_block_block_1_block_1_2_HardSigmoid_output_0_output_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 668928);
    _features_features_10_block_block_1_block_1_2_Mul_output_0_output_array.data = AI_PTR(g_fallower1_activations_map[0] + 719616);
    _features_features_10_block_block_1_block_1_2_Mul_output_0_output_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 719616);
    _features_features_10_block_block_2_avgpool_GlobalAveragePool_output_0_output_array.data = AI_PTR(g_fallower1_activations_map[0] + 668928);
    _features_features_10_block_block_2_avgpool_GlobalAveragePool_output_0_output_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 668928);
    _features_features_10_block_block_2_fc1_Conv_output_0_output_array.data = AI_PTR(g_fallower1_activations_map[0] + 671232);
    _features_features_10_block_block_2_fc1_Conv_output_0_output_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 671232);
    _features_features_10_block_block_2_activation_Relu_output_0_output_array.data = AI_PTR(g_fallower1_activations_map[0] + 668928);
    _features_features_10_block_block_2_activation_Relu_output_0_output_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 668928);
    _features_features_10_block_block_2_fc2_Conv_output_0_output_array.data = AI_PTR(g_fallower1_activations_map[0] + 669504);
    _features_features_10_block_block_2_fc2_Conv_output_0_output_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 669504);
    _features_features_10_block_block_2_scale_activation_HardSigmoid_output_0_output_array.data = AI_PTR(g_fallower1_activations_map[0] + 671808);
    _features_features_10_block_block_2_scale_activation_HardSigmoid_output_0_output_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 671808);
    _features_features_10_block_block_2_Mul_output_0_output_array.data = AI_PTR(g_fallower1_activations_map[0] + 719616);
    _features_features_10_block_block_2_Mul_output_0_output_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 719616);
    _features_features_10_block_block_3_block_3_0_Conv_output_0_output_array.data = AI_PTR(g_fallower1_activations_map[0] + 668928);
    _features_features_10_block_block_3_block_3_0_Conv_output_0_output_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 668928);
    _features_features_10_Add_output_0_output_array.data = AI_PTR(g_fallower1_activations_map[0] + 677376);
    _features_features_10_Add_output_0_output_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 677376);
    _features_features_11_block_block_0_block_0_0_Conv_output_0_output_array.data = AI_PTR(g_fallower1_activations_map[0] + 685824);
    _features_features_11_block_block_0_block_0_0_Conv_output_0_output_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 685824);
    _features_features_11_block_block_0_block_0_2_HardSigmoid_output_0_output_array.data = AI_PTR(g_fallower1_activations_map[0] + 736512);
    _features_features_11_block_block_0_block_0_2_HardSigmoid_output_0_output_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 736512);
    _features_features_11_block_block_0_block_0_2_Mul_output_0_output_array.data = AI_PTR(g_fallower1_activations_map[0] + 685824);
    _features_features_11_block_block_0_block_0_2_Mul_output_0_output_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 685824);
    _features_features_11_block_block_1_block_1_0_Conv_output_0_output_array.data = AI_PTR(g_fallower1_activations_map[0] + 736512);
    _features_features_11_block_block_1_block_1_0_Conv_output_0_output_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 736512);
    _features_features_11_block_block_1_block_1_2_HardSigmoid_output_0_output_array.data = AI_PTR(g_fallower1_activations_map[0] + 685824);
    _features_features_11_block_block_1_block_1_2_HardSigmoid_output_0_output_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 685824);
    _features_features_11_block_block_1_block_1_2_Mul_output_0_output_array.data = AI_PTR(g_fallower1_activations_map[0] + 736512);
    _features_features_11_block_block_1_block_1_2_Mul_output_0_output_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 736512);
    _features_features_11_block_block_2_avgpool_GlobalAveragePool_output_0_output_array.data = AI_PTR(g_fallower1_activations_map[0] + 660480);
    _features_features_11_block_block_2_avgpool_GlobalAveragePool_output_0_output_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 660480);
    _features_features_11_block_block_2_fc1_Conv_output_0_output_array.data = AI_PTR(g_fallower1_activations_map[0] + 662784);
    _features_features_11_block_block_2_fc1_Conv_output_0_output_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 662784);
    _features_features_11_block_block_2_activation_Relu_output_0_output_array.data = AI_PTR(g_fallower1_activations_map[0] + 660480);
    _features_features_11_block_block_2_activation_Relu_output_0_output_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 660480);
    _features_features_11_block_block_2_fc2_Conv_output_0_output_array.data = AI_PTR(g_fallower1_activations_map[0] + 661056);
    _features_features_11_block_block_2_fc2_Conv_output_0_output_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 661056);
    _features_features_11_block_block_2_scale_activation_HardSigmoid_output_0_output_array.data = AI_PTR(g_fallower1_activations_map[0] + 663360);
    _features_features_11_block_block_2_scale_activation_HardSigmoid_output_0_output_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 663360);
    _features_features_11_block_block_2_Mul_output_0_output_array.data = AI_PTR(g_fallower1_activations_map[0] + 685824);
    _features_features_11_block_block_2_Mul_output_0_output_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 685824);
    _features_features_11_block_block_3_block_3_0_Conv_output_0_output_array.data = AI_PTR(g_fallower1_activations_map[0] + 660480);
    _features_features_11_block_block_3_block_3_0_Conv_output_0_output_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 660480);
    _features_features_11_Add_output_0_output_array.data = AI_PTR(g_fallower1_activations_map[0] + 668928);
    _features_features_11_Add_output_0_output_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 668928);
    _features_features_12_features_12_0_Conv_output_0_output_array.data = AI_PTR(g_fallower1_activations_map[0] + 677376);
    _features_features_12_features_12_0_Conv_output_0_output_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 677376);
    _features_features_12_features_12_2_HardSigmoid_output_0_output_array.data = AI_PTR(g_fallower1_activations_map[0] + 728064);
    _features_features_12_features_12_2_HardSigmoid_output_0_output_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 728064);
    _features_features_12_features_12_2_Mul_output_0_output_array.data = AI_PTR(g_fallower1_activations_map[0] + 677376);
    _features_features_12_features_12_2_Mul_output_0_output_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 677376);
    _avgpool_GlobalAveragePool_output_0_output_array.data = AI_PTR(g_fallower1_activations_map[0] + 660480);
    _avgpool_GlobalAveragePool_output_0_output_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 660480);
    _classifier_classifier_0_Gemm_output_0_output_array.data = AI_PTR(g_fallower1_activations_map[0] + 662784);
    _classifier_classifier_0_Gemm_output_0_output_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 662784);
    _classifier_classifier_1_Relu_output_0_output_array.data = AI_PTR(g_fallower1_activations_map[0] + 660480);
    _classifier_classifier_1_Relu_output_0_output_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 660480);
    output_output_array.data = AI_PTR(g_fallower1_activations_map[0] + 661504);
    output_output_array.data_start = AI_PTR(g_fallower1_activations_map[0] + 661504);
    return true;
  }
  AI_ERROR_TRAP(net_ctx, INIT_FAILED, NETWORK_ACTIVATIONS);
  return false;
}




/******************************************************************************/
AI_DECLARE_STATIC
ai_bool fallower1_configure_weights(
  ai_network* net_ctx, const ai_network_params* params)
{
  AI_ASSERT(net_ctx)

  if (ai_platform_get_weights_map(g_fallower1_weights_map, 1, params)) {
    /* Updating weights (byte) offsets */
    
    _features_features_0_features_0_0_Conv_output_0_weights_array.format |= AI_FMT_FLAG_CONST;
    _features_features_0_features_0_0_Conv_output_0_weights_array.data = AI_PTR(g_fallower1_weights_map[0] + 0);
    _features_features_0_features_0_0_Conv_output_0_weights_array.data_start = AI_PTR(g_fallower1_weights_map[0] + 0);
    _features_features_0_features_0_0_Conv_output_0_bias_array.format |= AI_FMT_FLAG_CONST;
    _features_features_0_features_0_0_Conv_output_0_bias_array.data = AI_PTR(g_fallower1_weights_map[0] + 576);
    _features_features_0_features_0_0_Conv_output_0_bias_array.data_start = AI_PTR(g_fallower1_weights_map[0] + 576);
    _features_features_1_block_block_0_block_0_0_Conv_output_0_weights_array.format |= AI_FMT_FLAG_CONST;
    _features_features_1_block_block_0_block_0_0_Conv_output_0_weights_array.data = AI_PTR(g_fallower1_weights_map[0] + 640);
    _features_features_1_block_block_0_block_0_0_Conv_output_0_weights_array.data_start = AI_PTR(g_fallower1_weights_map[0] + 640);
    _features_features_1_block_block_0_block_0_0_Conv_output_0_bias_array.format |= AI_FMT_FLAG_CONST;
    _features_features_1_block_block_0_block_0_0_Conv_output_0_bias_array.data = AI_PTR(g_fallower1_weights_map[0] + 1216);
    _features_features_1_block_block_0_block_0_0_Conv_output_0_bias_array.data_start = AI_PTR(g_fallower1_weights_map[0] + 1216);
    _features_features_1_block_block_1_fc1_Conv_output_0_weights_array.format |= AI_FMT_FLAG_CONST;
    _features_features_1_block_block_1_fc1_Conv_output_0_weights_array.data = AI_PTR(g_fallower1_weights_map[0] + 1280);
    _features_features_1_block_block_1_fc1_Conv_output_0_weights_array.data_start = AI_PTR(g_fallower1_weights_map[0] + 1280);
    _features_features_1_block_block_1_fc1_Conv_output_0_bias_array.format |= AI_FMT_FLAG_CONST;
    _features_features_1_block_block_1_fc1_Conv_output_0_bias_array.data = AI_PTR(g_fallower1_weights_map[0] + 1792);
    _features_features_1_block_block_1_fc1_Conv_output_0_bias_array.data_start = AI_PTR(g_fallower1_weights_map[0] + 1792);
    _features_features_1_block_block_1_fc2_Conv_output_0_weights_array.format |= AI_FMT_FLAG_CONST;
    _features_features_1_block_block_1_fc2_Conv_output_0_weights_array.data = AI_PTR(g_fallower1_weights_map[0] + 1824);
    _features_features_1_block_block_1_fc2_Conv_output_0_weights_array.data_start = AI_PTR(g_fallower1_weights_map[0] + 1824);
    _features_features_1_block_block_1_fc2_Conv_output_0_bias_array.format |= AI_FMT_FLAG_CONST;
    _features_features_1_block_block_1_fc2_Conv_output_0_bias_array.data = AI_PTR(g_fallower1_weights_map[0] + 2336);
    _features_features_1_block_block_1_fc2_Conv_output_0_bias_array.data_start = AI_PTR(g_fallower1_weights_map[0] + 2336);
    _features_features_1_block_block_2_block_2_0_Conv_output_0_weights_array.format |= AI_FMT_FLAG_CONST;
    _features_features_1_block_block_2_block_2_0_Conv_output_0_weights_array.data = AI_PTR(g_fallower1_weights_map[0] + 2400);
    _features_features_1_block_block_2_block_2_0_Conv_output_0_weights_array.data_start = AI_PTR(g_fallower1_weights_map[0] + 2400);
    _features_features_1_block_block_2_block_2_0_Conv_output_0_bias_array.format |= AI_FMT_FLAG_CONST;
    _features_features_1_block_block_2_block_2_0_Conv_output_0_bias_array.data = AI_PTR(g_fallower1_weights_map[0] + 3424);
    _features_features_1_block_block_2_block_2_0_Conv_output_0_bias_array.data_start = AI_PTR(g_fallower1_weights_map[0] + 3424);
    _features_features_2_block_block_0_block_0_0_Conv_output_0_weights_array.format |= AI_FMT_FLAG_CONST;
    _features_features_2_block_block_0_block_0_0_Conv_output_0_weights_array.data = AI_PTR(g_fallower1_weights_map[0] + 3488);
    _features_features_2_block_block_0_block_0_0_Conv_output_0_weights_array.data_start = AI_PTR(g_fallower1_weights_map[0] + 3488);
    _features_features_2_block_block_0_block_0_0_Conv_output_0_bias_array.format |= AI_FMT_FLAG_CONST;
    _features_features_2_block_block_0_block_0_0_Conv_output_0_bias_array.data = AI_PTR(g_fallower1_weights_map[0] + 8096);
    _features_features_2_block_block_0_block_0_0_Conv_output_0_bias_array.data_start = AI_PTR(g_fallower1_weights_map[0] + 8096);
    _features_features_2_block_block_1_block_1_0_Conv_output_0_weights_array.format |= AI_FMT_FLAG_CONST;
    _features_features_2_block_block_1_block_1_0_Conv_output_0_weights_array.data = AI_PTR(g_fallower1_weights_map[0] + 8384);
    _features_features_2_block_block_1_block_1_0_Conv_output_0_weights_array.data_start = AI_PTR(g_fallower1_weights_map[0] + 8384);
    _features_features_2_block_block_1_block_1_0_Conv_output_0_bias_array.format |= AI_FMT_FLAG_CONST;
    _features_features_2_block_block_1_block_1_0_Conv_output_0_bias_array.data = AI_PTR(g_fallower1_weights_map[0] + 10976);
    _features_features_2_block_block_1_block_1_0_Conv_output_0_bias_array.data_start = AI_PTR(g_fallower1_weights_map[0] + 10976);
    _features_features_2_block_block_2_block_2_0_Conv_output_0_weights_array.format |= AI_FMT_FLAG_CONST;
    _features_features_2_block_block_2_block_2_0_Conv_output_0_weights_array.data = AI_PTR(g_fallower1_weights_map[0] + 11264);
    _features_features_2_block_block_2_block_2_0_Conv_output_0_weights_array.data_start = AI_PTR(g_fallower1_weights_map[0] + 11264);
    _features_features_2_block_block_2_block_2_0_Conv_output_0_bias_array.format |= AI_FMT_FLAG_CONST;
    _features_features_2_block_block_2_block_2_0_Conv_output_0_bias_array.data = AI_PTR(g_fallower1_weights_map[0] + 18176);
    _features_features_2_block_block_2_block_2_0_Conv_output_0_bias_array.data_start = AI_PTR(g_fallower1_weights_map[0] + 18176);
    _features_features_3_block_block_0_block_0_0_Conv_output_0_weights_array.format |= AI_FMT_FLAG_CONST;
    _features_features_3_block_block_0_block_0_0_Conv_output_0_weights_array.data = AI_PTR(g_fallower1_weights_map[0] + 18272);
    _features_features_3_block_block_0_block_0_0_Conv_output_0_weights_array.data_start = AI_PTR(g_fallower1_weights_map[0] + 18272);
    _features_features_3_block_block_0_block_0_0_Conv_output_0_bias_array.format |= AI_FMT_FLAG_CONST;
    _features_features_3_block_block_0_block_0_0_Conv_output_0_bias_array.data = AI_PTR(g_fallower1_weights_map[0] + 26720);
    _features_features_3_block_block_0_block_0_0_Conv_output_0_bias_array.data_start = AI_PTR(g_fallower1_weights_map[0] + 26720);
    _features_features_3_block_block_1_block_1_0_Conv_output_0_weights_array.format |= AI_FMT_FLAG_CONST;
    _features_features_3_block_block_1_block_1_0_Conv_output_0_weights_array.data = AI_PTR(g_fallower1_weights_map[0] + 27072);
    _features_features_3_block_block_1_block_1_0_Conv_output_0_weights_array.data_start = AI_PTR(g_fallower1_weights_map[0] + 27072);
    _features_features_3_block_block_1_block_1_0_Conv_output_0_bias_array.format |= AI_FMT_FLAG_CONST;
    _features_features_3_block_block_1_block_1_0_Conv_output_0_bias_array.data = AI_PTR(g_fallower1_weights_map[0] + 30240);
    _features_features_3_block_block_1_block_1_0_Conv_output_0_bias_array.data_start = AI_PTR(g_fallower1_weights_map[0] + 30240);
    _features_features_3_block_block_2_block_2_0_Conv_output_0_weights_array.format |= AI_FMT_FLAG_CONST;
    _features_features_3_block_block_2_block_2_0_Conv_output_0_weights_array.data = AI_PTR(g_fallower1_weights_map[0] + 30592);
    _features_features_3_block_block_2_block_2_0_Conv_output_0_weights_array.data_start = AI_PTR(g_fallower1_weights_map[0] + 30592);
    _features_features_3_block_block_2_block_2_0_Conv_output_0_bias_array.format |= AI_FMT_FLAG_CONST;
    _features_features_3_block_block_2_block_2_0_Conv_output_0_bias_array.data = AI_PTR(g_fallower1_weights_map[0] + 39040);
    _features_features_3_block_block_2_block_2_0_Conv_output_0_bias_array.data_start = AI_PTR(g_fallower1_weights_map[0] + 39040);
    _features_features_4_block_block_0_block_0_0_Conv_output_0_weights_array.format |= AI_FMT_FLAG_CONST;
    _features_features_4_block_block_0_block_0_0_Conv_output_0_weights_array.data = AI_PTR(g_fallower1_weights_map[0] + 39136);
    _features_features_4_block_block_0_block_0_0_Conv_output_0_weights_array.data_start = AI_PTR(g_fallower1_weights_map[0] + 39136);
    _features_features_4_block_block_0_block_0_0_Conv_output_0_bias_array.format |= AI_FMT_FLAG_CONST;
    _features_features_4_block_block_0_block_0_0_Conv_output_0_bias_array.data = AI_PTR(g_fallower1_weights_map[0] + 48352);
    _features_features_4_block_block_0_block_0_0_Conv_output_0_bias_array.data_start = AI_PTR(g_fallower1_weights_map[0] + 48352);
    _features_features_4_block_block_1_block_1_0_Conv_output_0_weights_array.format |= AI_FMT_FLAG_CONST;
    _features_features_4_block_block_1_block_1_0_Conv_output_0_weights_array.data = AI_PTR(g_fallower1_weights_map[0] + 48736);
    _features_features_4_block_block_1_block_1_0_Conv_output_0_weights_array.data_start = AI_PTR(g_fallower1_weights_map[0] + 48736);
    _features_features_4_block_block_1_block_1_0_Conv_output_0_bias_array.format |= AI_FMT_FLAG_CONST;
    _features_features_4_block_block_1_block_1_0_Conv_output_0_bias_array.data = AI_PTR(g_fallower1_weights_map[0] + 58336);
    _features_features_4_block_block_1_block_1_0_Conv_output_0_bias_array.data_start = AI_PTR(g_fallower1_weights_map[0] + 58336);
    _features_features_4_block_block_2_fc1_Conv_output_0_weights_array.format |= AI_FMT_FLAG_CONST;
    _features_features_4_block_block_2_fc1_Conv_output_0_weights_array.data = AI_PTR(g_fallower1_weights_map[0] + 58720);
    _features_features_4_block_block_2_fc1_Conv_output_0_weights_array.data_start = AI_PTR(g_fallower1_weights_map[0] + 58720);
    _features_features_4_block_block_2_fc1_Conv_output_0_bias_array.format |= AI_FMT_FLAG_CONST;
    _features_features_4_block_block_2_fc1_Conv_output_0_bias_array.data = AI_PTR(g_fallower1_weights_map[0] + 67936);
    _features_features_4_block_block_2_fc1_Conv_output_0_bias_array.data_start = AI_PTR(g_fallower1_weights_map[0] + 67936);
    _features_features_4_block_block_2_fc2_Conv_output_0_weights_array.format |= AI_FMT_FLAG_CONST;
    _features_features_4_block_block_2_fc2_Conv_output_0_weights_array.data = AI_PTR(g_fallower1_weights_map[0] + 68032);
    _features_features_4_block_block_2_fc2_Conv_output_0_weights_array.data_start = AI_PTR(g_fallower1_weights_map[0] + 68032);
    _features_features_4_block_block_2_fc2_Conv_output_0_bias_array.format |= AI_FMT_FLAG_CONST;
    _features_features_4_block_block_2_fc2_Conv_output_0_bias_array.data = AI_PTR(g_fallower1_weights_map[0] + 77248);
    _features_features_4_block_block_2_fc2_Conv_output_0_bias_array.data_start = AI_PTR(g_fallower1_weights_map[0] + 77248);
    _features_features_4_block_block_3_block_3_0_Conv_output_0_weights_array.format |= AI_FMT_FLAG_CONST;
    _features_features_4_block_block_3_block_3_0_Conv_output_0_weights_array.data = AI_PTR(g_fallower1_weights_map[0] + 77632);
    _features_features_4_block_block_3_block_3_0_Conv_output_0_weights_array.data_start = AI_PTR(g_fallower1_weights_map[0] + 77632);
    _features_features_4_block_block_3_block_3_0_Conv_output_0_bias_array.format |= AI_FMT_FLAG_CONST;
    _features_features_4_block_block_3_block_3_0_Conv_output_0_bias_array.data = AI_PTR(g_fallower1_weights_map[0] + 92992);
    _features_features_4_block_block_3_block_3_0_Conv_output_0_bias_array.data_start = AI_PTR(g_fallower1_weights_map[0] + 92992);
    _features_features_5_block_block_0_block_0_0_Conv_output_0_weights_array.format |= AI_FMT_FLAG_CONST;
    _features_features_5_block_block_0_block_0_0_Conv_output_0_weights_array.data = AI_PTR(g_fallower1_weights_map[0] + 93152);
    _features_features_5_block_block_0_block_0_0_Conv_output_0_weights_array.data_start = AI_PTR(g_fallower1_weights_map[0] + 93152);
    _features_features_5_block_block_0_block_0_0_Conv_output_0_bias_array.format |= AI_FMT_FLAG_CONST;
    _features_features_5_block_block_0_block_0_0_Conv_output_0_bias_array.data = AI_PTR(g_fallower1_weights_map[0] + 131552);
    _features_features_5_block_block_0_block_0_0_Conv_output_0_bias_array.data_start = AI_PTR(g_fallower1_weights_map[0] + 131552);
    _features_features_5_block_block_1_block_1_0_Conv_output_0_weights_array.format |= AI_FMT_FLAG_CONST;
    _features_features_5_block_block_1_block_1_0_Conv_output_0_weights_array.data = AI_PTR(g_fallower1_weights_map[0] + 132512);
    _features_features_5_block_block_1_block_1_0_Conv_output_0_weights_array.data_start = AI_PTR(g_fallower1_weights_map[0] + 132512);
    _features_features_5_block_block_1_block_1_0_Conv_output_0_bias_array.format |= AI_FMT_FLAG_CONST;
    _features_features_5_block_block_1_block_1_0_Conv_output_0_bias_array.data = AI_PTR(g_fallower1_weights_map[0] + 156512);
    _features_features_5_block_block_1_block_1_0_Conv_output_0_bias_array.data_start = AI_PTR(g_fallower1_weights_map[0] + 156512);
    _features_features_5_block_block_2_fc1_Conv_output_0_weights_array.format |= AI_FMT_FLAG_CONST;
    _features_features_5_block_block_2_fc1_Conv_output_0_weights_array.data = AI_PTR(g_fallower1_weights_map[0] + 157472);
    _features_features_5_block_block_2_fc1_Conv_output_0_weights_array.data_start = AI_PTR(g_fallower1_weights_map[0] + 157472);
    _features_features_5_block_block_2_fc1_Conv_output_0_bias_array.format |= AI_FMT_FLAG_CONST;
    _features_features_5_block_block_2_fc1_Conv_output_0_bias_array.data = AI_PTR(g_fallower1_weights_map[0] + 218912);
    _features_features_5_block_block_2_fc1_Conv_output_0_bias_array.data_start = AI_PTR(g_fallower1_weights_map[0] + 218912);
    _features_features_5_block_block_2_fc2_Conv_output_0_weights_array.format |= AI_FMT_FLAG_CONST;
    _features_features_5_block_block_2_fc2_Conv_output_0_weights_array.data = AI_PTR(g_fallower1_weights_map[0] + 219168);
    _features_features_5_block_block_2_fc2_Conv_output_0_weights_array.data_start = AI_PTR(g_fallower1_weights_map[0] + 219168);
    _features_features_5_block_block_2_fc2_Conv_output_0_bias_array.format |= AI_FMT_FLAG_CONST;
    _features_features_5_block_block_2_fc2_Conv_output_0_bias_array.data = AI_PTR(g_fallower1_weights_map[0] + 280608);
    _features_features_5_block_block_2_fc2_Conv_output_0_bias_array.data_start = AI_PTR(g_fallower1_weights_map[0] + 280608);
    _features_features_5_block_block_3_block_3_0_Conv_output_0_weights_array.format |= AI_FMT_FLAG_CONST;
    _features_features_5_block_block_3_block_3_0_Conv_output_0_weights_array.data = AI_PTR(g_fallower1_weights_map[0] + 281568);
    _features_features_5_block_block_3_block_3_0_Conv_output_0_weights_array.data_start = AI_PTR(g_fallower1_weights_map[0] + 281568);
    _features_features_5_block_block_3_block_3_0_Conv_output_0_bias_array.format |= AI_FMT_FLAG_CONST;
    _features_features_5_block_block_3_block_3_0_Conv_output_0_bias_array.data = AI_PTR(g_fallower1_weights_map[0] + 319968);
    _features_features_5_block_block_3_block_3_0_Conv_output_0_bias_array.data_start = AI_PTR(g_fallower1_weights_map[0] + 319968);
    _features_features_6_block_block_0_block_0_0_Conv_output_0_weights_array.format |= AI_FMT_FLAG_CONST;
    _features_features_6_block_block_0_block_0_0_Conv_output_0_weights_array.data = AI_PTR(g_fallower1_weights_map[0] + 320128);
    _features_features_6_block_block_0_block_0_0_Conv_output_0_weights_array.data_start = AI_PTR(g_fallower1_weights_map[0] + 320128);
    _features_features_6_block_block_0_block_0_0_Conv_output_0_bias_array.format |= AI_FMT_FLAG_CONST;
    _features_features_6_block_block_0_block_0_0_Conv_output_0_bias_array.data = AI_PTR(g_fallower1_weights_map[0] + 358528);
    _features_features_6_block_block_0_block_0_0_Conv_output_0_bias_array.data_start = AI_PTR(g_fallower1_weights_map[0] + 358528);
    _features_features_6_block_block_1_block_1_0_Conv_output_0_weights_array.format |= AI_FMT_FLAG_CONST;
    _features_features_6_block_block_1_block_1_0_Conv_output_0_weights_array.data = AI_PTR(g_fallower1_weights_map[0] + 359488);
    _features_features_6_block_block_1_block_1_0_Conv_output_0_weights_array.data_start = AI_PTR(g_fallower1_weights_map[0] + 359488);
    _features_features_6_block_block_1_block_1_0_Conv_output_0_bias_array.format |= AI_FMT_FLAG_CONST;
    _features_features_6_block_block_1_block_1_0_Conv_output_0_bias_array.data = AI_PTR(g_fallower1_weights_map[0] + 383488);
    _features_features_6_block_block_1_block_1_0_Conv_output_0_bias_array.data_start = AI_PTR(g_fallower1_weights_map[0] + 383488);
    _features_features_6_block_block_2_fc1_Conv_output_0_weights_array.format |= AI_FMT_FLAG_CONST;
    _features_features_6_block_block_2_fc1_Conv_output_0_weights_array.data = AI_PTR(g_fallower1_weights_map[0] + 384448);
    _features_features_6_block_block_2_fc1_Conv_output_0_weights_array.data_start = AI_PTR(g_fallower1_weights_map[0] + 384448);
    _features_features_6_block_block_2_fc1_Conv_output_0_bias_array.format |= AI_FMT_FLAG_CONST;
    _features_features_6_block_block_2_fc1_Conv_output_0_bias_array.data = AI_PTR(g_fallower1_weights_map[0] + 445888);
    _features_features_6_block_block_2_fc1_Conv_output_0_bias_array.data_start = AI_PTR(g_fallower1_weights_map[0] + 445888);
    _features_features_6_block_block_2_fc2_Conv_output_0_weights_array.format |= AI_FMT_FLAG_CONST;
    _features_features_6_block_block_2_fc2_Conv_output_0_weights_array.data = AI_PTR(g_fallower1_weights_map[0] + 446144);
    _features_features_6_block_block_2_fc2_Conv_output_0_weights_array.data_start = AI_PTR(g_fallower1_weights_map[0] + 446144);
    _features_features_6_block_block_2_fc2_Conv_output_0_bias_array.format |= AI_FMT_FLAG_CONST;
    _features_features_6_block_block_2_fc2_Conv_output_0_bias_array.data = AI_PTR(g_fallower1_weights_map[0] + 507584);
    _features_features_6_block_block_2_fc2_Conv_output_0_bias_array.data_start = AI_PTR(g_fallower1_weights_map[0] + 507584);
    _features_features_6_block_block_3_block_3_0_Conv_output_0_weights_array.format |= AI_FMT_FLAG_CONST;
    _features_features_6_block_block_3_block_3_0_Conv_output_0_weights_array.data = AI_PTR(g_fallower1_weights_map[0] + 508544);
    _features_features_6_block_block_3_block_3_0_Conv_output_0_weights_array.data_start = AI_PTR(g_fallower1_weights_map[0] + 508544);
    _features_features_6_block_block_3_block_3_0_Conv_output_0_bias_array.format |= AI_FMT_FLAG_CONST;
    _features_features_6_block_block_3_block_3_0_Conv_output_0_bias_array.data = AI_PTR(g_fallower1_weights_map[0] + 546944);
    _features_features_6_block_block_3_block_3_0_Conv_output_0_bias_array.data_start = AI_PTR(g_fallower1_weights_map[0] + 546944);
    _features_features_7_block_block_0_block_0_0_Conv_output_0_weights_array.format |= AI_FMT_FLAG_CONST;
    _features_features_7_block_block_0_block_0_0_Conv_output_0_weights_array.data = AI_PTR(g_fallower1_weights_map[0] + 547104);
    _features_features_7_block_block_0_block_0_0_Conv_output_0_weights_array.data_start = AI_PTR(g_fallower1_weights_map[0] + 547104);
    _features_features_7_block_block_0_block_0_0_Conv_output_0_bias_array.format |= AI_FMT_FLAG_CONST;
    _features_features_7_block_block_0_block_0_0_Conv_output_0_bias_array.data = AI_PTR(g_fallower1_weights_map[0] + 566304);
    _features_features_7_block_block_0_block_0_0_Conv_output_0_bias_array.data_start = AI_PTR(g_fallower1_weights_map[0] + 566304);
    _features_features_7_block_block_1_block_1_0_Conv_output_0_weights_array.format |= AI_FMT_FLAG_CONST;
    _features_features_7_block_block_1_block_1_0_Conv_output_0_weights_array.data = AI_PTR(g_fallower1_weights_map[0] + 566784);
    _features_features_7_block_block_1_block_1_0_Conv_output_0_weights_array.data_start = AI_PTR(g_fallower1_weights_map[0] + 566784);
    _features_features_7_block_block_1_block_1_0_Conv_output_0_bias_array.format |= AI_FMT_FLAG_CONST;
    _features_features_7_block_block_1_block_1_0_Conv_output_0_bias_array.data = AI_PTR(g_fallower1_weights_map[0] + 578784);
    _features_features_7_block_block_1_block_1_0_Conv_output_0_bias_array.data_start = AI_PTR(g_fallower1_weights_map[0] + 578784);
    _features_features_7_block_block_2_fc1_Conv_output_0_weights_array.format |= AI_FMT_FLAG_CONST;
    _features_features_7_block_block_2_fc1_Conv_output_0_weights_array.data = AI_PTR(g_fallower1_weights_map[0] + 579264);
    _features_features_7_block_block_2_fc1_Conv_output_0_weights_array.data_start = AI_PTR(g_fallower1_weights_map[0] + 579264);
    _features_features_7_block_block_2_fc1_Conv_output_0_bias_array.format |= AI_FMT_FLAG_CONST;
    _features_features_7_block_block_2_fc1_Conv_output_0_bias_array.data = AI_PTR(g_fallower1_weights_map[0] + 594624);
    _features_features_7_block_block_2_fc1_Conv_output_0_bias_array.data_start = AI_PTR(g_fallower1_weights_map[0] + 594624);
    _features_features_7_block_block_2_fc2_Conv_output_0_weights_array.format |= AI_FMT_FLAG_CONST;
    _features_features_7_block_block_2_fc2_Conv_output_0_weights_array.data = AI_PTR(g_fallower1_weights_map[0] + 594752);
    _features_features_7_block_block_2_fc2_Conv_output_0_weights_array.data_start = AI_PTR(g_fallower1_weights_map[0] + 594752);
    _features_features_7_block_block_2_fc2_Conv_output_0_bias_array.format |= AI_FMT_FLAG_CONST;
    _features_features_7_block_block_2_fc2_Conv_output_0_bias_array.data = AI_PTR(g_fallower1_weights_map[0] + 610112);
    _features_features_7_block_block_2_fc2_Conv_output_0_bias_array.data_start = AI_PTR(g_fallower1_weights_map[0] + 610112);
    _features_features_7_block_block_3_block_3_0_Conv_output_0_weights_array.format |= AI_FMT_FLAG_CONST;
    _features_features_7_block_block_3_block_3_0_Conv_output_0_weights_array.data = AI_PTR(g_fallower1_weights_map[0] + 610592);
    _features_features_7_block_block_3_block_3_0_Conv_output_0_weights_array.data_start = AI_PTR(g_fallower1_weights_map[0] + 610592);
    _features_features_7_block_block_3_block_3_0_Conv_output_0_bias_array.format |= AI_FMT_FLAG_CONST;
    _features_features_7_block_block_3_block_3_0_Conv_output_0_bias_array.data = AI_PTR(g_fallower1_weights_map[0] + 633632);
    _features_features_7_block_block_3_block_3_0_Conv_output_0_bias_array.data_start = AI_PTR(g_fallower1_weights_map[0] + 633632);
    _features_features_8_block_block_0_block_0_0_Conv_output_0_weights_array.format |= AI_FMT_FLAG_CONST;
    _features_features_8_block_block_0_block_0_0_Conv_output_0_weights_array.data = AI_PTR(g_fallower1_weights_map[0] + 633824);
    _features_features_8_block_block_0_block_0_0_Conv_output_0_weights_array.data_start = AI_PTR(g_fallower1_weights_map[0] + 633824);
    _features_features_8_block_block_0_block_0_0_Conv_output_0_bias_array.format |= AI_FMT_FLAG_CONST;
    _features_features_8_block_block_0_block_0_0_Conv_output_0_bias_array.data = AI_PTR(g_fallower1_weights_map[0] + 661472);
    _features_features_8_block_block_0_block_0_0_Conv_output_0_bias_array.data_start = AI_PTR(g_fallower1_weights_map[0] + 661472);
    _features_features_8_block_block_1_block_1_0_Conv_output_0_weights_array.format |= AI_FMT_FLAG_CONST;
    _features_features_8_block_block_1_block_1_0_Conv_output_0_weights_array.data = AI_PTR(g_fallower1_weights_map[0] + 662048);
    _features_features_8_block_block_1_block_1_0_Conv_output_0_weights_array.data_start = AI_PTR(g_fallower1_weights_map[0] + 662048);
    _features_features_8_block_block_1_block_1_0_Conv_output_0_bias_array.format |= AI_FMT_FLAG_CONST;
    _features_features_8_block_block_1_block_1_0_Conv_output_0_bias_array.data = AI_PTR(g_fallower1_weights_map[0] + 676448);
    _features_features_8_block_block_1_block_1_0_Conv_output_0_bias_array.data_start = AI_PTR(g_fallower1_weights_map[0] + 676448);
    _features_features_8_block_block_2_fc1_Conv_output_0_weights_array.format |= AI_FMT_FLAG_CONST;
    _features_features_8_block_block_2_fc1_Conv_output_0_weights_array.data = AI_PTR(g_fallower1_weights_map[0] + 677024);
    _features_features_8_block_block_2_fc1_Conv_output_0_weights_array.data_start = AI_PTR(g_fallower1_weights_map[0] + 677024);
    _features_features_8_block_block_2_fc1_Conv_output_0_bias_array.format |= AI_FMT_FLAG_CONST;
    _features_features_8_block_block_2_fc1_Conv_output_0_bias_array.data = AI_PTR(g_fallower1_weights_map[0] + 700064);
    _features_features_8_block_block_2_fc1_Conv_output_0_bias_array.data_start = AI_PTR(g_fallower1_weights_map[0] + 700064);
    _features_features_8_block_block_2_fc2_Conv_output_0_weights_array.format |= AI_FMT_FLAG_CONST;
    _features_features_8_block_block_2_fc2_Conv_output_0_weights_array.data = AI_PTR(g_fallower1_weights_map[0] + 700224);
    _features_features_8_block_block_2_fc2_Conv_output_0_weights_array.data_start = AI_PTR(g_fallower1_weights_map[0] + 700224);
    _features_features_8_block_block_2_fc2_Conv_output_0_bias_array.format |= AI_FMT_FLAG_CONST;
    _features_features_8_block_block_2_fc2_Conv_output_0_bias_array.data = AI_PTR(g_fallower1_weights_map[0] + 723264);
    _features_features_8_block_block_2_fc2_Conv_output_0_bias_array.data_start = AI_PTR(g_fallower1_weights_map[0] + 723264);
    _features_features_8_block_block_3_block_3_0_Conv_output_0_weights_array.format |= AI_FMT_FLAG_CONST;
    _features_features_8_block_block_3_block_3_0_Conv_output_0_weights_array.data = AI_PTR(g_fallower1_weights_map[0] + 723840);
    _features_features_8_block_block_3_block_3_0_Conv_output_0_weights_array.data_start = AI_PTR(g_fallower1_weights_map[0] + 723840);
    _features_features_8_block_block_3_block_3_0_Conv_output_0_bias_array.format |= AI_FMT_FLAG_CONST;
    _features_features_8_block_block_3_block_3_0_Conv_output_0_bias_array.data = AI_PTR(g_fallower1_weights_map[0] + 751488);
    _features_features_8_block_block_3_block_3_0_Conv_output_0_bias_array.data_start = AI_PTR(g_fallower1_weights_map[0] + 751488);
    _features_features_9_block_block_0_block_0_0_Conv_output_0_weights_array.format |= AI_FMT_FLAG_CONST;
    _features_features_9_block_block_0_block_0_0_Conv_output_0_weights_array.data = AI_PTR(g_fallower1_weights_map[0] + 751680);
    _features_features_9_block_block_0_block_0_0_Conv_output_0_weights_array.data_start = AI_PTR(g_fallower1_weights_map[0] + 751680);
    _features_features_9_block_block_0_block_0_0_Conv_output_0_bias_array.format |= AI_FMT_FLAG_CONST;
    _features_features_9_block_block_0_block_0_0_Conv_output_0_bias_array.data = AI_PTR(g_fallower1_weights_map[0] + 806976);
    _features_features_9_block_block_0_block_0_0_Conv_output_0_bias_array.data_start = AI_PTR(g_fallower1_weights_map[0] + 806976);
    _features_features_9_block_block_1_block_1_0_Conv_output_0_weights_array.format |= AI_FMT_FLAG_CONST;
    _features_features_9_block_block_1_block_1_0_Conv_output_0_weights_array.data = AI_PTR(g_fallower1_weights_map[0] + 808128);
    _features_features_9_block_block_1_block_1_0_Conv_output_0_weights_array.data_start = AI_PTR(g_fallower1_weights_map[0] + 808128);
    _features_features_9_block_block_1_block_1_0_Conv_output_0_bias_array.format |= AI_FMT_FLAG_CONST;
    _features_features_9_block_block_1_block_1_0_Conv_output_0_bias_array.data = AI_PTR(g_fallower1_weights_map[0] + 836928);
    _features_features_9_block_block_1_block_1_0_Conv_output_0_bias_array.data_start = AI_PTR(g_fallower1_weights_map[0] + 836928);
    _features_features_9_block_block_2_fc1_Conv_output_0_weights_array.format |= AI_FMT_FLAG_CONST;
    _features_features_9_block_block_2_fc1_Conv_output_0_weights_array.data = AI_PTR(g_fallower1_weights_map[0] + 838080);
    _features_features_9_block_block_2_fc1_Conv_output_0_weights_array.data_start = AI_PTR(g_fallower1_weights_map[0] + 838080);
    _features_features_9_block_block_2_fc1_Conv_output_0_bias_array.format |= AI_FMT_FLAG_CONST;
    _features_features_9_block_block_2_fc1_Conv_output_0_bias_array.data = AI_PTR(g_fallower1_weights_map[0] + 921024);
    _features_features_9_block_block_2_fc1_Conv_output_0_bias_array.data_start = AI_PTR(g_fallower1_weights_map[0] + 921024);
    _features_features_9_block_block_2_fc2_Conv_output_0_weights_array.format |= AI_FMT_FLAG_CONST;
    _features_features_9_block_block_2_fc2_Conv_output_0_weights_array.data = AI_PTR(g_fallower1_weights_map[0] + 921312);
    _features_features_9_block_block_2_fc2_Conv_output_0_weights_array.data_start = AI_PTR(g_fallower1_weights_map[0] + 921312);
    _features_features_9_block_block_2_fc2_Conv_output_0_bias_array.format |= AI_FMT_FLAG_CONST;
    _features_features_9_block_block_2_fc2_Conv_output_0_bias_array.data = AI_PTR(g_fallower1_weights_map[0] + 1004256);
    _features_features_9_block_block_2_fc2_Conv_output_0_bias_array.data_start = AI_PTR(g_fallower1_weights_map[0] + 1004256);
    _features_features_9_block_block_3_block_3_0_Conv_output_0_weights_array.format |= AI_FMT_FLAG_CONST;
    _features_features_9_block_block_3_block_3_0_Conv_output_0_weights_array.data = AI_PTR(g_fallower1_weights_map[0] + 1005408);
    _features_features_9_block_block_3_block_3_0_Conv_output_0_weights_array.data_start = AI_PTR(g_fallower1_weights_map[0] + 1005408);
    _features_features_9_block_block_3_block_3_0_Conv_output_0_bias_array.format |= AI_FMT_FLAG_CONST;
    _features_features_9_block_block_3_block_3_0_Conv_output_0_bias_array.data = AI_PTR(g_fallower1_weights_map[0] + 1116000);
    _features_features_9_block_block_3_block_3_0_Conv_output_0_bias_array.data_start = AI_PTR(g_fallower1_weights_map[0] + 1116000);
    _features_features_10_block_block_0_block_0_0_Conv_output_0_weights_array.format |= AI_FMT_FLAG_CONST;
    _features_features_10_block_block_0_block_0_0_Conv_output_0_weights_array.data = AI_PTR(g_fallower1_weights_map[0] + 1116384);
    _features_features_10_block_block_0_block_0_0_Conv_output_0_weights_array.data_start = AI_PTR(g_fallower1_weights_map[0] + 1116384);
    _features_features_10_block_block_0_block_0_0_Conv_output_0_bias_array.format |= AI_FMT_FLAG_CONST;
    _features_features_10_block_block_0_block_0_0_Conv_output_0_bias_array.data = AI_PTR(g_fallower1_weights_map[0] + 1337568);
    _features_features_10_block_block_0_block_0_0_Conv_output_0_bias_array.data_start = AI_PTR(g_fallower1_weights_map[0] + 1337568);
    _features_features_10_block_block_1_block_1_0_Conv_output_0_weights_array.format |= AI_FMT_FLAG_CONST;
    _features_features_10_block_block_1_block_1_0_Conv_output_0_weights_array.data = AI_PTR(g_fallower1_weights_map[0] + 1339872);
    _features_features_10_block_block_1_block_1_0_Conv_output_0_weights_array.data_start = AI_PTR(g_fallower1_weights_map[0] + 1339872);
    _features_features_10_block_block_1_block_1_0_Conv_output_0_bias_array.format |= AI_FMT_FLAG_CONST;
    _features_features_10_block_block_1_block_1_0_Conv_output_0_bias_array.data = AI_PTR(g_fallower1_weights_map[0] + 1397472);
    _features_features_10_block_block_1_block_1_0_Conv_output_0_bias_array.data_start = AI_PTR(g_fallower1_weights_map[0] + 1397472);
    _features_features_10_block_block_2_fc1_Conv_output_0_weights_array.format |= AI_FMT_FLAG_CONST;
    _features_features_10_block_block_2_fc1_Conv_output_0_weights_array.data = AI_PTR(g_fallower1_weights_map[0] + 1399776);
    _features_features_10_block_block_2_fc1_Conv_output_0_weights_array.data_start = AI_PTR(g_fallower1_weights_map[0] + 1399776);
    _features_features_10_block_block_2_fc1_Conv_output_0_bias_array.format |= AI_FMT_FLAG_CONST;
    _features_features_10_block_block_2_fc1_Conv_output_0_bias_array.data = AI_PTR(g_fallower1_weights_map[0] + 1731552);
    _features_features_10_block_block_2_fc1_Conv_output_0_bias_array.data_start = AI_PTR(g_fallower1_weights_map[0] + 1731552);
    _features_features_10_block_block_2_fc2_Conv_output_0_weights_array.format |= AI_FMT_FLAG_CONST;
    _features_features_10_block_block_2_fc2_Conv_output_0_weights_array.data = AI_PTR(g_fallower1_weights_map[0] + 1732128);
    _features_features_10_block_block_2_fc2_Conv_output_0_weights_array.data_start = AI_PTR(g_fallower1_weights_map[0] + 1732128);
    _features_features_10_block_block_2_fc2_Conv_output_0_bias_array.format |= AI_FMT_FLAG_CONST;
    _features_features_10_block_block_2_fc2_Conv_output_0_bias_array.data = AI_PTR(g_fallower1_weights_map[0] + 2063904);
    _features_features_10_block_block_2_fc2_Conv_output_0_bias_array.data_start = AI_PTR(g_fallower1_weights_map[0] + 2063904);
    _features_features_10_block_block_3_block_3_0_Conv_output_0_weights_array.format |= AI_FMT_FLAG_CONST;
    _features_features_10_block_block_3_block_3_0_Conv_output_0_weights_array.data = AI_PTR(g_fallower1_weights_map[0] + 2066208);
    _features_features_10_block_block_3_block_3_0_Conv_output_0_weights_array.data_start = AI_PTR(g_fallower1_weights_map[0] + 2066208);
    _features_features_10_block_block_3_block_3_0_Conv_output_0_bias_array.format |= AI_FMT_FLAG_CONST;
    _features_features_10_block_block_3_block_3_0_Conv_output_0_bias_array.data = AI_PTR(g_fallower1_weights_map[0] + 2287392);
    _features_features_10_block_block_3_block_3_0_Conv_output_0_bias_array.data_start = AI_PTR(g_fallower1_weights_map[0] + 2287392);
    _features_features_11_block_block_0_block_0_0_Conv_output_0_weights_array.format |= AI_FMT_FLAG_CONST;
    _features_features_11_block_block_0_block_0_0_Conv_output_0_weights_array.data = AI_PTR(g_fallower1_weights_map[0] + 2287776);
    _features_features_11_block_block_0_block_0_0_Conv_output_0_weights_array.data_start = AI_PTR(g_fallower1_weights_map[0] + 2287776);
    _features_features_11_block_block_0_block_0_0_Conv_output_0_bias_array.format |= AI_FMT_FLAG_CONST;
    _features_features_11_block_block_0_block_0_0_Conv_output_0_bias_array.data = AI_PTR(g_fallower1_weights_map[0] + 2508960);
    _features_features_11_block_block_0_block_0_0_Conv_output_0_bias_array.data_start = AI_PTR(g_fallower1_weights_map[0] + 2508960);
    _features_features_11_block_block_1_block_1_0_Conv_output_0_weights_array.format |= AI_FMT_FLAG_CONST;
    _features_features_11_block_block_1_block_1_0_Conv_output_0_weights_array.data = AI_PTR(g_fallower1_weights_map[0] + 2511264);
    _features_features_11_block_block_1_block_1_0_Conv_output_0_weights_array.data_start = AI_PTR(g_fallower1_weights_map[0] + 2511264);
    _features_features_11_block_block_1_block_1_0_Conv_output_0_bias_array.format |= AI_FMT_FLAG_CONST;
    _features_features_11_block_block_1_block_1_0_Conv_output_0_bias_array.data = AI_PTR(g_fallower1_weights_map[0] + 2568864);
    _features_features_11_block_block_1_block_1_0_Conv_output_0_bias_array.data_start = AI_PTR(g_fallower1_weights_map[0] + 2568864);
    _features_features_11_block_block_2_fc1_Conv_output_0_weights_array.format |= AI_FMT_FLAG_CONST;
    _features_features_11_block_block_2_fc1_Conv_output_0_weights_array.data = AI_PTR(g_fallower1_weights_map[0] + 2571168);
    _features_features_11_block_block_2_fc1_Conv_output_0_weights_array.data_start = AI_PTR(g_fallower1_weights_map[0] + 2571168);
    _features_features_11_block_block_2_fc1_Conv_output_0_bias_array.format |= AI_FMT_FLAG_CONST;
    _features_features_11_block_block_2_fc1_Conv_output_0_bias_array.data = AI_PTR(g_fallower1_weights_map[0] + 2902944);
    _features_features_11_block_block_2_fc1_Conv_output_0_bias_array.data_start = AI_PTR(g_fallower1_weights_map[0] + 2902944);
    _features_features_11_block_block_2_fc2_Conv_output_0_weights_array.format |= AI_FMT_FLAG_CONST;
    _features_features_11_block_block_2_fc2_Conv_output_0_weights_array.data = AI_PTR(g_fallower1_weights_map[0] + 2903520);
    _features_features_11_block_block_2_fc2_Conv_output_0_weights_array.data_start = AI_PTR(g_fallower1_weights_map[0] + 2903520);
    _features_features_11_block_block_2_fc2_Conv_output_0_bias_array.format |= AI_FMT_FLAG_CONST;
    _features_features_11_block_block_2_fc2_Conv_output_0_bias_array.data = AI_PTR(g_fallower1_weights_map[0] + 3235296);
    _features_features_11_block_block_2_fc2_Conv_output_0_bias_array.data_start = AI_PTR(g_fallower1_weights_map[0] + 3235296);
    _features_features_11_block_block_3_block_3_0_Conv_output_0_weights_array.format |= AI_FMT_FLAG_CONST;
    _features_features_11_block_block_3_block_3_0_Conv_output_0_weights_array.data = AI_PTR(g_fallower1_weights_map[0] + 3237600);
    _features_features_11_block_block_3_block_3_0_Conv_output_0_weights_array.data_start = AI_PTR(g_fallower1_weights_map[0] + 3237600);
    _features_features_11_block_block_3_block_3_0_Conv_output_0_bias_array.format |= AI_FMT_FLAG_CONST;
    _features_features_11_block_block_3_block_3_0_Conv_output_0_bias_array.data = AI_PTR(g_fallower1_weights_map[0] + 3458784);
    _features_features_11_block_block_3_block_3_0_Conv_output_0_bias_array.data_start = AI_PTR(g_fallower1_weights_map[0] + 3458784);
    _features_features_12_features_12_0_Conv_output_0_weights_array.format |= AI_FMT_FLAG_CONST;
    _features_features_12_features_12_0_Conv_output_0_weights_array.data = AI_PTR(g_fallower1_weights_map[0] + 3459168);
    _features_features_12_features_12_0_Conv_output_0_weights_array.data_start = AI_PTR(g_fallower1_weights_map[0] + 3459168);
    _features_features_12_features_12_0_Conv_output_0_bias_array.format |= AI_FMT_FLAG_CONST;
    _features_features_12_features_12_0_Conv_output_0_bias_array.data = AI_PTR(g_fallower1_weights_map[0] + 3680352);
    _features_features_12_features_12_0_Conv_output_0_bias_array.data_start = AI_PTR(g_fallower1_weights_map[0] + 3680352);
    _classifier_classifier_0_Gemm_output_0_weights_array.format |= AI_FMT_FLAG_CONST;
    _classifier_classifier_0_Gemm_output_0_weights_array.data = AI_PTR(g_fallower1_weights_map[0] + 3682656);
    _classifier_classifier_0_Gemm_output_0_weights_array.data_start = AI_PTR(g_fallower1_weights_map[0] + 3682656);
    _classifier_classifier_0_Gemm_output_0_bias_array.format |= AI_FMT_FLAG_CONST;
    _classifier_classifier_0_Gemm_output_0_bias_array.data = AI_PTR(g_fallower1_weights_map[0] + 4272480);
    _classifier_classifier_0_Gemm_output_0_bias_array.data_start = AI_PTR(g_fallower1_weights_map[0] + 4272480);
    output_weights_array.format |= AI_FMT_FLAG_CONST;
    output_weights_array.data = AI_PTR(g_fallower1_weights_map[0] + 4273504);
    output_weights_array.data_start = AI_PTR(g_fallower1_weights_map[0] + 4273504);
    output_bias_array.format |= AI_FMT_FLAG_CONST;
    output_bias_array.data = AI_PTR(g_fallower1_weights_map[0] + 4280672);
    output_bias_array.data_start = AI_PTR(g_fallower1_weights_map[0] + 4280672);
    return true;
  }
  AI_ERROR_TRAP(net_ctx, INIT_FAILED, NETWORK_WEIGHTS);
  return false;
}


/**  PUBLIC APIs SECTION  *****************************************************/



AI_DEPRECATED
AI_API_ENTRY
ai_bool ai_fallower1_get_info(
  ai_handle network, ai_network_report* report)
{
  ai_network* net_ctx = AI_NETWORK_ACQUIRE_CTX(network);

  if (report && net_ctx)
  {
    ai_network_report r = {
      .model_name        = AI_FALLOWER1_MODEL_NAME,
      .model_signature   = AI_FALLOWER1_MODEL_SIGNATURE,
      .model_datetime    = AI_TOOLS_DATE_TIME,
      
      .compile_datetime  = AI_TOOLS_COMPILE_TIME,
      
      .runtime_revision  = ai_platform_runtime_get_revision(),
      .runtime_version   = ai_platform_runtime_get_version(),

      .tool_revision     = AI_TOOLS_REVISION_ID,
      .tool_version      = {AI_TOOLS_VERSION_MAJOR, AI_TOOLS_VERSION_MINOR,
                            AI_TOOLS_VERSION_MICRO, 0x0},
      .tool_api_version  = AI_STRUCT_INIT,

      .api_version            = ai_platform_api_get_version(),
      .interface_api_version  = ai_platform_interface_api_get_version(),
      
      .n_macc            = 24802183,
      .n_inputs          = 0,
      .inputs            = NULL,
      .n_outputs         = 0,
      .outputs           = NULL,
      .params            = AI_STRUCT_INIT,
      .activations       = AI_STRUCT_INIT,
      .n_nodes           = 0,
      .signature         = 0xb2e07433,
    };

    if (!ai_platform_api_get_network_report(network, &r)) return false;

    *report = r;
    return true;
  }
  return false;
}



AI_API_ENTRY
ai_bool ai_fallower1_get_report(
  ai_handle network, ai_network_report* report)
{
  ai_network* net_ctx = AI_NETWORK_ACQUIRE_CTX(network);

  if (report && net_ctx)
  {
    ai_network_report r = {
      .model_name        = AI_FALLOWER1_MODEL_NAME,
      .model_signature   = AI_FALLOWER1_MODEL_SIGNATURE,
      .model_datetime    = AI_TOOLS_DATE_TIME,
      
      .compile_datetime  = AI_TOOLS_COMPILE_TIME,
      
      .runtime_revision  = ai_platform_runtime_get_revision(),
      .runtime_version   = ai_platform_runtime_get_version(),

      .tool_revision     = AI_TOOLS_REVISION_ID,
      .tool_version      = {AI_TOOLS_VERSION_MAJOR, AI_TOOLS_VERSION_MINOR,
                            AI_TOOLS_VERSION_MICRO, 0x0},
      .tool_api_version  = AI_STRUCT_INIT,

      .api_version            = ai_platform_api_get_version(),
      .interface_api_version  = ai_platform_interface_api_get_version(),
      
      .n_macc            = 24802183,
      .n_inputs          = 0,
      .inputs            = NULL,
      .n_outputs         = 0,
      .outputs           = NULL,
      .map_signature     = AI_MAGIC_SIGNATURE,
      .map_weights       = AI_STRUCT_INIT,
      .map_activations   = AI_STRUCT_INIT,
      .n_nodes           = 0,
      .signature         = 0xb2e07433,
    };

    if (!ai_platform_api_get_network_report(network, &r)) return false;

    *report = r;
    return true;
  }
  return false;
}


AI_API_ENTRY
ai_error ai_fallower1_get_error(ai_handle network)
{
  return ai_platform_network_get_error(network);
}


AI_API_ENTRY
ai_error ai_fallower1_create(
  ai_handle* network, const ai_buffer* network_config)
{
  return ai_platform_network_create(
    network, network_config, 
    AI_CONTEXT_OBJ(&AI_NET_OBJ_INSTANCE),
    AI_TOOLS_API_VERSION_MAJOR, AI_TOOLS_API_VERSION_MINOR, AI_TOOLS_API_VERSION_MICRO);
}


AI_API_ENTRY
ai_error ai_fallower1_create_and_init(
  ai_handle* network, const ai_handle activations[], const ai_handle weights[])
{
  ai_error err;
  ai_network_params params;

  err = ai_fallower1_create(network, AI_FALLOWER1_DATA_CONFIG);
  if (err.type != AI_ERROR_NONE) {
    return err;
  }
  
  if (ai_fallower1_data_params_get(&params) != true) {
    err = ai_fallower1_get_error(*network);
    return err;
  }
#if defined(AI_FALLOWER1_DATA_ACTIVATIONS_COUNT)
  /* set the addresses of the activations buffers */
  for (ai_u16 idx=0; activations && idx<params.map_activations.size; idx++) {
    AI_BUFFER_ARRAY_ITEM_SET_ADDRESS(&params.map_activations, idx, activations[idx]);
  }
#endif
#if defined(AI_FALLOWER1_DATA_WEIGHTS_COUNT)
  /* set the addresses of the weight buffers */
  for (ai_u16 idx=0; weights && idx<params.map_weights.size; idx++) {
    AI_BUFFER_ARRAY_ITEM_SET_ADDRESS(&params.map_weights, idx, weights[idx]);
  }
#endif
  if (ai_fallower1_init(*network, &params) != true) {
    err = ai_fallower1_get_error(*network);
  }
  return err;
}


AI_API_ENTRY
ai_buffer* ai_fallower1_inputs_get(ai_handle network, ai_u16 *n_buffer)
{
  if (network == AI_HANDLE_NULL) {
    network = (ai_handle)&AI_NET_OBJ_INSTANCE;
    AI_NETWORK_OBJ(network)->magic = AI_MAGIC_CONTEXT_TOKEN;
  }
  return ai_platform_inputs_get(network, n_buffer);
}


AI_API_ENTRY
ai_buffer* ai_fallower1_outputs_get(ai_handle network, ai_u16 *n_buffer)
{
  if (network == AI_HANDLE_NULL) {
    network = (ai_handle)&AI_NET_OBJ_INSTANCE;
    AI_NETWORK_OBJ(network)->magic = AI_MAGIC_CONTEXT_TOKEN;
  }
  return ai_platform_outputs_get(network, n_buffer);
}


AI_API_ENTRY
ai_handle ai_fallower1_destroy(ai_handle network)
{
  return ai_platform_network_destroy(network);
}


AI_API_ENTRY
ai_bool ai_fallower1_init(
  ai_handle network, const ai_network_params* params)
{
  ai_network* net_ctx = AI_NETWORK_OBJ(ai_platform_network_init(network, params));
  ai_bool ok = true;

  if (!net_ctx) return false;
  ok &= fallower1_configure_weights(net_ctx, params);
  ok &= fallower1_configure_activations(net_ctx, params);

  ok &= ai_platform_network_post_init(network);

  return ok;
}


AI_API_ENTRY
ai_i32 ai_fallower1_run(
  ai_handle network, const ai_buffer* input, ai_buffer* output)
{
  return ai_platform_network_process(network, input, output);
}


AI_API_ENTRY
ai_i32 ai_fallower1_forward(ai_handle network, const ai_buffer* input)
{
  return ai_platform_network_process(network, input, NULL);
}



#undef AI_FALLOWER1_MODEL_SIGNATURE
#undef AI_NET_OBJ_INSTANCE
#undef AI_TOOLS_DATE_TIME
#undef AI_TOOLS_COMPILE_TIME

