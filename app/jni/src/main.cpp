/*
OneLoneCoder.com - 3D Graphics Part #3 - Cameras & Clipping
"Tredimensjonal Grafikk" - @Javidx9

License
~~~~~~~
One Lone Coder Console Game Engine  Copyright (C) 2018  Javidx9
This program comes with ABSOLUTELY NO WARRANTY.
This is free software, and you are welcome to redistribute it
under certain conditions; See license for details.
Original works located at:
https://www.github.com/onelonecoder
https://www.onelonecoder.com
https://www.youtube.com/javidx9
GNU GPLv3
https://github.com/OneLoneCoder/videos/blob/master/LICENSE

From Javidx9 :)
~~~~~~~~~~~~~~~
Hello! Ultimately I don't care what you use this for. It's intended to be
educational, and perhaps to the oddly minded - a little bit of fun.
Please hack this, change it and use it in any way you see fit. You acknowledge
that I am not responsible for anything bad that happens as a result of
your actions. However this code is protected by GNU GPLv3, see the license in the
github repo. This means you must attribute me if you use it. You can view this
license here: https://github.com/OneLoneCoder/videos/blob/master/LICENSE
Cheers!

Background
~~~~~~~~~~
3D Graphics is an interesting, visually pleasing suite of algorithms. This is the
first video in a series that will demonstrate the fundamentals required to
build your own software based 3D graphics systems.

Video
~~~~~
https://youtu.be/ih20l3pJoeU
https://youtu.be/XgMWc6LumG4
https://youtu.be/HXSuNxpCzdM

Author
~~~~~~
Twitter: @javidx9
Blog: http://www.onelonecoder.com
Discord: https://discord.gg/WhwHUMV


Last Updated: 14/08/2018
*/

#include <istream>
#include <iosfwd>
#include <strstream>
#include <fstream>
#include <string>
#include <android/log.h>
#include <stdlib.h>
#include <stdbool.h>
#include "../SDL/src/core/android/SDL_android.h"
#include <vector>
#include <SDL.h>
#include <algorithm>
#include <string>
#include <list>
using namespace std;

const uint32_t WINDOW_WIDTH = 2560;
const uint32_t WINDOW_HEIGHT = 1550;

SDL_Window *win = 0;
SDL_Renderer *ren = 0;

struct vec3d
{
    float x = 0;
    float y = 0;
    float z = 0;
    float w = 1; // Need a 4th term to perform sensible matrix vector multiplication
};

struct triangle
{
    vec3d p[3];

    float color;
};

struct mesh
{
    vector<triangle> tris;

private:
    std::string file_read(const char* filename) {
        SDL_RWops *rw = SDL_RWFromFile(filename, "rb");
        if (rw == NULL) return NULL;

        Sint64 res_size = SDL_RWsize(rw);
        char* res = (char*)malloc(res_size + 1);

        Sint64 nb_read_total = 0, nb_read = 1;
        char* buf = res;
        while (nb_read_total < res_size && nb_read != 0) {
            nb_read = SDL_RWread(rw, buf, 1, (res_size - nb_read_total));
            nb_read_total += nb_read;
            buf += nb_read;
        }
        SDL_RWclose(rw);
        if (nb_read_total != res_size) {
            free(res);
            return NULL;
        }

        std::string str(res);
        return res;

    }

public:
    void LoadFromObjectFile( const char* filename ){
        file_readLine( filename );
    }

private:
    void file_readLine(const char* filename) {

        std::string stuff = file_read(filename);

        std::string line = "";

        std::vector<vec3d> verts;

        for( int I = 0 ; I < stuff.size() ; I++ ){

            if( stuff[I] == '\n' ){
                ///eol
                if (line.find("v") != std::string::npos) {
                    //__android_log_print(ANDROID_LOG_DEBUG, "LINE", "STRING: %s\n", line.c_str());
                    Do_Something_with_Line(line, verts);
                }
                if (line.find("f") != std::string::npos) {
                    //__android_log_print(ANDROID_LOG_DEBUG, "LINE", "STRING: %s\n", line.c_str());
                    Do_Something_with_Line(line, verts);
                }

                line = "";
            }else{
                line += stuff[I];
            }

        }
        if (line.find("v") != std::string::npos) {
            //__android_log_print(ANDROID_LOG_DEBUG, "LINE", "STRING: %s\n", line.c_str());
            Do_Something_with_Line(line, verts);
        }
        if (line.find("f") != std::string::npos) {
            //__android_log_print(ANDROID_LOG_DEBUG, "LINE", "STRING: %s\n", line.c_str());
            Do_Something_with_Line(line, verts);
        }

    }

