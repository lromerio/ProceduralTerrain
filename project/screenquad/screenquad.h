#pragma once
#include "icg_helper.h"
#include "config.h"

#include "array"

class ScreenQuad {

    private:
        GLuint vertex_array_id_;        // vertex array object
        GLuint program_id_;             // GLSL shader program ID
        GLuint vertex_buffer_object_;   // memory buffer

        float screenquad_width_;
        float screenquad_height_;

        glm::vec2 center = INITIAL_CENTER;

        int scaleFactor = INITIAL_SCALE;
        float H = INITIAL_H;
        float lacunarity = INITIAL_LACUNARITY;
        int octaves = INITIAL_OCTAVES;
        float cutoff_coef = INITIAL_CUT_COEFF;
        float offset = INITIAL_OFFSET;

    public:
        // Perlin noise parameters
        void setScaleFactor(int newValue) {
            scaleFactor = newValue;
        }
        void setH(int newValue) {
            H = newValue;
        }
        void setLacunarity(int newValue) {
            lacunarity = newValue;
        }
        void setOctaves(int newValue) {
            octaves = newValue;
        }
        void setCutoffCoef(int newValue) {
            cutoff_coef = newValue;
        }
        void setOffset(int newValue) {
            offset = newValue;
        }

        void Init(float screenquad_width, float screenquad_height) {

            // set screenquad size
            this->screenquad_width_ = screenquad_width;
            this->screenquad_height_ = screenquad_height;

            // initialize center
            this->center = center;

            // compile the shaders
            program_id_ = icg_helper::LoadShaders("screenquad_vshader.glsl",
                                                  "screenquad_fshader.glsl",
                                                  NULL,
                                                  NULL);
            if(!program_id_) {
                exit(EXIT_FAILURE);
            }

            glUseProgram(program_id_);

            // vertex one vertex Array
            glGenVertexArrays(1, &vertex_array_id_);
            glBindVertexArray(vertex_array_id_);

            // vertex coordinates
            {
                const GLfloat vertex_point[] = { /*V1*/ -1.0f, -1.0f, 0.0f,
                                                 /*V2*/ +1.0f, -1.0f, 0.0f,
                                                 /*V3*/ -1.0f, +1.0f, 0.0f,
                                                 /*V4*/ +1.0f, +1.0f, 0.0f};
                // buffer
                glGenBuffers(1, &vertex_buffer_object_);
                glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_object_);
                glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_point),
                             vertex_point, GL_STATIC_DRAW);

                // attribute
                GLuint vertex_point_id = glGetAttribLocation(program_id_, "vpoint");
                glEnableVertexAttribArray(vertex_point_id);
                glVertexAttribPointer(vertex_point_id, 3, GL_FLOAT, DONT_NORMALIZE,
                                      ZERO_STRIDE, ZERO_BUFFER_OFFSET);
            }

            // texture coordinates
            {
                const GLfloat vertex_texture_coordinates[] = { /*V1*/ 0.0f, 0.0f,
                                                               /*V2*/ 1.0f, 0.0f,
                                                               /*V3*/ 0.0f, 1.0f,
                                                               /*V4*/ 1.0f, 1.0f};

                // buffer
                glGenBuffers(1, &vertex_buffer_object_);
                glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_object_);
                glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_texture_coordinates),
                             vertex_texture_coordinates, GL_STATIC_DRAW);

                // attribute
                GLuint vertex_texture_coord_id = glGetAttribLocation(program_id_,
                                                                     "vtexcoord");
                glEnableVertexAttribArray(vertex_texture_coord_id);
                glVertexAttribPointer(vertex_texture_coord_id, 2, GL_FLOAT,
                                      DONT_NORMALIZE, ZERO_STRIDE,
                                      ZERO_BUFFER_OFFSET);
            }

            // pass perlin permutation as uniform
            glUniform1iv(glGetUniformLocation(program_id_, "permutation"), 256, &perlinPermutation[0]);

            // pass terrain size as uniform
            GLuint world_size_id = glGetUniformLocation(program_id_, "world_size");
            glUniform1f(world_size_id, WORLD_SIZE);

            // to avoid the current object being polluted
            glBindVertexArray(0);
            glUseProgram(0);
        }

        void setCenter(glm::vec2 newCenter) {
            center = newCenter;
        }

        void UpdateSize(int screenquad_width, int screenquad_height) {
            this->screenquad_width_ = screenquad_width;
            this->screenquad_height_ = screenquad_height;
        }

        void Cleanup() {
            glBindVertexArray(0);
            glUseProgram(0);
            glDeleteBuffers(1, &vertex_buffer_object_);
            glDeleteProgram(program_id_);
            glDeleteVertexArrays(1, &vertex_array_id_);
        }

        void Draw() {
            glUseProgram(program_id_);
            glBindVertexArray(vertex_array_id_);

            // Perlin noise parameters
            GLuint scaleFactor_id = glGetUniformLocation(program_id_, "scaleFactor");
            glUniform1i(scaleFactor_id, scaleFactor);

            GLuint octaves_id = glGetUniformLocation(program_id_, "octaves");
            glUniform1i(octaves_id, octaves);

            GLuint H_id = glGetUniformLocation(program_id_, "H");
            glUniform1f(H_id, H);

            GLuint lacunarity_id = glGetUniformLocation(program_id_, "lacunarity");
            glUniform1f(lacunarity_id, lacunarity);

            GLuint cutoff_coef_id = glGetUniformLocation(program_id_, "cutoff_coef");
            glUniform1f(cutoff_coef_id, cutoff_coef);

            GLuint offset_id = glGetUniformLocation(program_id_, "offset");
            glUniform1f(offset_id, offset);

            // pass terrain coord to shader
            GLuint center_id = glGetUniformLocation(program_id_, "center");
            glUniform2fv(center_id, 1, &center[0]);

            // draw
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

            glBindVertexArray(0);
            glUseProgram(0);
        }
};
