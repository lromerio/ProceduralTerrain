#pragma once

// glew must be before glfw
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "icg_helper.h"
#include "config.h"

#include "screenquad/screenquad.h"
#include "framebuffer.h"
#include "terrain/terrain.h"
#include "sky/sky.h"
#include "water/water.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw_gl3.h"
#include "bezier.h"
#include <glm/gtc/matrix_transform.hpp>

using namespace glm;

class ProceduralScene {

    private:

        //Screen
        int window_width = WINDOW_WIDTH;
        int window_height = WINDOW_HEIGHT;

        //MVP
        mat4 model = IDENTITY_MATRIX;
        mat4 projection = IDENTITY_MATRIX;
        mat4 view = IDENTITY_MATRIX;

        //Objects
        FrameBuffer framebuffer;
        FrameBuffer mirror_framebuffer;
        ScreenQuad screenquad;
        Terrain terrain;
        Sky sky;
        Water water;

        // View and navigation
        GLfloat cam_yaw    = START_CAM_YAW;
        GLfloat cam_pitch  =  START_CAM_PITCH;
        GLfloat lastX  =  WINDOW_WIDTH  / 2.0;
        GLfloat lastY  =  WINDOW_HEIGHT / 2.0;
        bool keys[1024];
        bool drag = false;
        bool firstMouse = true;
        enum Camera_mode {FLYTHROUGH, FPS, RECORD_BEZIER, BEZIER, CUSTOM, PRE_RECORDED};
        Camera_mode camera_mode = CUSTOM;
        float last_frame_time = 0;

        float curr_camera_speed = CAM_SPEED;
        float pitch_speed = 0.0f;
        float yaw_speed = 0.0f;

        // FPS
        GLfloat *heightmap;

        // Bezier
        Bezier path;
        Bezier cam;
        Bezier prerecorded_path;
        Bezier prerecorded_cam;
        bool start_path = true;
        float bezier_time;
        float curr_speed = 0.5f;
        vec3 eye = INITIAL_EYE;
        vec3 front = INITIAL_FRONT;
        vec3 up = INITIAL_UP;
        vec2 center = INITIAL_CENTER;
        bool precomputed = false;

        // Terrain
        bool renderTerrain = true;
        bool wireframe = false;
        int scaleFactor = INITIAL_SCALE;
        float H = INITIAL_H;
        float lacunarity = INITIAL_LACUNARITY;
        int octaves = INITIAL_OCTAVES;

        // Water
        bool renderWater = true;
        bool reflectSky = true;
        bool reflectTerrain = true;
        float waterTransparency = INITIAL_TRANSPARENCY;
        float waterReflection = INITIAL_REFLECTION;
        float waterRefraction = INITIAL_REFRACTION;
        float alpha = INITIAL_ALPHA;
        float waveSpeed = INITIAL_WAVE_SPEED;
        glm::vec2 waveDir = INITIAL_WAVE_DIR;

        // Light
        bool auto_light = false;
        bool dinamic_snow = false;
        float dayNight = 1.0;
        float dayNightSpeed = 1.0;
        float snowSpeed = 10.0;
        float customSnowHeight = 0.0;

    public:

        void Init(GLFWwindow* window) {

            // set background color
            glClearColor(0.0, 0.0, 0.0, 1.0);

            // Initialize gui
            ImGui_ImplGlfwGL3_Init(window, true);

            // enable depth test.
            glEnable(GL_DEPTH_TEST);
            glEnable(GL_MULTISAMPLE);

            // generate view matrix
            view = lookAt(eye, eye + front, up);

            // Initialize objects
            sky.Init();
            screenquad.Init(window_width, window_height);
            heightmap = new GLfloat[1];
            prerecordedBezierInit();

            GLuint framebuffer_tex_id = framebuffer.Init(window_width, window_height, true);
            terrain.Init(framebuffer_tex_id);

            GLuint mirror_framebuffer_tex_id = mirror_framebuffer.Init(window_width, window_height);
            water.Init( mirror_framebuffer_tex_id);

            renderNoiseToBuffer();
        }


