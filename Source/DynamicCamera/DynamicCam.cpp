#include "raylib.h"
#include "raymath.h"
#include <vector>
#include <iostream>
#include <cmath>

class Viewport 
{
    public:
        Vector2 worldPos;
        float zoom;

    private:
        float movementSpeed;
        float currMovementSpeed;

        float zoomSpeed;
        float minZoom;
        float maxZoom;

    public:
    Viewport(Vector2 worldPos, float movementSpeed, float zoomSpeed, float minZoom, float maxZoom) 
    {
        this->worldPos = worldPos;
        this->movementSpeed = movementSpeed;
        this->currMovementSpeed = movementSpeed;

        this->zoom = 1.0f;
        this->zoomSpeed = zoomSpeed;
        this->minZoom = minZoom;
        this->maxZoom = maxZoom;
    }

    void move(float deltaTime) 
    {
        //get key inputs and move camera world pos
        Vector2 direction = Vector2 (0, 0);
        bool keyPressed = false;

        if (IsKeyDown(KEY_W)) 
        {direction.y -= 1; keyPressed = true;}
        if (IsKeyDown(KEY_S))
        {direction.y += 1; keyPressed = true;}
        if (IsKeyDown(KEY_A))
        {direction.x -= 1; keyPressed = true;}
        if (IsKeyDown(KEY_D))
        {direction.x += 1; keyPressed = true;}

        //normalize direction for proper diagonal scale, multiply with movement speed and apply
        if (keyPressed)
        {
            direction = Vector2Normalize(direction);
            worldPos = Vector2Add(worldPos, Vector2Scale(direction, currMovementSpeed * deltaTime * 60));
        }
    }

    void zoomCamera()
    {
        //get mouse wheel inputs, scale with zoomSpeed and clamp to min/max zoom
        float wheel = GetMouseWheelMove();
        zoom += wheel * zoomSpeed;
        zoom = Clamp(zoom, minZoom, maxZoom);

        //also scale movement speed
        currMovementSpeed = movementSpeed / zoom;
    }

    Vector2 worldToScreenPos(Vector2 objectWorldPos)
    {
        //get screen center to adjust for zoom
        Vector2 originToScreenCenter = {GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f};
        
        Vector2 objToView = Vector2Subtract(objectWorldPos, worldPos);
        objToView = Vector2Scale(objToView, zoom);

        return Vector2Add(originToScreenCenter, objToView);
    }

    Vector2 screenToWorldPos(Vector2 screenPos)
    {
        Vector2 screenCenter = {GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f};

        Vector2 screenToCenter = Vector2Subtract(screenPos, screenCenter);
        screenToCenter = Vector2Scale(screenToCenter, 1.0f / zoom);
        
        return Vector2Add(screenToCenter, worldPos);
    }

    Vector2 worldToScreenScale(Vector2 objectWorldScale)
    {return Vector2Scale(objectWorldScale, zoom);}
};

int main()
{
    //screen initialisation
    const int WIDTH = 750;
    const int HEIGHT = 450;
    InitWindow(WIDTH, HEIGHT, "Dynamic World Camera");
    SetTargetFPS(60);

    //variable initialisation
    Viewport viewport = Viewport(Vector2(0, 0), 10, 0.1f, 0.1f, 5);

    Vector2 objectPosition = Vector2(50, 50);
    Vector2 objectScreenPos = viewport.worldToScreenPos(objectPosition);
    Vector2 objectScale = Vector2(10, 20);
    Vector2 objectScreenScale = viewport.worldToScreenScale(objectScale);

    Vector2 objectPosition2 = Vector2(55, 60);
    Vector2 objectScreenPos2 = viewport.worldToScreenPos(objectPosition2);
    Vector2 objectScale2 = Vector2(10, 20);
    Vector2 objectScreenScale2 = viewport.worldToScreenScale(objectScale2);

    //while loop
    while (!WindowShouldClose()) 
    {
        //update
        viewport.move();
        viewport.zoomCamera();

        objectScreenPos = viewport.worldToScreenPos(objectPosition);
        objectScreenScale = viewport.worldToScreenScale(objectScale);

        objectScreenPos2 = viewport.worldToScreenPos(objectPosition2);
        objectScreenScale2 = viewport.worldToScreenScale(objectScale2);

        //rendering
        BeginDrawing();
        ClearBackground(Color{30, 30, 30, 255});

        DrawRectangle(objectScreenPos.x - objectScreenScale.x/2, objectScreenPos.y - objectScreenScale.y/2, objectScreenScale.x, objectScreenScale.y, Color{255, 0, 0, 255});
        DrawRectangle(objectScreenPos2.x - objectScreenScale2.x/2, objectScreenPos2.y - objectScreenScale2.y/2, objectScreenScale2.x, objectScreenScale2.y, Color{0, 0, 200, 150});

        DrawText(TextFormat("%i", GetFPS()), 10, 14, 20, Color{0, 0, 0, 100});
        DrawText(TextFormat("%i", GetFPS()), 10, 11, 20, Color{0, 150, 0, 255});
        DrawText(TextFormat("%i", GetFPS()), 10, 10, 20, Color{0, 255, 0, 255});
        EndDrawing();
    }

    CloseWindow();
    return 0;
}