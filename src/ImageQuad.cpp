#include "ImageQuad.h"
#include <iostream>     // std::cout, std::fixed
#include <iomanip>      // std::setprecision
#include <algorithm>    // std::transform
#include <exception>    // std::transform

#define TINYEXR_IMPLEMENTATION
#include "tinyexr.h"
// #define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include "dither-matrix256.h"


using namespace nanogui;
using namespace Eigen;
using namespace std;


ImageQuad::ImageQuad() :
    m_filename()
{
    // empty
}


ImageQuad::~ImageQuad()
{
    clear();
}


void ImageQuad::init()
{
    m_shader = new GLShader();
    /* Simple gamma tonemapper as a GLSL shader */
    m_shader->init(
        "Tonemapper",

        /* Vertex shader */
        "#version 330\n"
        "uniform mat4 modelViewProj;\n"
        "in vec2 position;\n"
        "out vec2 uv;\n"
        "void main() {\n"
        "    gl_Position = modelViewProj * vec4(position.x, position.y, 0.0, 1.0);\n"
        "    uv = vec2((position.x+1)/2, (-position.y+1)/2);\n"
        "}",

        /* Fragment shader */
        "#version 330\n"
        "uniform sampler2D source;\n"
        "uniform sampler2D dither_texture;\n"
        "uniform bool dither;\n"
        "uniform float gain;\n"
        "uniform vec3 channels;\n"
        "uniform float gamma;\n"
        "uniform bool sRGB;\n"
        "in vec2 uv;\n"
        "out vec4 out_color;\n"
        "in vec4 gl_FragCoord;\n"
        "float toSRGB(float value) {\n"
        "    if (value < 0.0031308)\n"
        "        return 12.92 * value;\n"
        "    return 1.055 * pow(value, 0.41666) - 0.055;\n"
        "}\n"
        "void main() {\n"
        "    vec4 color = texture(source, uv);\n"
        "    color *= gain / color.w;\n"
        "    if (sRGB)\n"
        "       out_color.rgb = vec3(toSRGB(color.r), toSRGB(color.g), toSRGB(color.b));\n"
        "    else\n"
        "       out_color.rgb = pow(color.rgb, vec3(1.0/gamma));\n"
        "    float dith = texture(dither_texture, gl_FragCoord.xy/vec2(256,256)).r/65536 - 0.5;\n"
        "    out_color.rgb += dither ? vec3(dith/255.0) : vec3(0.0);\n"
        "    out_color.rgb *= channels;\n"
        "    out_color.w = 1.0;\n"
        "}"
    );

    MatrixXu indices(3, 2); /* Draw 2 triangles */
    indices.col(0) << 0, 1, 2;
    indices.col(1) << 2, 3, 0;

    MatrixXf positions(2, 4);
    positions.col(0) << -1, -1;
    positions.col(1) << 1, -1;
    positions.col(2) << 1, 1;
    positions.col(3) << -1, 1;

    m_shader->bind();
    m_shader->uploadIndices(indices);
    m_shader->uploadAttrib("position", positions);

    /* Allocate texture memory for the rendered image */
    glGenTextures(1, &m_texture);
    glBindTexture(GL_TEXTURE_2D, m_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    glPixelStorei(GL_UNPACK_ROW_LENGTH, width());
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width(), height(),
                 0, GL_RGBA, GL_FLOAT, (uint8_t *) m_image);
}


void ImageQuad::clear()
{
    if (m_texture)
        glDeleteTextures(1, &m_texture);

    if (m_shader)
        m_shader->free();

    delete m_shader;
    m_shader = 0;
    
    if (m_image)
    {
        delete [] m_image;
        m_image = nullptr;
        m_size = Vector2i::Zero();
    }
}