        void Display() {

            glViewport(0,0, window_width, window_height);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // Update camera
            cameraHandler();

            // Setup Day/Night (and snow) cycle
            const float time = glfwGetTime();
            float lightAngle;
            float snowHeight;
            if (auto_light) {
                lightAngle = time/10*dayNightSpeed;
                snowHeight = dinamic_snow ? 0.5*sin(lightAngle/snowSpeed) : 0.0;
            } else {
                lightAngle = dayNight;
                snowHeight = dinamic_snow ? customSnowHeight : 0.0;
            }

            // Mirroring
            vec3 mirror_eye = vec3(eye.x, -eye.y, eye.z);
            vec3 mirror_front = vec3(front.x, -front.y, front.z);
            mat4 view_reflection = lookAt(mirror_eye, mirror_eye + mirror_front, vec3(0.0f, -1.0f, 0.0f));
            if(renderWater && !wireframe) {
                // Render reflection to buffer
                mirror_framebuffer.Bind();
                {
                    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                    glEnable(GL_CLIP_DISTANCE0);
                    if(reflectSky) {
                        sky.Draw(model, view_reflection, projection, true, lightAngle);
                    }
                    if(reflectTerrain) {
                        terrain.Draw(model, view_reflection, projection, 1, lightAngle, snowHeight);
                    }
                    glDisable(GL_CLIP_DISTANCE0);
                }
                mirror_framebuffer.Unbind();
            }

            // Render scene
            if (renderTerrain) {
                if(!wireframe) {
                    terrain.Draw(model, view, projection, 0, lightAngle, snowHeight);
                } else {
                    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                    terrain.Draw(model, view, projection, 0, lightAngle, snowHeight);
                    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                }
            }
            if (renderWater && !wireframe) {
                water.Draw(time, model, view, projection, lightAngle);
            }
            if(!wireframe) {
                sky.Draw(model, view, projection, false, lightAngle);
            }

            // Render interface
            drawGui();
        }

        void Cleanup() {
            framebuffer.Cleanup();
            mirror_framebuffer.Cleanup();
            screenquad.Cleanup();
            terrain.Cleanup();
            sky.Cleanup();
            water.Cleanup();
            ImGui_ImplGlfwGL3_Shutdown();
        }

