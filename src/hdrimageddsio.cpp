

#include "hdrimage.h"
#include <ImathMatrix.h>
#include <spdlog/spdlog.h>


// these pragmas ignore warnings about unused static functions
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-function"
#elif defined(__GNUC__) || defined(__GNUG__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
#elif defined(_MSC_VER)
#pragma warning(push, 0)
#endif


#define TINYDDSLOADER_IMPLEMENTATION
#include "tinyddsloader.h"

#include "half.hpp"
using half_float::half;

#if defined(__clang__)
#pragma clang diagnostic pop
#elif defined(__GNUC__) || defined(__GNUG__)
#pragma GCC diagnostic pop
#elif defined(_MSC_VER)
#pragma warning(pop)
#endif




using Imath::M33f;
using Imath::V2f;
using Imath::V3f;

using std::string;
using std::vector;
using namespace tinyddsloader;

enum Types
{
    Float32,
    Float16,
    SInt8,
    SInt16,
    SInt32,
    UInt8,
    UInt16,
    UInt32,
    SNorm8,
    SNorm16,
    UNorm8,
    UNorm16,
    Typeless,
};


void getChannelsAndBytes(DDSFile::DXGIFormat format, bool &isFloatingPoint, bool &isHalfFloat, int &numChannels, int &bytesPerChannel, Types &type)
{
    isFloatingPoint = (format == DDSFile::DXGIFormat::R32G32B32A32_Float  ||
                       format == DDSFile::DXGIFormat::R32G32B32A32_Float  ||
                       format == DDSFile::DXGIFormat::R32G32B32_Float     ||
                       format == DDSFile::DXGIFormat::R32G32_Float        ||
                       format == DDSFile::DXGIFormat::R32_Float           ||
                       format == DDSFile::DXGIFormat::D32_Float           ||
                       format == DDSFile::DXGIFormat::R16G16B16A16_Float  ||
                       format == DDSFile::DXGIFormat::R16G16_Float        ||
                       format == DDSFile::DXGIFormat::R16_Float           ||
                       format == DDSFile::DXGIFormat::R11G11B10_Float);


    isHalfFloat = (format == DDSFile::DXGIFormat::R16G16B16A16_Float ||
                   format == DDSFile::DXGIFormat::R16G16_Float       ||
                   format == DDSFile::DXGIFormat::R16_Float);

    if(isFloatingPoint && !isHalfFloat)
        type = Types::Float32;

    if(isFloatingPoint && isHalfFloat)
        type = Types::Float16;


   if(format == DDSFile::DXGIFormat::R32_UInt          || 
      format == DDSFile::DXGIFormat::R32G32_UInt       || 
      format == DDSFile::DXGIFormat::R32G32B32_UInt    || 
      format == DDSFile::DXGIFormat::R32G32B32A32_UInt)
    {
        type = Types::UInt32;
    }

   if(format == DDSFile::DXGIFormat::R16_UInt          ||  
      format == DDSFile::DXGIFormat::R16G16_UInt       ||
      format == DDSFile::DXGIFormat::R16G16B16A16_UInt)
    {
        type = Types::UInt16;
    }

   if(format == DDSFile::DXGIFormat::R8_UInt           ||   
      format == DDSFile::DXGIFormat::R8G8_UInt         ||
      format == DDSFile::DXGIFormat::R8G8B8A8_UInt)
    {
        type = Types::UInt8;
    }

    if(format == DDSFile::DXGIFormat::R32_SInt          || 
       format == DDSFile::DXGIFormat::R32G32_SInt       || 
       format == DDSFile::DXGIFormat::R32G32B32_SInt    ||  
       format == DDSFile::DXGIFormat::R32G32B32A32_SInt)
    {
        type = Types::SInt32;
    }

    if(format == DDSFile::DXGIFormat::R16_SInt          ||  
       format == DDSFile::DXGIFormat::R16G16_SInt       ||
       format == DDSFile::DXGIFormat::R16G16B16A16_SInt)
    {
        type = Types::SInt16;
    }

    if(format == DDSFile::DXGIFormat::R8_SInt           ||
       format == DDSFile::DXGIFormat::R8G8_SInt         || 
       format == DDSFile::DXGIFormat::R8G8B8A8_SInt)
    {
        type = Types::SInt8;
    }

    if(format == DDSFile::DXGIFormat::R16_SNorm          || 
       format == DDSFile::DXGIFormat::R16G16_SNorm       ||
       format == DDSFile::DXGIFormat::R16G16B16A16_SNorm)
    {
        type = Types::SNorm16;
    }

    if(format == DDSFile::DXGIFormat::R8_SNorm           ||  
       format == DDSFile::DXGIFormat::R8G8_SNorm         ||
       format == DDSFile::DXGIFormat::R8G8B8A8_SNorm)
    {
        type = Types::SNorm8;
    }

    if(format == DDSFile::DXGIFormat::D16_UNorm           || 
       format == DDSFile::DXGIFormat::R16_UNorm           || 
       format == DDSFile::DXGIFormat::R16G16_UNorm        || 
       format == DDSFile::DXGIFormat::R16G16B16A16_UNorm)
    {
        type = Types::UNorm16;
    }

    if(format == DDSFile::DXGIFormat::R8_UNorm            ||  
       format == DDSFile::DXGIFormat::R8G8_UNorm          ||
       format == DDSFile::DXGIFormat::B8G8R8X8_UNorm      ||
       format == DDSFile::DXGIFormat::B8G8R8A8_UNorm      ||
       format == DDSFile::DXGIFormat::B8G8R8A8_UNorm_SRGB ||
       format == DDSFile::DXGIFormat::R8G8B8A8_UNorm)
    {
        type = Types::UNorm8;
    }


    numChannels = 0;

    if(format == DDSFile::DXGIFormat::R32_Float || 
       format == DDSFile::DXGIFormat::D32_Float || 
       format == DDSFile::DXGIFormat::R32_UInt  || 
       format == DDSFile::DXGIFormat::R32_SInt  || 
       format == DDSFile::DXGIFormat::R16_Float ||
       format == DDSFile::DXGIFormat::D16_UNorm || 
       format == DDSFile::DXGIFormat::R16_UNorm || 
       format == DDSFile::DXGIFormat::R16_UInt  ||  
       format == DDSFile::DXGIFormat::R16_SNorm || 
       format == DDSFile::DXGIFormat::R16_SInt  ||  
       format == DDSFile::DXGIFormat::R8_UNorm  ||  
       format == DDSFile::DXGIFormat::R8_UInt   ||   
       format == DDSFile::DXGIFormat::R8_SNorm  ||  
       format == DDSFile::DXGIFormat::R8_SInt)  
    {
        numChannels = 1;
    }
    else if(format == DDSFile::DXGIFormat::R32G32_Float || 
            format == DDSFile::DXGIFormat::R32G32_UInt  || 
            format == DDSFile::DXGIFormat::R32G32_SInt  || 
            format == DDSFile::DXGIFormat::R16G16_Float || 
            format == DDSFile::DXGIFormat::R16G16_UNorm || 
            format == DDSFile::DXGIFormat::R16G16_UInt  ||
            format == DDSFile::DXGIFormat::R16G16_SNorm ||
            format == DDSFile::DXGIFormat::R16G16_SInt  ||
            format == DDSFile::DXGIFormat::R8G8_UNorm   ||
            format == DDSFile::DXGIFormat::R8G8_UInt    ||
            format == DDSFile::DXGIFormat::R8G8_SNorm   ||
            format == DDSFile::DXGIFormat::R8G8_SInt)
    {
        numChannels = 2;
    }
    else if(format == DDSFile::DXGIFormat::R32G32B32_Float || 
            format == DDSFile::DXGIFormat::R32G32B32_UInt  || 
            format == DDSFile::DXGIFormat::R32G32B32_SInt  || 
            format == DDSFile::DXGIFormat::R11G11B10_Float)
    {
        numChannels = 3;
    }
    else if(format == DDSFile::DXGIFormat::R32G32B32A32_Float  || 
            format == DDSFile::DXGIFormat::R32G32B32A32_UInt   || 
            format == DDSFile::DXGIFormat::R32G32B32A32_SInt   || 
            format == DDSFile::DXGIFormat::R16G16B16A16_Float  || 
            format == DDSFile::DXGIFormat::R16G16B16A16_UNorm  || 
            format == DDSFile::DXGIFormat::R16G16B16A16_UInt   ||
            format == DDSFile::DXGIFormat::R16G16B16A16_SNorm  ||
            format == DDSFile::DXGIFormat::R16G16B16A16_SInt   ||
            format == DDSFile::DXGIFormat::R10G10B10A2_UNorm   ||
            format == DDSFile::DXGIFormat::R10G10B10A2_UInt    ||
            format == DDSFile::DXGIFormat::B8G8R8X8_UNorm      ||
            format == DDSFile::DXGIFormat::B8G8R8A8_UNorm      ||
            format == DDSFile::DXGIFormat::B8G8R8A8_UNorm_SRGB ||
            format == DDSFile::DXGIFormat::R8G8B8A8_UNorm      ||
            format == DDSFile::DXGIFormat::R8G8B8A8_UInt       ||
            format == DDSFile::DXGIFormat::R8G8B8A8_SNorm      ||
            format == DDSFile::DXGIFormat::R8G8B8A8_SInt)
    {
        numChannels = 4;
    }

    bytesPerChannel = 0;
    if(isFloatingPoint && isHalfFloat)
    {
        bytesPerChannel = 2;
    }
    else if (isFloatingPoint && !isHalfFloat)
    {
        bytesPerChannel = 4;
    }
    else 
    {
        bytesPerChannel = 1;
    }
}