void ImageQuad::draw(const Matrix4f & mvp,
                     float gain, float gamma,
                     bool sRGB, bool dither,
                     const Vector3f & channels)
{
    if (m_shader)
    {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_texture);

        m_shader->bind();
        m_shader->setUniform("gain", gain);
        m_shader->setUniform("gamma", gamma);
        m_shader->setUniform("channels", channels);
        m_shader->setUniform("source", 0);
        m_shader->setUniform("dither_texture", 1);
        m_shader->setUniform("dither", dither);
        m_shader->setUniform("sRGB", sRGB);
        m_shader->setUniform("modelViewProj", mvp);
        m_shader->drawIndexed(GL_TRIANGLES, 0, 2);
    }
}


bool ImageQuad::load(const string & filename)
{
    clear();

    m_filename = filename;

    // try PNG, JPG, HDR, etc files first
    int n;
    m_image = stbi_loadf(m_filename.c_str(), &m_size.x(), &m_size.y(), &n, 4);
    if (m_image)
        return true;

    // now try EXR
    const char* err;
    if (LoadEXR(&m_image, &m_size.x(), &m_size.y(), m_filename.c_str(), &err))
    {
        cout << "ERROR!" << endl;
        fprintf(stderr, "Parse EXR err: %s\n", err);
        
        m_image = nullptr;
        m_size = Vector2i::Zero();

        return false;
    }

    
    cout << m_size << endl;
    return true;
}

Color ImageQuad::pixel(int x, int y) const
{
    return Color(m_image[4*x + 4*m_size[0] * y + 0],
                 m_image[4*x + 4*m_size[0] * y + 1],
                 m_image[4*x + 4*m_size[0] * y + 2],
                 m_image[4*x + 4*m_size[0] * y + 3]);
}


static string getFileExtension(const string& filename)
{
    if (filename.find_last_of(".") != string::npos)
        return filename.substr(filename.find_last_of(".")+1);
    return "";
}

static float toSRGB(float value)
{
    if (value < 0.0031308f)
       return 12.92f * value;
    return 1.055f * pow(value, 0.41666f) - 0.055f;
}

bool ImageQuad::save(const string & filename,
                     float gain, float gamma,
                     bool sRGB, bool dither)
{
    string extension = getFileExtension(filename);

    transform(extension.begin(),
              extension.end(),
              extension.begin(),
              ::tolower);

    if (extension == "hdr")
        return stbi_write_hdr(filename.c_str(), m_size[0], m_size[1], 4, m_image);
    else
    {
        vector<unsigned char> data(m_size[0]*m_size[1]*3, 0);

        float invGamma = 1.0f/gamma;
        for (int y = 0; y < m_size[1]; ++y)
            for (int x = 0; x < m_size[0]; ++x)
            {
                Color c = pixel(x,y);
                c *= gain;
                if (sRGB)
                   c = Color(toSRGB(c[0]), toSRGB(c[1]), toSRGB(c[2]), c[3]);
                else
                   c = Color(pow(c[0], invGamma), pow(c[1], invGamma), pow(c[2], invGamma), c[3]);

                if (dither)
                {
                    int xmod = x % 256;
                    int ymod = y % 256;
                    float ditherValue = (dither_matrix256[xmod + ymod * 256]/65536.0f - 0.5f)/255.0f;
                    c += Color(ditherValue, 0.0f);
                }
                
                data[3*x + 3*y*m_size[0] + 0] = (int) min(max(c[0] * 255, 0.0f), 255.0f);
                data[3*x + 3*y*m_size[0] + 1] = (int) min(max(c[1] * 255, 0.0f), 255.0f);
                data[3*x + 3*y*m_size[0] + 2] = (int) min(max(c[2] * 255, 0.0f), 255.0f);
            }

        if (extension == "png")
            return stbi_write_png(filename.c_str(), m_size[0], m_size[1],
                                    3, &data[0], sizeof(unsigned char)*m_size[0]*3);
        else if (extension == "bmp")
            return stbi_write_bmp(filename.c_str(), m_size[0], m_size[1], 3, &data[0]);
        else if (extension == "tga")
            return stbi_write_tga(filename.c_str(), m_size[0], m_size[1], 3, &data[0]);
        else
            throw runtime_error("Could not determine desired file type from extension.");
    }
}