    void Do_Something_with_Line( std::string sline , std::vector<vec3d> &verts ){

        replaceAll( sline , "\r" , "" );
        replaceAll( sline , "\\r" , "" );

        char *line = &sline[0];

        std::strstream s;
        s << line;

        char junk;

        if (line[0] == 'v')
        {
            //__android_log_print(ANDROID_LOG_DEBUG, "LINE", "STRING ALERT: %s\n", "GETTING VERICES");
            vec3d v;
            s >> junk >> v.x >> v.y >> v.z;
            verts.push_back(v);
        }

        if (line[0] == 'f')
        {
            //__android_log_print(ANDROID_LOG_DEBUG, "LINE", "STRING ALERT: %s\n", "GETTING F");
            int f[3];
            s >> junk >> f[0] >> f[1] >> f[2];
            tris.push_back({ verts[f[0] - 1], verts[f[1] - 1], verts[f[2] - 1] });

        }

    }

    void replaceAll(std::string& str, const std::string& from, const std::string& to) {
        if(from.empty())
            return;
        size_t start_pos = 0;
        while((start_pos = str.find(from, start_pos)) != std::string::npos) {
            str.replace(start_pos, from.length(), to);
            start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
        }
    }
};

struct mat4x4
{
    float m[4][4] = { 0 };
};

vec3d Matrix_MultiplyVector(mat4x4 &m, vec3d &i);
mat4x4 Matrix_MakeIdentity();
mat4x4 Matrix_MakeRotationX(float fAngleRad);
mat4x4 Matrix_MakeRotationY(float fAngleRad);
mat4x4 Matrix_MakeRotationZ(float fAngleRad);
mat4x4 Matrix_MakeTranslation(float x, float y, float z);
mat4x4 Matrix_MakeProjection(float fFovDegrees, float fAspectRatio, float fNear, float fFar);
mat4x4 Matrix_MultiplyMatrix(mat4x4 &m1, mat4x4 &m2);
mat4x4 Matrix_PointAt(vec3d &pos, vec3d &target, vec3d &up);
mat4x4 Matrix_QuickInverse(mat4x4 &m); // Only for Rotation/Translation Matrices
vec3d Vector_Add(vec3d &v1, vec3d &v2);
vec3d Vector_Sub(vec3d &v1, vec3d &v2);
vec3d Vector_Mul(vec3d &v1, float k);
vec3d Vector_Div(vec3d &v1, float k);
float Vector_DotProduct(vec3d &v1, vec3d &v2);
float Vector_Length(vec3d &v);
vec3d Vector_Normalise(vec3d &v);
vec3d Vector_CrossProduct(vec3d &v1, vec3d &v2);
vec3d Vector_IntersectPlane(vec3d &plane_p, vec3d &plane_n, vec3d &lineStart, vec3d &lineEnd);
int Triangle_ClipAgainstPlane(vec3d plane_p, vec3d plane_n, triangle &in_tri, triangle &out_tri1, triangle &out_tri2);
void SDL_RenderDrawTriangle(SDL_Renderer *ren   ,
                            int X1 , int Y1     ,
                            int X2 , int Y2     ,
                            int X3 , int Y3     );

mesh meshCube;
mat4x4 matProj;	// Matrix that converts from view space to screen space
vec3d vCamera;	// Location of camera in world space
vec3d vLookDir;	// Direction vector along the direction camera points
float fYaw;		// FPS Camera rotation in XZ plane
float fTheta;	// Spins World transform