template<typename imageType>
void convert(HDRImage &img,
             const DDSFile::ImageData *data,
             const int numChannels)
{
    uint32_t width  = data->m_width;
    uint32_t height = data->m_height;

    img.resize(width, height);

    const imageType *ddsMem = (const imageType *)data->m_mem;

#ifdef NDEBUG
    parallel_for(0, height,
                 [&image, width, numChannels, ddsMem](uint32_t y)
#else
                for (uint32_t y = 0; y < height; y++)
#endif
                 {
                     for (uint32_t x = 0; x < width; x++)
                     {
                        Color4 color(0.0f, 1.0f);
                        // Channel Loop
                        for(int c = 0; c < 4; c++)
                        {
                            if(c < numChannels)
                            {
                                uint32_t inputIndex = numChannels * (x + y * width) + c;

                                float fval = static_cast<float>(ddsMem[inputIndex]);

                                color[c] = fval;
                            }
                        }
                        img(x, y) = color;
                     }
                 }
#ifdef NDEBUG                 
                 );
#endif
}


template<>
void convert<uint8_t>(HDRImage &img,
                      const DDSFile::ImageData *data,
                      const int numChannels)
{
    uint32_t width  = data->m_width;
    uint32_t height = data->m_height;
 

    const uint8_t *ddsMem = (const uint8_t *)data->m_mem;

#if defined(NDEBUG)
    parallel_for(0, height,
                 [&image, width, numChannels, ddsMem](uint32_t y)
#else
                for (uint32_t y = 0; y < height; y++)
#endif
                 {
                     for (uint32_t x = 0; x < width; x++)
                     {
                        Color4 color(0.f, 1.f);
                        // Channel Loop
                        for(int c = 0; c < 4; c++)
                        {
                            if(c < numChannels)
                            {
                                uint32_t inputIndex = numChannels * (x + y * width) + c;
 
                                uint8_t uval = ddsMem[inputIndex];
                                float fval = static_cast<float>(uval) / 255.0f;

                                color[c] = fval;
                            }
                        }
                        img(x, y) = color;
                     }
                 }
#ifdef NDEBUG                 
                 );
#endif
}

