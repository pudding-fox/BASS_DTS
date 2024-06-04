/*
 * COPYRIGHT:        See COPYING in the top level directory
 * PROJECT:          ReactOS CRT
 * FILE:             lib/sdk/crt/math/cos.c
 * PURPOSE:          Generic C Implementation of cos
 * PROGRAMMER:       Timo Kreuzer (timo.kreuzer@reactos.org)
 */

#ifdef _MSC_VER
#pragma warning(suppress:4164) /* intrinsic not declared */
#pragma function(cos)
#endif /* _MSC_VER */

double
cos(double x);