void SDL_RenderDrawTriangle(SDL_Renderer *ren   ,
                            int X1 , int Y1     ,
                            int X2 , int Y2     ,
                            int X3 , int Y3     ) {


    SDL_RenderDrawLine(ren, X1, Y1, X2, Y2);
    SDL_RenderDrawLine(ren, X2, Y2, X3, Y3);
    SDL_RenderDrawLine(ren, X3, Y3, X1, Y1);

}


vec3d Matrix_MultiplyVector(mat4x4 &m, vec3d &i)
{
    vec3d v;
    v.x = i.x * m.m[0][0] + i.y * m.m[1][0] + i.z * m.m[2][0] + i.w * m.m[3][0];
    v.y = i.x * m.m[0][1] + i.y * m.m[1][1] + i.z * m.m[2][1] + i.w * m.m[3][1];
    v.z = i.x * m.m[0][2] + i.y * m.m[1][2] + i.z * m.m[2][2] + i.w * m.m[3][2];
    v.w = i.x * m.m[0][3] + i.y * m.m[1][3] + i.z * m.m[2][3] + i.w * m.m[3][3];
    return v;
}

mat4x4 Matrix_MakeIdentity()
{
    mat4x4 matrix;
    matrix.m[0][0] = 1.0f;
    matrix.m[1][1] = 1.0f;
    matrix.m[2][2] = 1.0f;
    matrix.m[3][3] = 1.0f;
    return matrix;
}

mat4x4 Matrix_MakeRotationX(float fAngleRad)
{
    mat4x4 matrix;
    matrix.m[0][0] = 1.0f;
    matrix.m[1][1] = cosf(fAngleRad);
    matrix.m[1][2] = sinf(fAngleRad);
    matrix.m[2][1] = -sinf(fAngleRad);
    matrix.m[2][2] = cosf(fAngleRad);
    matrix.m[3][3] = 1.0f;
    return matrix;
}

mat4x4 Matrix_MakeRotationY(float fAngleRad)
{
    mat4x4 matrix;
    matrix.m[0][0] = cosf(fAngleRad);
    matrix.m[0][2] = sinf(fAngleRad);
    matrix.m[2][0] = -sinf(fAngleRad);
    matrix.m[1][1] = 1.0f;
    matrix.m[2][2] = cosf(fAngleRad);
    matrix.m[3][3] = 1.0f;
    return matrix;
}

mat4x4 Matrix_MakeRotationZ(float fAngleRad)
{
    mat4x4 matrix;
    matrix.m[0][0] = cosf(fAngleRad);
    matrix.m[0][1] = sinf(fAngleRad);
    matrix.m[1][0] = -sinf(fAngleRad);
    matrix.m[1][1] = cosf(fAngleRad);
    matrix.m[2][2] = 1.0f;
    matrix.m[3][3] = 1.0f;
    return matrix;
}

mat4x4 Matrix_MakeTranslation(float x, float y, float z)
{
    mat4x4 matrix;
    matrix.m[0][0] = 1.0f;
    matrix.m[1][1] = 1.0f;
    matrix.m[2][2] = 1.0f;
    matrix.m[3][3] = 1.0f;
    matrix.m[3][0] = x;
    matrix.m[3][1] = y;
    matrix.m[3][2] = z;
    return matrix;
}

mat4x4 Matrix_MakeProjection(float fFovDegrees, float fAspectRatio, float fNear, float fFar)
{
    float fFovRad = 1.0f / tanf(fFovDegrees * 0.5f / 180.0f * 3.14159f);
    mat4x4 matrix;
    matrix.m[0][0] = fAspectRatio * fFovRad;
    matrix.m[1][1] = fFovRad;
    matrix.m[2][2] = fFar / (fFar - fNear);
    matrix.m[3][2] = (-fFar * fNear) / (fFar - fNear);
    matrix.m[2][3] = 1.0f;
    matrix.m[3][3] = 0.0f;
    return matrix;
}

mat4x4 Matrix_MultiplyMatrix(mat4x4 &m1, mat4x4 &m2)
{
    mat4x4 matrix;
    for (int c = 0; c < 4; c++)
        for (int r = 0; r < 4; r++)
            matrix.m[r][c] = m1.m[r][0] * m2.m[0][c] + m1.m[r][1] * m2.m[1][c] + m1.m[r][2] * m2.m[2][c] + m1.m[r][3] * m2.m[3][c];
    return matrix;
}

