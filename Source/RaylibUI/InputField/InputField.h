#pragma once

#include "raylib.h"
#include "raymath.h"
#include <string>
#include <iostream>
#include <cmath>

class InputField
{
    public:
        int width;
        int height;
        int borderWidth;

        std::string fillerText;
        std::string text;

        int textSize;

        Color backColor;
        Color borderColor;

        Color selectedBackColor;
        Color selectedBorderColor;

        Color textColor;
        Color fillerTextColor;
        Color selectedFillerTextColor;

    private:

        bool isSelected = false;
        int cursorPosition = 0;
        int scrollOffset = 0;

    public:

    InputField(int width, int height, int borderWidth, std::string fillerText, int textSize, Color backColor = DARKGRAY, Color borderColor = GRAY, Color selectedBackColor = GRAY, Color selectedBorderColor = LIGHTGRAY, Color textColor = WHITE, Color fillerTextColor = GRAY, Color selectedFillerTextColor = LIGHTGRAY)
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

        this->textColor = textColor;
        this->fillerTextColor = fillerTextColor;
        this->selectedFillerTextColor = selectedFillerTextColor;
    }

    void Draw(int x, int y)
    {
        //get rectnagle field
        Rectangle field = {(float)x, (float)y, (float)width, (float)height};

        //check for mouse sleection
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        {isSelected = CheckCollisionPointRec(GetMousePosition(), field);}

        //key inputs - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        if (isSelected)
        {
            int key = GetCharPressed();

            //use while loop for quueueing new characters
            while (key > 0)
            {
                //check wheter keycode is within printable characters
                if (key >= 32 && key <= 126)
                {
                    //insert within cursor pos and increment cursor pos
                    text.insert(text.begin() + cursorPosition, static_cast<char>(key));
                    cursorPosition++;
                }

                //add any other potential characters into the queue
                key = GetCharPressed();
            }

            //backspace and delete (remove from left/right of the cursor)
            if (IsKeyPressed(KEY_BACKSPACE) && cursorPosition > 0)
            {text.erase(cursorPosition - 1, 1); cursorPosition--;}
            if (IsKeyPressed(KEY_DELETE) && cursorPosition < (int)text.length())
            {text.erase(cursorPosition, 1);}

            //left and right
            if (IsKeyPressed(KEY_LEFT) && cursorPosition > 0)
            {cursorPosition--;}
            if (IsKeyPressed(KEY_RIGHT) && cursorPosition < (int)text.length())
            {cursorPosition++;}
        }

        //text scrolling  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        //get space available
        int textAreaWidth = width - borderWidth * 2 - 10;

        //text widht left of cursor
        std::string beforeCursor = text.substr(0, cursorPosition);
        int cursorX = MeasureText(beforeCursor.c_str(), textSize);

        //adjust crolloffset so the cursor is always visible
        if (cursorX - scrollOffset > textAreaWidth)
        {scrollOffset = cursorX - textAreaWidth;}

        if (cursorX - scrollOffset < 0)
        {scrollOffset = cursorX;}

        //clamp to start
        if (scrollOffset < 0)
        {scrollOffset = 0;}

        //draw elements  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        //draw backdrop and border
        Color currentBackColor = isSelected ? selectedBackColor : backColor;
        Color currentBorderColor = isSelected ? selectedBorderColor : borderColor;

        DrawRectangleRec(field, currentBackColor);
        DrawRectangleLinesEx(field, borderWidth, currentBorderColor);

        //draw text
        int textX = x + borderWidth + 5;
        int textY = y + (height - textSize) / 2;

        //use Scissor mode for draws to be strictly visible within a rect
        BeginScissorMode(x + borderWidth, y + borderWidth, width - borderWidth * 2, height - borderWidth * 2);

        //filler text
        if (text.empty())
        {
            if (isSelected)
            {DrawText(fillerText.c_str(), textX, textY, textSize, selectedFillerTextColor);}
            else
            {DrawText(fillerText.c_str(), textX, textY, textSize, fillerTextColor);}
        }
        //acctualy text
        else
        {DrawText(text.c_str(), textX - scrollOffset, textY, textSize, textColor);}

        //draw blinking cursor line
        if (isSelected)
        {
            //blink functionality (timing)
            if (((int)(GetTime() * 2) % 2) == 0)
            {
                int cursorScreenX = textX + cursorX - scrollOffset;
                DrawLine(cursorScreenX, y + borderWidth + 5, cursorScreenX, y + height - borderWidth - 5, textColor);
            }
        }

        EndScissorMode();
    }

    //helper funcs
    std::string GetText()
    {return text;}

    void Clear()
    {
        text.clear();
        cursorPosition = 0;
        scrollOffset = 0;
    }

    bool IsSelected()
    {return isSelected;}
};