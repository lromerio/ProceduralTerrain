#pragma once
#include "icg_helper.h"
#include "config.h"
#include <glm/gtc/type_ptr.hpp>

class Water {

    private:
        GLuint vertex_array_id_;                // vertex array object
        GLuint vertex_buffer_object_position_;  // memory buffer for positions
        GLuint vertex_buffer_object_index_;     // memory buffer for indices
        GLuint program_id_;
        GLuint num_indices_;                    // number of vertices to render

        // Textures
        GLuint normal_texture_id_;
        GLuint normal_texture2_id_;
        GLuint texture_mirror_id_;

        // Matrix
        GLuint model_id;
        GLuint view_id;
        GLuint projection_id;

        // Time, transparency and reflection
        GLuint time_id;
        GLuint transparency_id;
        GLuint reflection_id;
        GLuint refraction_id;


        // Light
        GLuint lightAngle_id;
        GLuint light_col_id;

        // Waves
        GLuint waveDir_id;
        GLuint waveSpeed_id;
        GLuint alpha_id;
        GLuint center_id;


        // Waves
        float transparency = INITIAL_TRANSPARENCY;
        float reflection = INITIAL_REFLECTION;
        float refraction = INITIAL_REFRACTION;
        float alpha = INITIAL_ALPHA;
        glm::vec2 waveDir = INITIAL_WAVE_DIR;
        float waveSpeed = INITIAL_WAVE_SPEED;

        // important parameters
        int resolution = 1;
        glm::vec2 center = INITIAL_CENTER;

    public:
        void setTransparency(float newValue) {
            transparency = newValue;
        }
        void setReflection(float newValue) {
            reflection = newValue;
        }
        void setWaveDir(glm::vec2 newValue) {
            waveDir = newValue;
        }
        void setWaveSpeed(float newValue) {
            waveSpeed = newValue;
        }
        void setAlpha(float newValue) {
            alpha = newValue;
        }
        void setRefraction(float newValue) {
            refraction = newValue;
        }