mat4x4 Matrix_PointAt(vec3d &pos, vec3d &target, vec3d &up)
{
    // Calculate new forward direction
    vec3d newForward = Vector_Sub(target, pos);
    newForward = Vector_Normalise(newForward);

    // Calculate new Up direction
    vec3d a = Vector_Mul(newForward, Vector_DotProduct(up, newForward));
    vec3d newUp = Vector_Sub(up, a);
    newUp = Vector_Normalise(newUp);

    // New Right direction is easy, its just cross product
    vec3d newRight = Vector_CrossProduct(newUp, newForward);

    // Construct Dimensioning and Translation Matrix
    mat4x4 matrix;
    matrix.m[0][0] = newRight.x;	matrix.m[0][1] = newRight.y;	matrix.m[0][2] = newRight.z;	matrix.m[0][3] = 0.0f;
    matrix.m[1][0] = newUp.x;		matrix.m[1][1] = newUp.y;		matrix.m[1][2] = newUp.z;		matrix.m[1][3] = 0.0f;
    matrix.m[2][0] = newForward.x;	matrix.m[2][1] = newForward.y;	matrix.m[2][2] = newForward.z;	matrix.m[2][3] = 0.0f;
    matrix.m[3][0] = pos.x;			matrix.m[3][1] = pos.y;			matrix.m[3][2] = pos.z;			matrix.m[3][3] = 1.0f;
    return matrix;

}

mat4x4 Matrix_QuickInverse(mat4x4 &m) // Only for Rotation/Translation Matrices
{
    mat4x4 matrix;
    matrix.m[0][0] = m.m[0][0]; matrix.m[0][1] = m.m[1][0]; matrix.m[0][2] = m.m[2][0]; matrix.m[0][3] = 0.0f;
    matrix.m[1][0] = m.m[0][1]; matrix.m[1][1] = m.m[1][1]; matrix.m[1][2] = m.m[2][1]; matrix.m[1][3] = 0.0f;
    matrix.m[2][0] = m.m[0][2]; matrix.m[2][1] = m.m[1][2]; matrix.m[2][2] = m.m[2][2]; matrix.m[2][3] = 0.0f;
    matrix.m[3][0] = -(m.m[3][0] * matrix.m[0][0] + m.m[3][1] * matrix.m[1][0] + m.m[3][2] * matrix.m[2][0]);
    matrix.m[3][1] = -(m.m[3][0] * matrix.m[0][1] + m.m[3][1] * matrix.m[1][1] + m.m[3][2] * matrix.m[2][1]);
    matrix.m[3][2] = -(m.m[3][0] * matrix.m[0][2] + m.m[3][1] * matrix.m[1][2] + m.m[3][2] * matrix.m[2][2]);
    matrix.m[3][3] = 1.0f;
    return matrix;
}

vec3d Vector_Add(vec3d &v1, vec3d &v2)
{
    return { v1.x + v2.x, v1.y + v2.y, v1.z + v2.z };
}

vec3d Vector_Sub(vec3d &v1, vec3d &v2)
{
    return { v1.x - v2.x, v1.y - v2.y, v1.z - v2.z };
}

vec3d Vector_Mul(vec3d &v1, float k)
{
    return { v1.x * k, v1.y * k, v1.z * k };
}

vec3d Vector_Div(vec3d &v1, float k)
{
    return { v1.x / k, v1.y / k, v1.z / k };
}

float Vector_DotProduct(vec3d &v1, vec3d &v2)
{
    return v1.x*v2.x + v1.y*v2.y + v1.z * v2.z;
}

float Vector_Length(vec3d &v)
{
    return sqrtf(Vector_DotProduct(v, v));
}

vec3d Vector_Normalise(vec3d &v)
{
    float l = Vector_Length(v);
    return { v.x / l, v.y / l, v.z / l };
}