template<>
void convert<int8_t>(HDRImage &img,
                      const DDSFile::ImageData *data,
                      const int numChannels)
{
    uint32_t width  = data->m_width;
    uint32_t height = data->m_height;
 
    const int8_t *ddsMem = (const int8_t *)data->m_mem;

#if defined(NDEBUG)
    parallel_for(0, height,
                 [&image, width, numChannels, ddsMem](uint32_t y)
#else
                for (uint32_t y = 0; y < height; y++)
#endif
                 {
                     for (uint32_t x = 0; x < width; x++)
                     {
                        Color4 color(0.f, 1.f);
                        // Channel Loop
                        for(int c = 0; c < 4; c++)
                        {
                            if(c < numChannels)
                            {
                                uint32_t inputIndex = numChannels * (x + y * width) + c;
 
                                int8_t ival = ddsMem[inputIndex];
                                float fval = static_cast<float>(ival + 128) / 255.0f;   // to range [0,+1]

                                fval = fval * 2.0f - 1.0f;     // to range [-1,+1]

                                color[c] = fval;
                            }
                        }
                        img(x, y) = color;
                     }
                 }
#ifdef NDEBUG                 
                 );
#endif
}



void HDRImage::load_dds(const string &filename)
{
    DDSFile dds;
    Result ret = dds.Load(filename.c_str());
    if (ret != tinyddsloader::Result::Success)
        throw std::runtime_error("Failed to load DDS.");

    bool  isFloatingPoint;
    bool  isHalfFloat;
    int   numChannels;
    int   bytesPerChannel;
    Types type; 

    getChannelsAndBytes(dds.GetFormat(), isFloatingPoint, isHalfFloat, numChannels, bytesPerChannel, type);

    if(numChannels == 0 || bytesPerChannel == 0)
        throw std::runtime_error("Don't know how to load those channels.");

    const DDSFile::ImageData *data = dds.GetImageData();

    // Resize internal HDRImage
    resize(dds.GetWidth(), dds.GetHeight());

    if(type == Types::Float32)
        convert<float>(*this, data, numChannels);
    else if (type == Types::Float16)
        convert<half>(*this, data, numChannels);
    else if (type == Types::SInt32)
        convert<int32_t>(*this, data, numChannels);
    else if (type == Types::SInt16)
        convert<int16_t>(*this, data, numChannels);
    else if (type == Types::SInt8)
        convert<int8_t>(*this, data, numChannels);
    else if (type == Types::UInt32)
        convert<uint32_t>(*this, data, numChannels);
    else if (type == Types::UInt16)
        convert<uint16_t>(*this, data, numChannels);
    else if (type == Types::UInt8)
        convert<uint8_t>(*this, data, numChannels);
    else if (type == Types::SNorm16)
        convert<int16_t>(*this, data, numChannels);
    else if (type == Types::SNorm8)
        convert<int8_t>(*this, data, numChannels);
    else if (type == Types::UNorm16)
        convert<uint16_t>(*this, data, numChannels);
    else if (type == Types::UNorm8)
        convert<uint8_t>(*this, data, numChannels);

}



void printImageInfo(const DDSFile &dds)
{
    spdlog::debug("width = {}.",  dds.GetWidth());
    spdlog::debug("height = {}.", dds.GetHeight());
    spdlog::debug("depth = {}.",  dds.GetDepth());
    spdlog::debug("mipCount = {}.", dds.GetMipCount());
    spdlog::debug("arraySize = {}.", dds.GetArraySize());
    spdlog::debug("bits per pixel = {}.", dds.GetBitsPerPixel(dds.GetFormat()));
}