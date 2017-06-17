#pragma once

#include "glm/vec3.hpp"
#include "icg_helper.h"

#include <vector>

class Bezier {
    private:
        vector<glm::vec3> control_points;
        int element_count = 0;
        int spline_degree = 3;
        int splines = 0;

    public:
        glm::vec3 getBezier(float t) {

            if (splines < 1) {

                return calculateBezier(t/element_count, 0, element_count - 1);

            } else {

                int spline_start = floor(t/spline_degree) * spline_degree;
                int spline_end = spline_start + spline_degree;

                // Handle non complete spline
                spline_end = (spline_end > splines * spline_degree) ? element_count - 1 : spline_end;

                int spline_size = spline_end - spline_start;
                float newT = (t - spline_start)/spline_size;

                return calculateBezier(newT, spline_start, spline_end);
            }
        }

        void addControlPoint(glm::vec3 &point) {
            if (element_count > spline_degree && element_count % spline_degree == 0) {

                // If we terminate a spline enforce continuity
                int knot = element_count - spline_degree;
                control_points.at(knot) = (control_points.at(knot - 1) + control_points.at(knot + 1))/2.0f;

                splines++;
            }

            control_points.push_back(point);
            element_count++;
        }

        int getCount() {
            return element_count;
        }

    private:
        glm::vec3 calculateBezier(float t, int spline_start, int spline_end) {

            glm::vec3 sum = glm::vec3(0.0, 0.0, 0.0);

            for (int i = 0; i < spline_end - spline_start + 1; i++) {

                glm::vec3 bi = control_points.at(spline_start + i) * bernstein(t, spline_end - spline_start, i);
                sum = sum + bi;
            }

            return sum;
        }

        float bernstein(float t, int n, int i) {

            return binomial(n, i) * pow(t, i) * pow(1 - t, n - i);
        }

        float binomial(int n, int k) {

            return factorial(n) / (factorial(n - k) * factorial(k));
        }

        float factorial(int n) {

            if (n == 1 || n == 0) {
                return 1;
            } else {
                return n * factorial(n - 1);
            }
        }
};
