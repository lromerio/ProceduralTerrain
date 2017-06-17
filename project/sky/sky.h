#pragma once
#include "icg_helper.h"
#include "config.h"
#include "glm/gtc/type_ptr.hpp"

static const int half_size = WORLD_SIZE/2;
static const unsigned int NbCubeVertices = 36;
static const glm::vec3 CubeVertices[] =
{
    glm::vec3(-half_size, -half_size, -half_size),
    glm::vec3(-half_size, half_size, -half_size),
    glm::vec3(half_size, -half_size, -half_size),
    glm::vec3(-half_size, half_size, -half_size),
    glm::vec3(half_size, -half_size, -half_size),
    glm::vec3(half_size, half_size, -half_size),
    glm::vec3(half_size, half_size, half_size),
    glm::vec3(half_size, -half_size, half_size),
    glm::vec3(half_size, half_size, -half_size),
    glm::vec3(half_size, -half_size, half_size),
    glm::vec3(half_size, half_size, -half_size),
    glm::vec3(half_size, -half_size, -half_size),
    glm::vec3(half_size, half_size, half_size),
    glm::vec3(-half_size, half_size, half_size),
    glm::vec3(half_size, -half_size, half_size),
    glm::vec3(-half_size, half_size, half_size),
    glm::vec3(half_size, -half_size, half_size),
    glm::vec3(-half_size, -half_size, half_size),
    glm::vec3(-half_size, -half_size, half_size),
    glm::vec3(-half_size, -half_size, -half_size),
    glm::vec3(half_size, -half_size, half_size),
    glm::vec3(-half_size, -half_size, -half_size),
    glm::vec3(half_size, -half_size, half_size),
    glm::vec3(half_size, -half_size, -half_size),
    glm::vec3(-half_size, half_size, -half_size),
    glm::vec3(-half_size, -half_size, -half_size),
    glm::vec3(-half_size, half_size, half_size),
    glm::vec3(-half_size, -half_size, -half_size),
    glm::vec3(-half_size, half_size, half_size),
    glm::vec3(-half_size, -half_size, half_size),
    glm::vec3(half_size, half_size, -half_size),
    glm::vec3(-half_size, half_size, -half_size),
    glm::vec3(half_size, half_size, half_size),
    glm::vec3(-half_size, half_size, -half_size),
    glm::vec3(half_size, half_size, half_size),
    glm::vec3(-half_size, half_size, half_size)
};

static const unsigned int NbCubeUVs = 36;
static const glm::vec2 CubeUVs[] =
{
    glm::vec2(0.333, 0.75),
    glm::vec2(0.666, 0.75),
    glm::vec2(0.333, 0.5),
    glm::vec2(0.666, 0.75),
    glm::vec2(0.333, 0.5),
    glm::vec2(0.666, 0.5),

    glm::vec2(0.666, 0.25),
    glm::vec2(0.333, 0.25),
    glm::vec2(0.666, 0.5),
    glm::vec2(0.333, 0.25),
    glm::vec2(0.666, 0.5),
    glm::vec2(0.333, 0.5),

    glm::vec2(0.666, 0.25),
    glm::vec2(0.666, 0.0),
    glm::vec2(0.333, 0.25),
    glm::vec2(0.666, 0.0),
    glm::vec2(0.333, 0.25),
    glm::vec2(0.333, 0.0),

    glm::vec2(0.0, 0.75),
    glm::vec2(0.333, 0.75),
    glm::vec2(0.0, 0.5),
    glm::vec2(0.333, 0.75),
    glm::vec2(0.0, 0.5),
    glm::vec2(0.333, 0.5),

    glm::vec2(0.666, 0.75),
    glm::vec2(0.333, 0.75),
    glm::vec2(0.666, 1.0),
    glm::vec2(0.333, 0.75),
    glm::vec2(0.666, 1.0),
    glm::vec2(0.333, 1.0),

    glm::vec2(0.666, 0.5),
    glm::vec2(0.666, 0.75),
    glm::vec2(1.0, 0.5),
    glm::vec2(0.666, 0.75),
    glm::vec2(1.0, 0.5),
    glm::vec2(1.0, 0.75)
};

class Sky {

    private:
        GLuint vertex_array_id_;        // vertex array object
        GLuint program_id_;             // GLSL shader program ID
        GLuint vertex_buffer_object_;   // memory buffer
        GLuint texture_id_;             // texture ID