        // callback methods
        void mousePressCallback(GLFWwindow* window, int button, int action, int mod) {
            if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
                drag = true;
            }
            else{
                drag = false;
            }
        }

        void cursorPositionCallback(GLFWwindow* window, double x, double y) {

            if(drag && (camera_mode == CUSTOM || camera_mode == RECORD_BEZIER)){

                if (firstMouse){
                    lastX = x;
                    lastY = y;
                    firstMouse = false;
                }

                GLfloat xoffset = x - lastX;
                GLfloat yoffset = lastY - y; // Reversed since y-coordinates go from bottom to left
                lastX = x;
                lastY = y;

                xoffset *= MOUSE_SENSITIVITY;
                yoffset *= MOUSE_SENSITIVITY;

                cam_yaw   += xoffset;
                cam_pitch += yoffset;

                updateFront();

            } else {
                lastX = x;
                lastY = y;
            }
        }

        void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {

            if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
                glfwSetWindowShouldClose(window, GL_TRUE);
            }

            if (key >= 0 && key < 1024)
                {
                    if (action == GLFW_PRESS)
                        keys[key] = true;
                    else if (action == GLFW_RELEASE)
                        keys[key] = false;
            }

            if (camera_mode == RECORD_BEZIER &&  action == GLFW_RELEASE && key == GLFW_KEY_R){
                record();
            }
        }


        // Gets called when the windows/framebuffer is resized.
        void resizeCallback(int width, int height) {
            window_width = width;
            window_height = height;

            float near = CAM_NEAR;
            float far = CAM_FAR;
            float aspect = (float)window_width / window_height;
            projection = computePerspectiveProjection(START_CAM_FOV, aspect, near, far);

            //mirror_framebuffer.UpdateSize(width, height);
            //framebuffer.UpdateSize(width, height);
        }

    // helper methods
    private:

        void cameraHandler() {

            switch(camera_mode) {
                case CUSTOM:
                    do_movement();
                    break;
                case FLYTHROUGH:
                    do_movement_flythrough();
                    break;
                case FPS:
                    do_movement_flythrough();
                    break;
                case RECORD_BEZIER:
                    do_movement();
                    break;
                case BEZIER:
                    do_movement_bezier();
                    break;
                case PRE_RECORDED:
                    do_movement_bezier();
                    break;
                default:
                    break;
            }

            view = lookAt(eye, eye + front, up);
        }

        void renderNoiseToBuffer() {

            framebuffer.Bind();
            {
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                screenquad.Draw();
                if (camera_mode == FPS) {
                    glReadPixels(window_width/2, window_height/2, 1, 1, GL_RED, GL_FLOAT, heightmap);
                }
            }
            framebuffer.Unbind();
        }

        void record(){
            vec3* recordPoint = new vec3(center.x, eye.y, center.y);
            path.addControlPoint(*recordPoint);
            vec3* recordAngle = new vec3(cam_pitch, cam_yaw, 0);
            cam.addControlPoint(*recordAngle);

            cout << "point " << recordPoint->x << " " << recordPoint->y << " " << recordPoint->z << endl;
            cout << "angle " << recordAngle->x << " " << recordAngle->y << endl;
            cout << endl;

        }

        void prerecordedBezierInit() {
            vec3* point;
            vec3* angle;

            point = new vec3(0.0, 20.0, 0.0);
            prerecorded_path.addControlPoint(*point);
            angle = new vec3(2.80488, 171.884, 0);
            prerecorded_cam.addControlPoint(*angle);

            point = new vec3(-0.0764674, 13.036, -0.0100195);
            prerecorded_path.addControlPoint(*point);
            angle = new vec3(-32.4414, 172.219, 0);
            prerecorded_cam.addControlPoint(*angle);

            point = new vec3(-0.0956858, 9.94455, -0.0126155);
            prerecorded_path.addControlPoint(*point);
            angle = new vec3(-32.0468, 172.617, 0);
            prerecorded_cam.addControlPoint(*angle);

            point = new vec3(-0.126162, 5.91191, -0.0183634);
            prerecorded_path.addControlPoint(*point);
            angle = new vec3(-24.6968, 167.017, 0);
            prerecorded_cam.addControlPoint(*angle);

            point = new vec3(-0.127933, 5.70301, -0.0187716);
            prerecorded_path.addControlPoint(*point);
            angle = new vec3(-24.6968, 167.017, 0);
            prerecorded_cam.addControlPoint(*angle);

            point = new vec3(-0.154594, 3.2237, -0.0322502);
            prerecorded_path.addControlPoint(*point);
            angle = new vec3(-12.9468, 122.517, 0);
            prerecorded_cam.addControlPoint(*angle);

            point = new vec3(-0.136699, 12.6406, 0.010991);
            prerecorded_path.addControlPoint(*point);
            angle = new vec3(-38.3061, 110.153, 0);
            prerecorded_cam.addControlPoint(*angle);

            point = new vec3(-0.127946, 15.6198, 0.0254738);
            prerecorded_path.addControlPoint(*point);
            angle = new vec3(-27.9061, 137.403, 0);
            prerecorded_cam.addControlPoint(*angle);

            point = new vec3(-0.127946, 15.6198, 0.0254738);
            prerecorded_path.addControlPoint(*point);
            angle = new vec3(-25.7561, 140.503, 0);
            prerecorded_cam.addControlPoint(*angle);

            point = new vec3(-0.119916, 16.8863, 0.0794416);
            prerecorded_path.addControlPoint(*point);
            angle = new vec3(4.22987, 197.66, 0);
            prerecorded_cam.addControlPoint(*angle);

            point = new vec3(-0.119916, 16.8863, 0.0794416);
            prerecorded_path.addControlPoint(*point);
            angle = new vec3(21.8504, 264.972, 0);
            prerecorded_cam.addControlPoint(*angle);
            //
            point = new vec3(-0.120363, 26.4823, 0.137978);
            prerecorded_path.addControlPoint(*point);
            angle = new vec3(33.2766, 269.562, 0);
            prerecorded_cam.addControlPoint(*angle);

            point = new vec3(-0.120466, 28.677, 0.151354);
            prerecorded_path.addControlPoint(*point);
            angle = new vec3(33.2766, 269.562, 0);
            prerecorded_cam.addControlPoint(*angle);

            point = new vec3(-0.120188, 33.7663, 0.1969);
            prerecorded_path.addControlPoint(*point);
            angle = new vec3(21.2524, 269.546, 0);
            prerecorded_cam.addControlPoint(*angle);

            point = new vec3(-0.164935, 33.185, 0.300475);
            prerecorded_path.addControlPoint(*point);
            angle = new vec3(-24.9279, 201.296, 0);
            prerecorded_cam.addControlPoint(*angle);

            point = new vec3(-0.236531, 25.4521, 0.36964);
            prerecorded_path.addControlPoint(*point);
            angle = new vec3(-17.228, 173.596, 0);
            prerecorded_cam.addControlPoint(*angle);
            //
            point = new vec3(-0.237592, 25.378, 0.370527);
            prerecorded_path.addControlPoint(*point);
            angle = new vec3(-17.228, 173.596, 0);
            prerecorded_cam.addControlPoint(*angle);

            point = new vec3(-0.316805, 13.5839, 0.349014);
            prerecorded_path.addControlPoint(*point);
            angle = new vec3(-30.0257, 150.34, 0);
            prerecorded_cam.addControlPoint(*angle);

            point = new vec3(-0.340688, 9.4208, 0.335942);
            prerecorded_path.addControlPoint(*point);
            angle = new vec3(-51.406, 155.128, 0);
            prerecorded_cam.addControlPoint(*angle);

            point = new vec3(-0.35186, 5.48069, 0.330763);
            prerecorded_path.addControlPoint(*point);
            angle = new vec3(-52.006, 155.128, 0);
            prerecorded_cam.addControlPoint(*angle);

            point = new vec3(-0.384819, 4.480028, 0.310678);
            prerecorded_path.addControlPoint(*point);
            angle = new vec3(5.0, 144.028, 0);
            prerecorded_cam.addControlPoint(*angle);

            point = new vec3(-0.387239, 4.418495, 0.308922);
            prerecorded_path.addControlPoint(*point);
            angle = new vec3(5.0, 144.028, 0);
            prerecorded_cam.addControlPoint(*angle);

            point = new vec3(-0.15, 60.0, 0.15);
            prerecorded_path.addControlPoint(*point);
            angle = new vec3(2.80488, 171.884, 0);
            prerecorded_cam.addControlPoint(*angle);

            point = new vec3(0.0, 20.0, 0.0);
            prerecorded_path.addControlPoint(*point);
            angle = new vec3(2.80488, 171.884, 0);
            prerecorded_cam.addControlPoint(*angle);

        }

        void do_movement_bezier(){

            // Update speed
            if(keys[GLFW_KEY_W]){

                curr_speed += 0.01;
            }
            if(keys[GLFW_KEY_S]){

                curr_speed -= 0.01;
                curr_speed = curr_speed < 0.0 ? 0.0 : curr_speed;
            }

            if (start_path) {

                bezier_time = 0;
                start_path = false;
            } else {


                // Update Bezier parameter
                float curr_time = glfwGetTime();
                float frame_time = curr_time - last_frame_time;
                last_frame_time = curr_time;
                bezier_time += frame_time*curr_speed;

                if(camera_mode == PRE_RECORDED) {
                    if (bezier_time > prerecorded_path.getCount() - 1) {

                        // (Re)Start
                        start_path = true;
                        center = vec2(0.0, 0.0);
                    } else {

                        // Update position
                        vec3 currPoint = prerecorded_path.getBezier(bezier_time);
                        eye.y = currPoint.y;
                        center = vec2(currPoint.x, currPoint.z);
                        vec3 currAngles = prerecorded_cam.getBezier(bezier_time);
                        cam_pitch = currAngles.x;
                        cam_yaw = currAngles.y;
                        updateFront();
                    }
                } else {
                    if (bezier_time > path.getCount() - 1) {

                        // (Re)Start
                        start_path = true;
                        center = vec2(0.0, 0.0);
                    } else {

                        // Update position
                        vec3 currPoint = path.getBezier(bezier_time);
                        eye.y = currPoint.y;
                        center = vec2(currPoint.x, currPoint.z);
                        vec3 currAngles = cam.getBezier(bezier_time);
                        cam_pitch = currAngles.x;
                        cam_yaw = currAngles.y;
                        updateFront();
                    }
                }
            }

            // Render
            screenquad.setCenter(center);
            terrain.setCenter(center);
            renderNoiseToBuffer();
        }

        void do_movement_fps(){

        }

        void do_movement_flythrough(){

            bool needRender = false;

            // Decelerate
            if (!keys[GLFW_KEY_W] && !keys[GLFW_KEY_S]) {
                curr_camera_speed *= CAM_DECELERATION_FACTOR;
            }
            if(!keys[GLFW_KEY_A] && !keys[GLFW_KEY_D]){

                yaw_speed *= CAM_DECELERATION_FACTOR;
            }
            if (!keys[GLFW_KEY_Q] && !keys[GLFW_KEY_E]) {

                pitch_speed *= CAM_DECELERATION_FACTOR;
            }

            updateFront();

            if (keys[GLFW_KEY_W]) {

                curr_camera_speed += CAM_STRAIGHT_ACC_FACTOR;
                curr_camera_speed = clamp(curr_camera_speed, MIN_CAM_SPEED, MAX_CAM_SPEED);

                if (camera_mode == FPS) {
                    curr_camera_speed = clamp(curr_camera_speed, -CAM_SPEED, CAM_SPEED);
                }

                vec3 diff = curr_camera_speed * front;
                center += vec2(diff.x, -diff.z);

                // Update eye
                if (camera_mode == FLYTHROUGH) {
                    eye += vec3(0.0, VERTICAL_SPEED_MULT*diff.y, 0.0);
                } else {
                    eye.y = heightmap[0]*TERRAIN_HEIGHT_MULTIPLIER+4;
                    eye.y = (eye.y < 4.0f) ? 4.0f : eye.y;
                }

                needRender = true;
            }
            if (keys[GLFW_KEY_S]) {

                curr_camera_speed -= CAM_STRAIGHT_ACC_FACTOR;
                curr_camera_speed = clamp(curr_camera_speed, MIN_CAM_SPEED, MAX_CAM_SPEED);

                vec3 diff = curr_camera_speed * front;
                center += vec2(diff.x, -diff.z);

                // Update eye
                if (camera_mode == FLYTHROUGH) {
                    eye -= vec3(0.0, VERTICAL_SPEED_MULT*diff.y, 0.0);
                } else {
                    eye.y = heightmap[0]*TERRAIN_HEIGHT_MULTIPLIER+4;
                }

                needRender = true;
            }
            if (keys[GLFW_KEY_D]) {
                yaw_speed += CAM_ANGLE_ACC_FACTOR;
            }
            if (keys[GLFW_KEY_A]) {
                yaw_speed -= CAM_ANGLE_ACC_FACTOR;
            }
            if (keys[GLFW_KEY_Q]) {
                pitch_speed += CAM_ANGLE_ACC_FACTOR;
            }
            if (keys[GLFW_KEY_E]) {
                pitch_speed -= CAM_ANGLE_ACC_FACTOR;
            }

            cam_yaw += yaw_speed;

            cam_pitch += pitch_speed;
            cam_pitch = clamp(cam_pitch, MIN_CAM_PITCH, MAX_CAM_PITCH);

            if(cam_pitch == MIN_CAM_PITCH || cam_pitch == MAX_CAM_PITCH) {
                pitch_speed = 0.0;
            }

            // Render new noise texture if needed
            if (needRender) {
                screenquad.setCenter(center);
                terrain.setCenter(center);
                renderNoiseToBuffer();
            }
        }

        void do_movement() {

            bool needRender = false;

            if (keys[GLFW_KEY_W]){

                vec3 diff = CAM_SPEED * front;
                center += vec2(diff.x, -diff.z);
                eye += vec3(0.0, VERTICAL_SPEED_MULT*diff.y, 0.0);

                needRender = true;
            }
            if (keys[GLFW_KEY_S]){

                vec3 diff = CAM_SPEED * front;
                center -= vec2(diff.x, -diff.z);
                eye -= vec3(0.0, VERTICAL_SPEED_MULT*diff.y, 0.0);

                needRender = true;
            }
            if (keys[GLFW_KEY_A]){

                vec3 diff = glm::normalize(glm::cross(front, up)) * CAM_SPEED;
                center -= vec2(diff.x, -diff.z);
                eye.y -= diff.y;

                needRender = true;
            }
            if (keys[GLFW_KEY_D]){

                vec3 diff = glm::normalize(glm::cross(front, up)) * CAM_SPEED;
                center += vec2(diff.x, -diff.z);
                eye.y += diff.y;

                needRender = true;
            }

            // Render new noise texture if needed
            if (needRender) {
                screenquad.setCenter(center);
                terrain.setCenter(center);
                water.setCenter(center);
                renderNoiseToBuffer();
            }
        }

        void updateFront() {

            glm::vec3 frontValue;
            frontValue.x = cos(glm::radians(cam_yaw))*cos(glm::radians(cam_pitch));
            frontValue.y = sin(glm::radians(cam_pitch));
            frontValue.z = sin(glm::radians(cam_yaw))*cos(glm::radians(cam_pitch));
            front = glm::normalize(frontValue);
        }

        mat4 computePerspectiveProjection(float fovy, float aspect, float near, float far) {

            mat4 proj = IDENTITY_MATRIX;

            float top = near*tan(radians(fovy));
            float bottom = -top;
            float right = top*aspect;
            float left = -right;

            proj[0][0] = 2.0f*near / (right - left);
            proj[1][1] = 2.0f*near / (top - bottom);
            proj[2][2] = -(far+near) / (far - near);
            proj[2][3] = -1.0f;
            proj[2][0] = (right + left) / (right - left);
            proj[2][1] = (top + bottom) / (top - bottom);
            proj[3][2] = -2.0f*far*near / (far - near);

            return proj;
        }