vec3d Vector_CrossProduct(vec3d &v1, vec3d &v2)
{
    vec3d v;
    v.x = v1.y * v2.z - v1.z * v2.y;
    v.y = v1.z * v2.x - v1.x * v2.z;
    v.z = v1.x * v2.y - v1.y * v2.x;
    return v;
}

vec3d Vector_IntersectPlane(vec3d &plane_p, vec3d &plane_n, vec3d &lineStart, vec3d &lineEnd)
{
    plane_n = Vector_Normalise(plane_n);
    float plane_d = -Vector_DotProduct(plane_n, plane_p);
    float ad = Vector_DotProduct(lineStart, plane_n);
    float bd = Vector_DotProduct(lineEnd, plane_n);
    float t = (-plane_d - ad) / (bd - ad);
    vec3d lineStartToEnd = Vector_Sub(lineEnd, lineStart);
    vec3d lineToIntersect = Vector_Mul(lineStartToEnd, t);
    return Vector_Add(lineStart, lineToIntersect);
}

int Triangle_ClipAgainstPlane(vec3d plane_p, vec3d plane_n, triangle &in_tri, triangle &out_tri1, triangle &out_tri2)
{
    // Make sure plane normal is indeed normal
    plane_n = Vector_Normalise(plane_n);

    // Return signed shortest distance from point to plane, plane normal must be normalised
    auto dist = [&](vec3d &p)
    {
        vec3d n = Vector_Normalise(p);
        return (plane_n.x * p.x + plane_n.y * p.y + plane_n.z * p.z - Vector_DotProduct(plane_n, plane_p));
    };

    // Create two temporary storage arrays to classify points either side of plane
    // If distance sign is positive, point lies on "inside" of plane
    vec3d* inside_points[3];  int nInsidePointCount = 0;
    vec3d* outside_points[3]; int nOutsidePointCount = 0;

    // Get signed distance of each point in triangle to plane
    float d0 = dist(in_tri.p[0]);
    float d1 = dist(in_tri.p[1]);
    float d2 = dist(in_tri.p[2]);

    if (d0 >= 0) { inside_points[nInsidePointCount++] = &in_tri.p[0]; }
    else { outside_points[nOutsidePointCount++] = &in_tri.p[0]; }
    if (d1 >= 0) { inside_points[nInsidePointCount++] = &in_tri.p[1]; }
    else { outside_points[nOutsidePointCount++] = &in_tri.p[1]; }
    if (d2 >= 0) { inside_points[nInsidePointCount++] = &in_tri.p[2]; }
    else { outside_points[nOutsidePointCount++] = &in_tri.p[2]; }

    // Now classify triangle points, and break the input triangle into
    // smaller output triangles if required. There are four possible
    // outcomes...

    if (nInsidePointCount == 0)
    {
        // All points lie on the outside of plane, so clip whole triangle
        // It ceases to exist

        return 0; // No returned triangles are valid
    }

    if (nInsidePointCount == 3)
    {
        // All points lie on the inside of plane, so do nothing
        // and allow the triangle to simply pass through
        out_tri1 = in_tri;

        return 1; // Just the one returned original triangle is valid
    }

    if (nInsidePointCount == 1 && nOutsidePointCount == 2)
    {
        // Triangle should be clipped. As two points lie outside
        // the plane, the triangle simply becomes a smaller triangle

        // Copy appearance info to new triangle
        out_tri1.color = in_tri.color;

        // The inside point is valid, so keep that...
        out_tri1.p[0] = *inside_points[0];

        // but the two new points are at the locations where the
        // original sides of the triangle (lines) intersect with the plane
        out_tri1.p[1] = Vector_IntersectPlane(plane_p, plane_n, *inside_points[0], *outside_points[0]);
        out_tri1.p[2] = Vector_IntersectPlane(plane_p, plane_n, *inside_points[0], *outside_points[1]);

        return 1; // Return the newly formed single triangle
    }

    if (nInsidePointCount == 2 && nOutsidePointCount == 1)
    {
        // Triangle should be clipped. As two points lie inside the plane,
        // the clipped triangle becomes a "quad". Fortunately, we can
        // represent a quad with two new triangles

        // Copy appearance info to new triangles
        out_tri1.color = in_tri.color;

        out_tri2.color = in_tri.color;

        // The first triangle consists of the two inside points and a new
        // point determined by the location where one side of the triangle
        // intersects with the plane
        out_tri1.p[0] = *inside_points[0];
        out_tri1.p[1] = *inside_points[1];
        out_tri1.p[2] = Vector_IntersectPlane(plane_p, plane_n, *inside_points[0], *outside_points[0]);

        // The second triangle is composed of one of he inside points, a
        // new point determined by the intersection of the other side of the
        // triangle and the plane, and the newly created point above
        out_tri2.p[0] = *inside_points[1];
        out_tri2.p[1] = out_tri1.p[2];
        out_tri2.p[2] = Vector_IntersectPlane(plane_p, plane_n, *inside_points[1], *outside_points[0]);

        return 2; // Return two newly formed triangles which form a quad
    }
}


