//
// Created by Wojciech Jarosz on 9/14/17.
//

#pragma once

#include "fwd.h"
#include <string>
#include <vector>

/*!
 * @brief		Generic color space conversion
 *
 * Converts from a source color space \a src to a destination color space \a dst,
 * each specified using the \a EColorSpace enumeriation
 *
 * @param dst 	Destination color space
 * @param a 	First component of the destination color
 * @param b 	Second component of the destination color
 * @param c 	Third component of the destination color
 * @param src 	Source color space
 * @param A  	First component of the source color
 * @param B  	Second component of the source color
 * @param C  	Third component of the source color
 */
void convertColorSpace(EColorSpace dst, float *a, float *b, float *c, EColorSpace src, float A, float B, float C);

// to/from linear to sRGB and AdobeRGB
float SRGBToLinear(float a);
void SRGBToLinear(float * r, float * g, float * b);
Color3 SRGBToLinear(const Color3 &c);
Color4 SRGBToLinear(const Color4 &c);
float LinearToSRGB(float a);
void LinearToSRGB(float * r, float * g, float * b);
Color3 LinearToSRGB(const Color3 &c);
Color4 LinearToSRGB(const Color4 &c);
float AdobeRGBToLinear(float a);
void AdobeRGBToLinear(float * r, float * g, float * b);
Color3 AdobeRGBToLinear(const Color3 &c);
Color4 AdobeRGBToLinear(const Color4 &c);
float LinearToAdobeRGB(float a);
void LinearToAdobeRGB(float * r, float * g, float * b);
Color3 LinearToAdobeRGB(const Color3 &c);
Color4 LinearToAdobeRGB(const Color4 &c);

// to and from XYZ
void XYZToLinearSRGB(float *R, float *G, float *B, float X, float Y, float z);
void LinearSRGBToXYZ(float *X, float *Y, float *Z, float R, float G, float B);
void XYZToLinearAdobeRGB(float *R, float *G, float *B, float X, float Y, float z);
void LinearAdobeRGBToXYZ(float *X, float *Y, float *Z, float R, float G, float B);
void XYZToLab(float *L, float *a, float *b, float X, float Y, float Z);
void LabToXYZ(float *X, float *Y, float *Z, float L, float a, float b);
void XYZToLuv(float *L, float *u, float *v, float X, float Y, float Z);
void LuvToXYZ(float *X, float *Y, float *Z, float L, float u, float v);
void XYZToxy(float *x, float *y, float X, float Y, float Z);
void xyYToXZ(float *X, float *Z, float x, float y, float Y);
void XYZToHSL(float *H, float *L, float *S, float X, float Y, float Z);
void HSLToXYZ(float *X, float *Y, float *Z, float H, float L, float S);
void XYZToHSV(float *H, float *S, float *V, float X, float Y, float Z);
void HSVToXYZ(float *X, float *Y, float *Z, float H, float S, float V);

//! Normalize the L,a,b values to each fall within the range [0,1]
void normalizeLab(float * L, float * a, float * b);
//! Take normalized L,a,b values and undo the normalization back to the original range
void unnormalizeLab(float * L, float * a, float * b);

// HLS and HSV are more naturally defined as transformations to/from RGB, so
// define those explicitly
void RGBToHSL(float *H, float *L, float *S, float R, float G, float B);
void HSLToRGB(float *R, float *G, float *B, float H, float L, float S);
void RGBToHSV(float *H, float *S, float *V, float R, float G, float B);
void HSVToRGB(float *R, float *G, float *B, float H, float S, float V);




const std::vector<std::string> & colorSpaceNames();