    public:
        void Init() {
            // compile the shaders.
            program_id_ = icg_helper::LoadShaders("sky_vshader.glsl",
                                                  "sky_fshader.glsl",
                                                  NULL,
                                                  NULL);
            if(!program_id_) {
                exit(EXIT_FAILURE);
            }

            glUseProgram(program_id_);

            // vertex one vertex array
            glGenVertexArrays(1, &vertex_array_id_);
            glBindVertexArray(vertex_array_id_);

            // vertex coordinates
            {
                // buffer
                glGenBuffers(1, &vertex_buffer_object_);
                glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_object_);
                glBufferData(GL_ARRAY_BUFFER, NbCubeVertices * sizeof(glm::vec3),
                             &CubeVertices[0], GL_STATIC_DRAW);

                // attribute
                GLuint vertex_point_id = glGetAttribLocation(program_id_, "vpoint");
                glEnableVertexAttribArray(vertex_point_id);
                glVertexAttribPointer(vertex_point_id, 3, GL_FLOAT, DONT_NORMALIZE,
                                      ZERO_STRIDE, ZERO_BUFFER_OFFSET);
            }


            // texture coordinates
            {
                // buffer
                glGenBuffers(1, &vertex_buffer_object_);
                glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_object_);
                glBufferData(GL_ARRAY_BUFFER, NbCubeUVs * sizeof(glm::vec2),
                             &CubeUVs[0], GL_STATIC_DRAW);

                // attribute
                GLuint vertex_texture_coord_id = glGetAttribLocation(program_id_,
                                                                     "vtexcoord");
                glEnableVertexAttribArray(vertex_texture_coord_id);
                glVertexAttribPointer(vertex_texture_coord_id, 2, GL_FLOAT,
                                      DONT_NORMALIZE,
                                      ZERO_STRIDE, ZERO_BUFFER_OFFSET);
            }


            // load texture
            initTexture("sky1c.tga", &texture_id_, "cubemap", GL_TEXTURE0);

            // to avoid the current object being polluted
            glBindVertexArray(0);
            glUseProgram(0);
        }

        void Cleanup() {
            glBindVertexArray(0);
            glUseProgram(0);
            glDeleteBuffers(1, &vertex_buffer_object_);
            glDeleteProgram(program_id_);
            glDeleteVertexArrays(1, &vertex_array_id_);
            glDeleteTextures(1, &texture_id_);
        }

        void Draw(const glm::mat4& model, const glm::mat4& view,const glm::mat4& projection, bool clip, float lightAngle) {

            // Compute MVP matrix
            glm::mat4 MVP = projection * view * model;

            glUseProgram(program_id_);
            glBindVertexArray(vertex_array_id_);

            // Day/night
            GLuint lightAngle_id = glGetUniformLocation(program_id_, "lightAngle");
            glUniform1f(lightAngle_id, lightAngle);


            // bind textures
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, texture_id_);

            // pass clip
            GLuint clip_id = glGetUniformLocation(program_id_, "clip");
            glUniform1i(clip_id, clip);
            
            // setup MVP
            GLuint MVP_id = glGetUniformLocation(program_id_, "MVP");
            glUniformMatrix4fv(MVP_id, 1, GL_FALSE, value_ptr(MVP));
            
            // draw
            glDrawArrays(GL_TRIANGLES,0,NbCubeVertices);

            glBindVertexArray(0);
            glUseProgram(0);
        }

        void initTexture(string filename, GLuint *texture_id, string texture_name, int val) {

                int width;
                int height;
                int nb_component;
                // set stb_image to have the same coordinates as OpenGL
                stbi_set_flip_vertically_on_load(1);
                unsigned char *image = stbi_load(filename.c_str(), &width, &height, &nb_component, 0);

                if (image == nullptr) {
                    throw(string("Failed to load texture"));
                }

                glGenTextures(1, texture_id);
                glBindTexture(GL_TEXTURE_2D, *texture_id);
                glGenerateMipmap(GL_TEXTURE_2D);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

                if (nb_component == 3) {
                    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
                } else if (nb_component == 4) {
                    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
                }

                GLuint tex_id = glGetUniformLocation(program_id_, texture_name.c_str());
                //glUseProgram(program_id_);
                glUniform1i(tex_id, val - GL_TEXTURE0);

                // cleanup
                glBindTexture(GL_TEXTURE_2D, val);
                stbi_image_free(image);
            }
};