bool OnUserUpdate(float fElapsedTime)
{

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if ( event.type == SDL_FINGERDOWN || event.type == SDL_FINGERMOTION ) {

            if( event.tfinger.x * WINDOW_WIDTH > WINDOW_WIDTH/2 ){  ///Going right
                vCamera.x += 2.0f;
            }else{                                                  ///Going left
                vCamera.x -= 2.0f;
            }

            if( event.tfinger.y * WINDOW_HEIGHT < WINDOW_HEIGHT/2 ){  ///Going right
                vCamera.y += 2.0f;
            }else{                                                  ///Going left
                vCamera.y -= 2.0f;
            }

        }else if( event.type == SDL_MULTIGESTURE ){

            fYaw += event.mgesture.dTheta * 2;

            vCamera.x -= event.mgesture.dDist * 100;

        }
    }


    // Set up "World Tranmsform" though not updating theta
    // makes this a bit redundant
    mat4x4 matRotZ, matRotX;
    //fTheta += 1.0f * fElapsedTime; // Uncomment to spin me right round baby right round
    matRotZ = Matrix_MakeRotationZ(fTheta * 0.5f);
    matRotX = Matrix_MakeRotationX(fTheta);

    mat4x4 matTrans;
    matTrans = Matrix_MakeTranslation(0.0f, 0.0f, 5.0f);

    mat4x4 matWorld;
    matWorld = Matrix_MakeIdentity();	// Form World Matrix
    matWorld = Matrix_MultiplyMatrix(matRotZ, matRotX); // Transform by rotation
    matWorld = Matrix_MultiplyMatrix(matWorld, matTrans); // Transform by translation

    // Create "Point At" Matrix for camera
    vec3d vUp = { 0,1,0 };
    vec3d vTarget = { 0,0,1 };
    mat4x4 matCameraRot = Matrix_MakeRotationY(fYaw);
    vLookDir = Matrix_MultiplyVector(matCameraRot, vTarget);
    vTarget = Vector_Add(vCamera, vLookDir);
    mat4x4 matCamera = Matrix_PointAt(vCamera, vTarget, vUp);

    // Make view matrix from camera
    mat4x4 matView = Matrix_QuickInverse(matCamera);

    // Store triagles for rastering later
    vector<triangle> vecTrianglesToRaster;

    // Draw Triangles
    for (auto tri : meshCube.tris)
    {
        triangle triProjected, triTransformed, triViewed;

        // World Matrix Transform
        triTransformed.p[0] = Matrix_MultiplyVector(matWorld, tri.p[0]);
        triTransformed.p[1] = Matrix_MultiplyVector(matWorld, tri.p[1]);
        triTransformed.p[2] = Matrix_MultiplyVector(matWorld, tri.p[2]);

        // Calculate triangle Normal
        vec3d normal, line1, line2;

        // Get lines either side of triangle
        line1 = Vector_Sub(triTransformed.p[1], triTransformed.p[0]);
        line2 = Vector_Sub(triTransformed.p[2], triTransformed.p[0]);

        // Take cross product of lines to get normal to triangle surface
        normal = Vector_CrossProduct(line1, line2);

        // You normally need to normalise a normal!
        normal = Vector_Normalise(normal);

        // Get Ray from triangle to camera
        vec3d vCameraRay = Vector_Sub(triTransformed.p[0], vCamera);

        // If ray is aligned with normal, then triangle is visible
        if (Vector_DotProduct(normal, vCameraRay) < 0.0f)
        {
            // Illumination
            vec3d light_direction = { 0.0f, 1.0f, -1.0f };
            light_direction = Vector_Normalise(light_direction);

            // How "aligned" are light direction and triangle surface normal?
            // Choose console colours as required (much easier with RGB)
            triTransformed.color = max(0.1f, Vector_DotProduct(light_direction, normal));



            // Convert World Space --> View Space
            triViewed.p[0] = Matrix_MultiplyVector(matView, triTransformed.p[0]);
            triViewed.p[1] = Matrix_MultiplyVector(matView, triTransformed.p[1]);
            triViewed.p[2] = Matrix_MultiplyVector(matView, triTransformed.p[2]);
            triViewed.color = triTransformed.color;

            // Clip Viewed Triangle against near plane, this could form two additional
            // additional triangles.
            int nClippedTriangles = 0;
            triangle clipped[2];
            nClippedTriangles = Triangle_ClipAgainstPlane({ 0.0f, 0.0f, 0.1f }, { 0.0f, 0.0f, 1.0f }, triViewed, clipped[0], clipped[1]);

            // We may end up with multiple triangles form the clip, so project as
            // required
            for (int n = 0; n < nClippedTriangles; n++)
            {
                // Project triangles from 3D --> 2D
                triProjected.p[0] = Matrix_MultiplyVector(matProj, clipped[n].p[0]);
                triProjected.p[1] = Matrix_MultiplyVector(matProj, clipped[n].p[1]);
                triProjected.p[2] = Matrix_MultiplyVector(matProj, clipped[n].p[2]);
                triProjected.color = clipped[n].color;

                // Scale into view, we moved the normalising into cartesian space
                // out of the matrix.vector function from the previous videos, so
                // do this manually
                triProjected.p[0] = Vector_Div(triProjected.p[0], triProjected.p[0].w);
                triProjected.p[1] = Vector_Div(triProjected.p[1], triProjected.p[1].w);
                triProjected.p[2] = Vector_Div(triProjected.p[2], triProjected.p[2].w);

                // X/Y are inverted so put them back
                triProjected.p[0].x *= -1.0f;
                triProjected.p[1].x *= -1.0f;
                triProjected.p[2].x *= -1.0f;
                triProjected.p[0].y *= -1.0f;
                triProjected.p[1].y *= -1.0f;
                triProjected.p[2].y *= -1.0f;

                // Offset verts into visible normalised space
                vec3d vOffsetView = { 1,1,0 };
                triProjected.p[0] = Vector_Add(triProjected.p[0], vOffsetView);
                triProjected.p[1] = Vector_Add(triProjected.p[1], vOffsetView);
                triProjected.p[2] = Vector_Add(triProjected.p[2], vOffsetView);
                triProjected.p[0].x *= 0.5f * (float)WINDOW_WIDTH;
                triProjected.p[0].y *= 0.5f * (float)WINDOW_HEIGHT;
                triProjected.p[1].x *= 0.5f * (float)WINDOW_WIDTH;
                triProjected.p[1].y *= 0.5f * (float)WINDOW_HEIGHT;
                triProjected.p[2].x *= 0.5f * (float)WINDOW_WIDTH;
                triProjected.p[2].y *= 0.5f * (float)WINDOW_HEIGHT;

                // Store triangle for sorting
                vecTrianglesToRaster.push_back(triProjected);
            }
        }
    }

    // Sort triangles from back to front
    sort(vecTrianglesToRaster.begin(), vecTrianglesToRaster.end(), [](triangle &t1, triangle &t2)
    {
        float z1 = (t1.p[0].z + t1.p[1].z + t1.p[2].z) / 3.0f;
        float z2 = (t2.p[0].z + t2.p[1].z + t2.p[2].z) / 3.0f;
        return z1 > z2;
    });

    // Clear Screen
    SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);
    SDL_RenderClear(ren);

    // Loop through all transformed, viewed, projected, and sorted triangles
    for (auto &triToRaster : vecTrianglesToRaster)
    {
        // Clip triangles against all four screen edges, this could yield
        // a bunch of triangles, so create a queue that we traverse to
        //  ensure we only test new triangles generated against planes
        triangle clipped[2];
        list<triangle> listTriangles;

        // Add initial triangle
        listTriangles.push_back(triToRaster);
        int nNewTriangles = 1;

        for (int p = 0; p < 4; p++)
        {
            int nTrisToAdd = 0;
            while (nNewTriangles > 0)
            {
                // Take triangle from front of queue
                triangle test = listTriangles.front();
                listTriangles.pop_front();
                nNewTriangles--;

                // Clip it against a plane. We only need to test each
                // subsequent plane, against subsequent new triangles
                // as all triangles after a plane clip are guaranteed
                // to lie on the inside of the plane. I like how this
                // comment is almost completely and utterly justified
                switch (p)
                {
                    case 0:	nTrisToAdd = Triangle_ClipAgainstPlane({ 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, test, clipped[0], clipped[1]); break;
                    case 1:	nTrisToAdd = Triangle_ClipAgainstPlane({ 0.0f, WINDOW_HEIGHT - 1, 0.0f }, { 0.0f, -1.0f, 0.0f }, test, clipped[0], clipped[1]); break;
                    case 2:	nTrisToAdd = Triangle_ClipAgainstPlane({ 0.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f }, test, clipped[0], clipped[1]); break;
                    case 3:	nTrisToAdd = Triangle_ClipAgainstPlane({ WINDOW_WIDTH - 1, 0.0f, 0.0f }, { -1.0f, 0.0f, 0.0f }, test, clipped[0], clipped[1]); break;
                }

                // Clipping may yield a variable number of triangles, so
                // add these new ones to the back of the queue for subsequent
                // clipping against next planes
                for (int w = 0; w < nTrisToAdd; w++)
                    listTriangles.push_back(clipped[w]);
            }
            nNewTriangles = listTriangles.size();
        }


        // Draw the transformed, viewed, clipped, projected, sorted, clipped triangles
        for (auto &t : listTriangles)
        {
            //FillTriangle(t.p[0].x, t.p[0].y, t.p[1].x, t.p[1].y, t.p[2].x, t.p[2].y, t.sym, t.col);
            //DrawTriangle(t.p[0].x, t.p[0].y, t.p[1].x, t.p[1].y, t.p[2].x, t.p[2].y, PIXEL_SOLID, FG_BLACK);
            SDL_SetRenderDrawColor(ren, 0, 255, 0, 255);
            SDL_RenderDrawTriangle( ren , t.p[0].x, t.p[0].y, t.p[1].x, t.p[1].y, t.p[2].x, t.p[2].y);
        }
    }

    SDL_RenderPresent(ren);

    return true;
}