        void Init(GLuint tex_mirror) {
            // compile the shaders.
            program_id_ = icg_helper::LoadShaders("water_vshader.glsl",
                                                  "water_fshader.glsl",
                                                  NULL,
                                                  NULL);
            if(!program_id_) {
                exit(EXIT_FAILURE);
            }

            glUseProgram(program_id_);

            // vertex one vertex array
            glGenVertexArrays(1, &vertex_array_id_);
            glBindVertexArray(vertex_array_id_);

            // vertex coordinates and indices
            {
                std::vector<GLfloat> vertices;
                std::vector<GLuint> indices;

                float triangle_size = WORLD_SIZE/RESOLUTION;

                // Vertex position of the triangles
                for (int i = 0; i < RESOLUTION; ++i) {
                    for (int j = 0; j < RESOLUTION; ++j) {
                        vertices.push_back(triangle_size*i - WORLD_SIZE/2);
                        vertices.push_back(triangle_size*j - WORLD_SIZE/2);
                    }
                }

                // Add indices column by column following an 'S' path
                for (int i = 1; i < RESOLUTION; i++) {
                    if(i%2 == 0) {
                        // Add indices while going up
                        for (int j = 0; j < RESOLUTION; j++) {
                            indices.push_back((i-1)*RESOLUTION + j);
                            indices.push_back(i*RESOLUTION + j);
                        }
                    } else {
                        // Add indices while going down
                        for (int j = 0; j < RESOLUTION; j++) {
                            indices.push_back(i*RESOLUTION + (RESOLUTION - 1) - j);
                            indices.push_back((i-1)*RESOLUTION + (RESOLUTION - 1) - j);
                        }
                    }
                }

                num_indices_ = indices.size();

                // position buffer
                glGenBuffers(1, &vertex_buffer_object_position_);
                glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_object_position_);
                glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat),
                             &vertices[0], GL_STATIC_DRAW);

                // vertex indices
                glGenBuffers(1, &vertex_buffer_object_index_);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vertex_buffer_object_index_);
                glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint),
                             &indices[0], GL_STATIC_DRAW);

                // position shader attribute
                GLuint loc_position = glGetAttribLocation(program_id_, "position");
                glEnableVertexAttribArray(loc_position);
                glVertexAttribPointer(loc_position, 2, GL_FLOAT, DONT_NORMALIZE,
                                      ZERO_STRIDE, ZERO_BUFFER_OFFSET);
            }

            // pass real grid size as uniform
            GLuint world_size_id = glGetUniformLocation(program_id_, "world_size");
            glUniform1f(world_size_id, WORLD_SIZE);

            {
                texture_mirror_id_ = tex_mirror;
                glBindTexture(GL_TEXTURE_2D, texture_mirror_id_);
                //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                //GLuint tex_mirror_id = glGetUniformLocation(program_id_, "tex_mirror");
                //glUniform1i(tex_mirror_id, GL_TEXTURE0);

                initTexture("normal_texture_water.tga", &normal_texture_id_, "normal_tex", GL_TEXTURE1);
                initTexture("normal_texture_water2.tga", &normal_texture2_id_, "normal_tex2", GL_TEXTURE2);
            }

            // other uniforms
            getAllUniformLocation();

            // to avoid the current object being polluted
            glBindVertexArray(0);
            glUseProgram(0);
        }

        void setCenter(glm::vec2 newCenter) {
            center = newCenter;
        }

        void Cleanup() {
            glBindVertexArray(0);
            glUseProgram(0);
            glDeleteBuffers(1, &vertex_buffer_object_position_);
            glDeleteBuffers(1, &vertex_buffer_object_index_);
            glDeleteVertexArrays(1, &vertex_array_id_);
            glDeleteProgram(program_id_);
            glDeleteTextures(1, &texture_mirror_id_);
            glDeleteTextures(1, &normal_texture_id_);
            glDeleteTextures(1, &normal_texture2_id_);
        }

        void Draw(float time, const glm::mat4 &model, const glm::mat4 &view,
                  const glm::mat4 &projection, float lightAngle) {

            glUseProgram(program_id_);
            glBindVertexArray(vertex_array_id_);

            // bind textures
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, texture_mirror_id_);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, normal_texture_id_);
            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, normal_texture2_id_);


            // setup MVP
            // setup matrix stack
            glUniformMatrix4fv(model_id, ONE, DONT_TRANSPOSE, glm::value_ptr(model));
            glUniformMatrix4fv(view_id, ONE, DONT_TRANSPOSE, glm::value_ptr(view));
            glUniformMatrix4fv(projection_id, ONE, DONT_TRANSPOSE, glm::value_ptr(projection));

            // Pass time, transparency and reflection
            glUniform1f(time_id, time);
            glUniform1f(transparency_id, transparency);
            glUniform1f(reflection_id, reflection);
            glUniform1f(refraction_id, refraction);

            // Pass light parameters to shader
            glUniform1f(lightAngle_id, lightAngle);

            // Pass waves parameters to shader
            glUniform1f(alpha_id, alpha);
            glUniform1f(waveSpeed_id, waveSpeed);
            glUniform2fv(waveDir_id, ONE, glm::value_ptr(waveDir));
            glUniform2fv(center_id, ONE, glm::value_ptr(center));

            // draw
            glEnable(GL_BLEND);
            glDrawElements(GL_TRIANGLE_STRIP, num_indices_, GL_UNSIGNED_INT, 0);
            glDisable(GL_BLEND);

            glBindVertexArray(0);
            glUseProgram(0);
        }

        void getAllUniformLocation() {

            // Matrix
            model_id = glGetUniformLocation(program_id_, "model");
            view_id = glGetUniformLocation(program_id_, "view");
            projection_id = glGetUniformLocation(program_id_, "projection");

            // Time, transparency and reflection
            time_id = glGetUniformLocation(program_id_, "time");
            transparency_id = glGetUniformLocation(program_id_, "transparency");
            reflection_id = glGetUniformLocation(program_id_, "reflection");
            refraction_id = glGetUniformLocation(program_id_, "refraction");

            // Light
            lightAngle_id = glGetUniformLocation(program_id_, "lightAngle");

            // Waves
            waveDir_id = glGetUniformLocation(program_id_, "waveDirection");
            waveSpeed_id = glGetUniformLocation(program_id_, "waveSpeed");
            alpha_id = glGetUniformLocation(program_id_, "alpha");
            center_id = glGetUniformLocation(program_id_, "center");
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

                if (nb_component == 3) {
                    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
                } else if (nb_component == 4) {
                    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
                }

                GLuint tex_id = glGetUniformLocation(program_id_, texture_name.c_str());
                glUniform1i(tex_id, val - GL_TEXTURE0);
            }
};
