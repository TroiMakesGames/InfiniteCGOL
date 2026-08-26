#pragma once

#include "raylib.h"
#include "raymath.h"
#include <string>
#include <iostream>
#include <cmath>

class Button
{
    public:
        int width;
        int height;
        int borderWidth;

        std::string fillerText;
        int textSize;

        Color backColor;
        Color borderColor;

        Color selectedBackColor;
        Color selectedBorderColor;

        Color fillerTextColor;
        Color selectedFillerTextColor;

    private:
        bool isPressed = false;
        bool isPressedFirstFrame = false;

    public:

    Button(int width, int height, int borderWidth, std::string fillerText, int textSize, Color backColor = DARKGRAY, Color borderColor = GRAY, Color selectedBackColor = GRAY, Color selectedBorderColor = LIGHTGRAY, Color fillerTextColor = GRAY, Color selectedFillerTextColor = LIGHTGRAY)
    {
        this->width = width;
        this->height = height;
        this->borderWidth = borderWidth;

        this->fillerText = fillerText;
        this->textSize = textSize;

        this->backColor = backColor;
        this->borderColor = borderColor;

        this->selectedBackColor = selectedBackColor;
        this->selectedBorderColor = selectedBorderColor;

        this->fillerTextColor = fillerTextColor;
        this->selectedFillerTextColor = selectedFillerTextColor;
    }

    void Draw(int x, int y)
    {
        //get rectnagle field
        Rectangle field = {(float)x, (float)y, (float)width, (float)height};

        //check for press (first frame vs constant frame logic)
        //track mouse preess and release for constant press, check only press for first frame (otherwise default reset)
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        {
            isPressed = CheckCollisionPointRec(GetMousePosition(), field);
            isPressedFirstFrame = isPressed;
        }
        else
        {isPressedFirstFrame = false;}

        if (isPressed && IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
        {isPressed = false;}

        //draw elements  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        //draw backdrop and border
        Color currentBackColor = isPressed ? selectedBackColor : backColor;
        Color currentBorderColor = isPressed ? selectedBorderColor : borderColor;

        DrawRectangleRec(field, currentBackColor);
        DrawRectangleLinesEx(field, borderWidth, currentBorderColor);

        //draw text
        int textX = x + borderWidth + 5;
        int textY = y + (height - textSize) / 2;

        //use Scissor mode for draws to be strictly visible within a rect
        BeginScissorMode(x + borderWidth, y + borderWidth, width - borderWidth * 2, height - borderWidth * 2);

        if (isPressed)
        {DrawText(fillerText.c_str(), textX, textY, textSize, selectedFillerTextColor);}
        else
        {DrawText(fillerText.c_str(), textX, textY, textSize, fillerTextColor);}
        
        EndScissorMode();
    }

    //helper funcs
    bool IsPressed()
    {return isPressed;}

    bool IsPressedFirstFrame()
    {return isPressedFirstFrame;}
};