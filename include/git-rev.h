/* SPDX-License-Identifier: Apache-2.0 */
#define STR_EXPAND(x) #x
#define STR(x) STR_EXPAND(x)
#define GIT_REV STR(GIT_REV_)
#define GIT_REV_ \2364d86a-dirty
