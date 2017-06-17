#pragma once
#include "icg_helper.h"
#include "config.h"
#include <glm/gtc/type_ptr.hpp>

class Terrain {

    private:
        GLuint vertex_array_id_;                // vertex array object
        GLuint vertex_buffer_object_position_;  // memory buffer for positions
        GLuint vertex_buffer_object_index_;     // memory buffer for indices
        GLuint num_indices_;
        GLuint program_id_;                     // GLSL shader program ID
        GLuint texture_heightmap_id;
        GLuint rock_texture_id_;
        GLuint sand_texture_id_;
        GLuint water_texture_id_;
        GLuint grass_texture_id_;
        GLuint snow_texture_id_;
        GLuint main_texture_id_;
        GLuint shore_texture_id_;
        GLuint grass_high_texture_id_;

        // Matrix
        GLint model_id;
        GLint view_id;
        GLint projection_id;

        // Light
        GLuint lightAngle_id;

        // Others
        GLuint center_id;
        GLuint clip_id;
        GLuint snowHeight_id;

        // Wireframe
        GLuint wireframe_id;
        bool wireframe = false;

        // important parameters
        glm::vec2 center = INITIAL_CENTER;

    public:
        void Init(GLuint tex_id) {
            // compile the shaders.
            program_id_ = icg_helper::LoadShaders("terrain_vshader.glsl",
                                                  "terrain_fshader.glsl",
                                                  "terrain_tcshader.glsl",
                                                  "terrain_teshader.glsl");
            if(!program_id_) {
                exit(EXIT_FAILURE);
            }

            glUseProgram(program_id_);

            // vertex one vertex array
            glGenVertexArrays(1, &vertex_array_id_);
            glBindVertexArray(vertex_array_id_);

            // pass real grid size as uniform
            GLuint world_size_id = glGetUniformLocation(program_id_, "world_size");
            glUniform1f(world_size_id, WORLD_SIZE);

            this->center = center;

            // vertex coordinates and indices
           {
               std::vector<GLfloat> vertices;
               std::vector<GLuint> indices;

               float quad_resolution = 256.0;
               float quad_size = WORLD_SIZE/quad_resolution;

               // Vertex position of the quads
               for (int i = 0; i < quad_resolution; ++i) {
                   for (int j = 0; j < quad_resolution; ++j) {
                       vertices.push_back(quad_size*i - WORLD_SIZE/2);
                       vertices.push_back(quad_size*j - WORLD_SIZE/2);
                   }
               }

               for (int i = 1; i < quad_resolution - 1; i++) {
                   for (int j = 0; j < quad_resolution - 1; j++) {
                       indices.push_back(i*quad_resolution + j);
                       indices.push_back((i+1)*quad_resolution + j);
                       indices.push_back((i+1)*quad_resolution + j + 1);
                       indices.push_back(i*quad_resolution + j + 1);
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

            // load/Assign heightmap texture
            {
                this->texture_heightmap_id = tex_id;
                glBindTexture(GL_TEXTURE_2D, texture_heightmap_id);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                GLuint i_tex_id = glGetUniformLocation(program_id_, "tex");
                glUniform1i(i_tex_id, 0 /*GL_TEXTURE0*/);

                // cleanup
                glBindTexture(GL_TEXTURE_2D, 0);
            }

            // load terrain textures
            initTexture("rock2.tga", &sand_texture_id_, "sand_tex", GL_TEXTURE1);
            initTexture("g1.tga", &grass_texture_id_, "grass_tex", GL_TEXTURE2);
            initTexture("r6.tga", &rock_texture_id_, "rock_tex", GL_TEXTURE3);
            initTexture("snow.tga", &snow_texture_id_, "snow_tex", GL_TEXTURE4);
            initTexture("d1.tga", &main_texture_id_, "main_tex", GL_TEXTURE5);
            initTexture("f2.tga", &shore_texture_id_, "shore_tex", GL_TEXTURE6);
            initTexture("g5.tga", &grass_high_texture_id_, "grass_high_tex", GL_TEXTURE7);


            getAllUniformLocation();

            // to avoid the current object being polluted
            glBindVertexArray(0);
            glUseProgram(0);

        }

        void setCenter(glm::vec2 newCenter) {
            center = newCenter;
        }

        void setWireframe(bool value) {
            wireframe = value;
        }

        void Cleanup() {
            glBindVertexArray(0);
            glUseProgram(0);
            glDeleteBuffers(1, &vertex_buffer_object_position_);
            glDeleteVertexArrays(1, &vertex_array_id_);
            glDeleteProgram(program_id_);
            glDeleteTextures(1, &sand_texture_id_);
            glDeleteTextures(1, &rock_texture_id_);
            glDeleteTextures(1, &grass_texture_id_);
            glDeleteTextures(1, &snow_texture_id_);
            glDeleteTextures(1, &main_texture_id_);
            glDeleteTextures(1, &texture_heightmap_id);
            glDeleteTextures(1, &shore_texture_id_);
            glDeleteTextures(1, &grass_high_texture_id_);
        }

        void Draw(const glm::mat4 &model, const glm::mat4 &view, const glm::mat4 &projection,
                  int clip, float lightAngle, float snowHeight) {

            glUseProgram(program_id_);
            glBindVertexArray(vertex_array_id_);

            bindAllTexture();

            // Matrix
            glUniformMatrix4fv(model_id, ONE, DONT_TRANSPOSE, glm::value_ptr(model));
            glUniformMatrix4fv(view_id, ONE, DONT_TRANSPOSE, glm::value_ptr(view));
            glUniformMatrix4fv(projection_id, ONE, DONT_TRANSPOSE, glm::value_ptr(projection));

            // Light
            glUniform1f(lightAngle_id, lightAngle);

            // Wireframe
            glUniform1i(wireframe_id, wireframe);

            // Others
            glUniform2fv(center_id, 1, &center[0]);
            glUniform1i(clip_id, clip);
            glUniform1f(snowHeight_id, snowHeight);

            // Draw
            glPatchParameteri(GL_PATCH_VERTICES, 4);
            //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            glDrawElements(GL_PATCHES, num_indices_, GL_UNSIGNED_INT, 0);

            glBindVertexArray(0);
            glUseProgram(0);

        }

        void initTexture(string filename, GLuint *texture_id, string texture_name, int val) {

            int width;
            int height;
            int nb_component;

            //set stb_image to have the same coordinates as OpenGl
            stbi_set_flip_vertically_on_load(1);
            unsigned char *image = stbi_load(filename.c_str(), &width, &height, &nb_component, 0);

            if (image == nullptr) {
                throw(string("Failed to load texture"));
            }

            glGenTextures(1, texture_id);
            glBindTexture(GL_TEXTURE_2D, *texture_id);

            if (nb_component == 3) {
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
            } else if (nb_component == 4) {
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
            }


            glGenerateMipmap(GL_TEXTURE_2D);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);


            GLuint tex_id = glGetUniformLocation(program_id_, texture_name.c_str());
            //glUseProgram(program_id_);
            glUniform1i(tex_id, val - GL_TEXTURE0);

            // cleanup
            glBindTexture(GL_TEXTURE_2D, val);
            stbi_image_free(image);
        }


        void getAllUniformLocation() {

            // Matrix
            model_id = glGetUniformLocation(program_id_, "model");
            view_id = glGetUniformLocation(program_id_, "view");
            projection_id = glGetUniformLocation(program_id_, "projection");

            // Light
            lightAngle_id = glGetUniformLocation(program_id_, "lightAngle");

            // Wireframe
            wireframe_id = glGetUniformLocation(program_id_, "wireframe");

            // Others
            center_id = glGetUniformLocation(program_id_, "center");
            clip_id = glGetUniformLocation(program_id_, "clip");
            snowHeight_id = glGetUniformLocation(program_id_, "snowHeight");
        }

        void bindAllTexture() {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, texture_heightmap_id);

            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, sand_texture_id_);

            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, grass_texture_id_);

            glActiveTexture(GL_TEXTURE3);
            glBindTexture(GL_TEXTURE_2D, rock_texture_id_);

            glActiveTexture(GL_TEXTURE4);
            glBindTexture(GL_TEXTURE_2D, snow_texture_id_);

            glActiveTexture(GL_TEXTURE5);
            glBindTexture(GL_TEXTURE_2D, main_texture_id_);

            glActiveTexture(GL_TEXTURE6);
            glBindTexture(GL_TEXTURE_2D, shore_texture_id_);

            glActiveTexture(GL_TEXTURE7);
            glBindTexture(GL_TEXTURE_2D, grass_high_texture_id_);
        }
};
