//
// Created by Иван Ильин on 23.01.2021.
//

#ifndef INC_3DZAVR_TRANSFORMANIMATION_H
#define INC_3DZAVR_TRANSFORMANIMATION_H

#include "utils/Time.h"
#include "utils/Log.h"
#include "Mesh.h"

template <typename T>
class TransformAnimation {
public:
    enum Type{
        transition,
        translateToPoint,
        attractToPoint,
        rotation,
        rotateRelativePoint,
        rotateUpLeftLookAt,
        showCreation,
        wait,
    };
public:
    T& obj;

    double time = 0; // normalized time (from 0 to 1)

    double endAnimationPoint = 0;
    double startAnimationPoint = 0;
    Point4D val{};
    Point4D p{};
    double r = 0;
    Type t_type;

    double duration = 0;

    bool started = false;

    Point4D bezier4[4] = {{0,0}, {0.8, 0}, {0.2, 1}, {1, 1}};
    double bezier(double time_);

    explicit TransformAnimation(T& o, Type t, const Point4D& value, double d);
    explicit TransformAnimation(T& o, Type t, const Point4D& point, const Point4D& value, double d);
    explicit TransformAnimation(T& o, Type t, const Point4D& point, double rate, double d);
    explicit TransformAnimation(T& o, Type t, double d);

    bool update();
    [[nodiscard]] Type type() const { return t_type; }

    std::vector<Triangle> triangles;
};

//
// Created by Иван Ильин on 23.01.2021.
//

template <typename T>
TransformAnimation<T>::TransformAnimation(T& o, Type t, const Point4D& value, double d) : obj(o) {
    duration = d;
    t_type = t;
    val = value;
}

template <typename T>
TransformAnimation<T>::TransformAnimation(T& o, TransformAnimation::Type t, double d) : obj(o)  {
    duration = d;
    t_type = t;
    if(t == showCreation)
        triangles = obj.data();
}

template <typename T>
TransformAnimation<T>::TransformAnimation(T& o, TransformAnimation::Type t, const Point4D& point, const Point4D& value, double d) : obj(o)  {
    duration = d;
    t_type = t;
    val = value;
    p = point;
}

template <typename T>
TransformAnimation<T>::TransformAnimation(T& o, TransformAnimation::Type t, const Point4D &point, double rate, double d) : obj(o) {
    duration = d;
    t_type = t;
    p = point;
    r = rate;
}


template <typename T>
double TransformAnimation<T>::bezier(double time_) {

    double h = 0.000001;
    double eps = 0.000001;

    auto f = [=](double t, double px){
        return (1.0-t)*(1.0-t)*(1.0-t)*bezier4[0].x + 3.0*(1.0-t)*(1.0-t)*t*bezier4[1].x + 3.0*(1.0-t)*t*t*bezier4[2].x + t*t*t*bezier4[3].x - px;
    };

    auto py = [=](double t){
        return (1.0-t)*(1.0-t)*(1.0-t)*bezier4[0].y + 3.0*(1.0-t)*(1.0-t)*t*bezier4[1].y + 3.0*(1.0-t)*t*t*bezier4[2].y + t*t*t*bezier4[3].y;
    };

    auto df = [=](double t, double px){
        return (f(t+h, px) - f(t-h, px))/(2.0*h);
    };

    // Newton method
    double k1 = 0.0, k2 = 0.5;
    int s = 0;

    while(abs(k1 - k2) > eps) {
        k1 = k2;
        k2 = k1 - f(k1, time_) / df(k1, time_);
        s++;
    }

    return py(k1);
}

template <typename T>
bool TransformAnimation<T>::update() {
    if(!started) {
        startAnimationPoint = Time::time();
        endAnimationPoint = startAnimationPoint + duration;
    }

    double t_old = time;
    // linear normalized time:
    time = (Time::time() - startAnimationPoint)/(endAnimationPoint - startAnimationPoint);

    double dtime = time - t_old;
    // sin like progress:
    //double dp = 0.5*M_PI*sin(M_PI*time)*dt;

    // Bézier curves progress:
    //double dp = ( -p1 * 3*(1-time)*(1-time) - p2 * 6*time*(1-time) + p2 * 3*time*(1-time)*(1-time) - p3 * 3*time*time + p3 * 6*(1-time)*time + p4 * 3*time*time ).y*dt;
    //double dp = 3.0*((p2-p1)*(1-time)*(1-time) + (p3-p2)*2.0*time*(1.0-time) + (p4-p3)*time*time).y*dt;
    double dp = bezier(time) - bezier(t_old);

    Point4D dval = val * dp;

    std::vector<Triangle> newTriangles;

    switch (t_type) {
        case transition:
            obj.translate(dval);
            break;
        case translateToPoint:
            if(!started) {
                val = val - obj.position();
                t_type = transition;
            }
            break;
        case attractToPoint:
            obj.attractToPoint(p, dp * r);
            break;
        case rotation:
            obj.rotate(dval);
            break;
        case rotateRelativePoint:
            obj.rotateRelativePoint(p, dval);
            break;
        case rotateUpLeftLookAt:
            obj.rotateUpLeftLookAt(dval);
            break;
        case showCreation:
            for(auto &t : triangles) {
                double d1 = (t[1] - t[0]).abs();
                double d2 = (t[2] - t[1]).abs();
                double sum = d1 + d2;
                double a = bezier(time) * sum;
                if(a < d1)
                    newTriangles.emplace_back(t[0], t[0], t[0] + (t[1] - t[0])*a/d1);
                else
                    newTriangles.emplace_back(t[0], t[1], t[1] + (t[2] - t[1])*(a-d1)/d2);
            }
            obj.data() = newTriangles;
            break;
        case wait:

            break;
        default: ;

    }

    started = true;
    return time < 0.999;
}

#endif //INC_3DZAVR_TRANSFORMANIMATION_H