int SDL_main(int argc, char **argv) {

    if (SDL_Init(SDL_INIT_EVERYTHING) == -1) {
        __android_log_print(ANDROID_LOG_VERBOSE, "SDL", "SDL_Init: %s\n", SDL_GetError());
        return 1;
    }
    ///Window size: 2560x1550 , Device size: 2560x1600
    win = SDL_CreateWindow("Hello World!", 0, 0, 2560, 1550,
                           SDL_WINDOW_SHOWN | SDL_WINDOW_FULLSCREEN);
    if (win == 0) {
        __android_log_print(ANDROID_LOG_ERROR, "SDL", "SDL_CreateWindow: %s\n", SDL_GetError());
        return 1;
    }
    ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (ren == 0) {
        __android_log_print(ANDROID_LOG_ERROR, "SDL", "SDL_CreateRenderer: %s\n", SDL_GetError());
        return 1;
    }

    // Load object file
    meshCube.LoadFromObjectFile("mountains.obj");

    // Projection Matrix
    matProj = Matrix_MakeProjection(90.0f, (float)WINDOW_HEIGHT / (float)WINDOW_WIDTH, 0.1f, 1000.0f);

    float Counter = 0;
    while(1){
        Counter++;
        OnUserUpdate(Counter);
    }

    return 0;
}

