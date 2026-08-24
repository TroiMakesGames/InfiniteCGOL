#pragma once    //use script in compiled build only once

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

    void move() 
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
            worldPos = Vector2Add(worldPos, Vector2Scale(direction, currMovementSpeed));
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