//************* INTERFACE *************************************************************

        void drawGui() {

            // Create window
            ImGui_ImplGlfwGL3_NewFrame();
            ImGui::SetNextWindowSize(ImVec2(600, 300), ImGuiSetCond_FirstUseEver);
            ImGui::Begin("Settings");

            drawCameraMenu();
            drawTerrainMenu();
            drawWaterMenu();
            drawLightMenu();

            ImGui::End();
            ImGui::Render();
        }

        void drawTerrainMenu() {

            ImGui::Spacing();
            ImGui::Spacing();
            ImGui::Spacing();
            ImGui::Spacing();
            ImGui::Text("TERRAIN");
            ImGui::Spacing();

            ImGui::Checkbox("Render terrain", &renderTerrain);
            if(ImGui::Checkbox("Wireframe", &wireframe)) {
                terrain.setWireframe(wireframe);
            }

            ImGui::Spacing();
            ImGui::Spacing();

            if (ImGui::SliderInt("Scale", &scaleFactor, 0, 20)) {
                screenquad.setScaleFactor(scaleFactor);
                renderNoiseToBuffer();
            }
            if (ImGui::SliderFloat("H", &H, 0.0, 3.0, "%.2f")) {
                screenquad.setH(H);
                renderNoiseToBuffer();
            }
            if (ImGui::SliderFloat("Lacunarity", &lacunarity, 2.0, 5.0, "%.2f")) {
                screenquad.setLacunarity(lacunarity);
                renderNoiseToBuffer();
            }
            if (ImGui::SliderInt("Octaves", &octaves, 1, 16)) {
                screenquad.setOctaves(octaves);
                renderNoiseToBuffer();
            }
        }

        void drawWaterMenu() {

            ImGui::Spacing();
            ImGui::Spacing();
            ImGui::Spacing();
            ImGui::Spacing();
            ImGui::Text("WATER");
            ImGui::Spacing();


            ImGui::Checkbox("Render water", &renderWater);
            ImGui::Checkbox("Sky reflection", &reflectSky);
            ImGui::Checkbox("Terrain reflection", &reflectTerrain);

            ImGui::Spacing();
            ImGui::Spacing();

            if (ImGui::SliderFloat("Transparency", &waterTransparency, 0.0, 1.0, "%.2f")) {
                water.setTransparency(waterTransparency);
            }
            if (ImGui::SliderFloat("Reflection", &waterReflection, 0.0, 1.0, "%.2f")) {
                water.setReflection(waterReflection);
            }
            if(ImGui::SliderFloat("Refraction", &waterRefraction, 0.0, 0.1, "%.3f")) {
                water.setRefraction(waterRefraction);
            }
            if (ImGui::SliderFloat("Alpha", &alpha, 0.0, 100.0, "%.1f")) {
                water.setAlpha(alpha);
            }

            ImGui::Spacing();
            ImGui::Text("Waves");
            if (ImGui::SliderFloat("Direction x", &waveDir.x, -1.0, 1.0, "%.2f")) {
                water.setWaveDir(waveDir);
            }
            if (ImGui::SliderFloat("Direction y", &waveDir.y, -1.0, 1.0, "%.2f")) {
                water.setWaveDir(waveDir);
            }
            if (ImGui::SliderFloat("Speed", &waveSpeed, 0.0, 1.0)) {
                water.setWaveSpeed(waveSpeed);
            }
        }

        void drawLightMenu() {

            ImGui::Spacing();
            ImGui::Spacing();
            ImGui::Spacing();
            ImGui::Spacing();
            ImGui::Text("LIGHT");
            ImGui::Spacing();


            ImGui::Text("Day/Night");
            ImGui::Checkbox("Dinamic Snow", &dinamic_snow);
            ImGui::Checkbox("Auto", &auto_light);
            if (!auto_light) {
                ImGui::SliderFloat("Day time", &dayNight, 0.0, 10.0);
                ImGui::SliderFloat("Snow height", &customSnowHeight, -0.5, 0.175);
            } else {
                ImGui::SliderFloat("Speed", &dayNightSpeed, 1.0, 10.0, "%.0f");
                ImGui::SliderFloat("Snow speed", &snowSpeed, 1.0, 20.0, "%.0f");
            }
        }

        void drawCameraMenu() {

            ImGui::Spacing();
            ImGui::Spacing();
            ImGui::Spacing();
            ImGui::Spacing();
            ImGui::Text("CAMERA");
            ImGui::Spacing();

            static int new_camera_mode = 4;
            ImGui::RadioButton("Fly-through", &new_camera_mode, 0); ImGui::SameLine();
            ImGui::RadioButton("FPS", &new_camera_mode, 1); ImGui::SameLine();
            ImGui::RadioButton("RecordBezier", &new_camera_mode, 2); ImGui::SameLine();
            ImGui::RadioButton("Custom Bezier", &new_camera_mode, 3); ImGui::SameLine();
            ImGui::RadioButton("Prerecorded Bezier", &new_camera_mode, 5); ImGui::SameLine();
            ImGui::RadioButton("Custom", &new_camera_mode, 4);

            switch (new_camera_mode) {
            case 0:
                camera_mode = FLYTHROUGH;
                break;
            case 1:
                camera_mode = FPS;
                renderNoiseToBuffer();
                eye.y = heightmap[0]*20+4;
                break;
            case 2:
                camera_mode = RECORD_BEZIER;
                break;
            case 3:
                camera_mode = BEZIER;
                break;
            case 4:
                camera_mode = CUSTOM;
                break;
            case 5:
                camera_mode = PRE_RECORDED;
                break;
            default:
                camera_mode = FLYTHROUGH;
                break;
            }
        }
};
