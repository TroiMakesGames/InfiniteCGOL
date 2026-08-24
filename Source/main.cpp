#include "raylib.h"
#include "raymath.h"

#include <vector>
#include <iostream>
#include <cmath>
#include <random>

#include <unordered_set>
#include <unordered_map>

//dynamic controlable viewport
#include "DynamicCam.h"

//helper class for storing singel cell data in one unordered_set
//structs instead of classes for defaulting to public
struct Coord
{
    int x;
    int y;

    //operand for comparing raw value  copies
    bool operator==(const Coord& other) const
    {return x == other.x && y == other.y;}
};

//unordered_set uses an object type and a hashing function to use on the object type
//  therefore the CoordHash struct containes a defenition of itself being used as a function (operator()(params))
struct CoordHash
{
    //size_t - unsigned integer dedicated to stuff like sizes, indexes and hash values
    std::size_t operator()(const Coord& coord) const        //the "const" in the end serves as a "promise" that Coord&, even tho it is added as a reference ("&"), wont be modified 
    {
        std::size_t hashX = std::hash<int>{}(coord.x);
        std::size_t hashY = std::hash<int>{}(coord.y);

        //some crazy bit shift math and binary logic magic (basicaly hashX + hashY)
        return hashX ^ (hashY << 1);
    }
};

class Grid
{
    public:
        std::unordered_set<Coord, CoordHash> liveCells;

    private:
        //neighbor offsets
        const Coord neighbors[8] = {
            {-1, -1}, { 0, -1}, { 1, -1},
            {-1,  0},           { 1,  0},
            {-1,  1}, { 0,  1}, { 1,  1}
        };

    public:
    Grid()
    {
        //nothing here lol...
    }

    //random gen
    void randomize(int minX, int maxX, int minY, int maxY)
    {
        //clear previous
        liveCells.clear();

        //get random seed and get initialise a generator using the seed
        std::random_device rd;
        std::mt19937 gen(rd());

        //generate a distribution object (int array of uniform possibility 1 or 0)
        std::uniform_int_distribution<int> distribution(0, 1);

        for (int y = minY; y <= maxY; y++)
        {
            for (int x = minX; x <= maxX; x++)
            {
                //check if the distribution of the next call of gen is 1 or 0
                //  (calling a reference to gen returns a generated value and moves the pointer forward for next call return value)
                if (distribution(gen) == 1)
                {liveCells.insert({x, y});}
            }
        }
    }

    //helper func to see if a coord is inside the live cells
    bool isAlive(const Coord& coord) const
    {return liveCells.contains(coord);}

    void updateGrid()
    {
        //count live neighbors
        /*
        the way this is done is WAY different from the previous implementations
        since we dont have acces to neighbors as they dont exist as live cells and we dont have a 2d grid of bools
        we need to change up the neighbor counting method

        instead of counting neighbors for each live cell, we tell each neighbor of a live cell that it has an additional live neigbhor

        go through all live cells
            go through all neighbors
                get the neighbor as a new coord instance 
                add it a map/dict
                increment its count of live neighbors by 1 (current proccessed cell)
        
        when checking for rules we use all of the coords inside the map/dict, which includes dead neighbors of live cells
        which measn that new cells can be spanwed from dead cells that neighbor enough live cells
        */
        
        //initialise incrementation map
        std::unordered_map<Coord, int, CoordHash> neighborCounts;
        neighborCounts.reserve(liveCells.size() * 8);
        
        //go through live cells and neighbor offsets
        for (const Coord& cell : liveCells)
        {
            for (const Coord& offset : neighbors)
            {
                //add new possibly live coord to the map (if was added before it gets skipped automaticaly), increment it
                Coord neighbor ={cell.x + offset.x, cell.y + offset.y};
                neighborCounts[neighbor]++;
            }
        }

        //get new vars and check rules
        std::unordered_set<Coord, CoordHash> newLiveCells;
        newLiveCells.reserve(neighborCounts.size());

        //go through neighbor counts map
        for (const auto& [coord, liveCount] : neighborCounts)
        {
            //check if the neigbhor coord was in fact previously alive
            bool currentlyAlive = liveCells.contains(coord);

            if (liveCount == 3)
            {newLiveCells.insert(coord);}
            else if (currentlyAlive && liveCount == 2)
            {newLiveCells.insert(coord);}
        }

        //replace old with new
        liveCells = std::move(newLiveCells);
    }
};

void drawCells(Viewport viewport, Grid& grid, float cellSize)
{
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    {
        //get mouse pos in  the world coords ... in mouseCell + cellSize/2 is added to fix rounding offset errors common for grid drawing like that
        Vector2 mouseWorldPos = viewport.screenToWorldPos(GetMousePosition());
        Coord mouseCell = {static_cast<int>(floor((mouseWorldPos.x + cellSize/2) / cellSize)), static_cast<int>(floor((mouseWorldPos.y + cellSize/2) / cellSize))};

        //check if cell is alive or dead for toggling
        if (grid.liveCells.contains(mouseCell))
        {grid.liveCells.erase(mouseCell);}
        else
        {grid.liveCells.insert(mouseCell);}
    }
}

int main() {
    //screen initialisation
    const int WIDTH = 1000;
    const int HEIGHT = 800;
    InitWindow(WIDTH, HEIGHT, "Infinite Conway's Game of Life");
    SetTargetFPS(60);

    //variable initialisation
    Viewport viewport = Viewport(Vector2(0, 0), 10, 0.05f, 0.05f, 5);

    bool isPaused = true;
    Grid grid = Grid();
    grid.randomize(-50, 50, -50, 50);
    float cellSize = 10.0f;

    //adjust viewport zoom so that on start it doesnt draw cells too large
    viewport.zoom = 0.35f;

    //controls text
    std::vector<std::string> controlText = {
        "Click to draw/erase",
        "Scroll to zoom",
        "WASD to move",
        "Press P to pause"
    };

    //while loop
    while (!WindowShouldClose())
    {
        //update
        viewport.move();
        viewport.zoomCamera();

        //check for pausing
        if (IsKeyPressed(KEY_P)) {isPaused = !isPaused;}
        if (!isPaused)
        {grid.updateGrid();}

        //get user drawing
        drawCells(viewport, grid, cellSize);

        // draw
        BeginDrawing();
        ClearBackground(Color{30, 30, 30, 255});

        //draw rectangles
        for (const Coord& cell : grid.liveCells)
        {
            //get cell worldpos as float cast
            Vector2 worldPos = {static_cast<float>(cell.x * cellSize), static_cast<float>(cell.y * cellSize)};

            //convert to scren pos
            Vector2 screenPos = viewport.worldToScreenPos(worldPos);

           //get screenscale
            Vector2 screenSize = viewport.worldToScreenScale({cellSize, cellSize});

            //draw cell (DrawRectangleV() uses vector2)
            DrawRectangleV({screenPos.x - screenSize.x / 2.0f, screenPos.y - screenSize.y / 2.0f}, screenSize, Color(255, 255, 255, 255));
        }

        //text drawing
        DrawText(TextFormat("%i", GetFPS()), 10, 14, 20, Color{0, 0, 0, 100});
        DrawText(TextFormat("%i", GetFPS()), 10, 11, 20, Color{0, 150, 0, 255});
        DrawText(TextFormat("%i", GetFPS()), 10, 10, 20, Color{0, 255, 0, 255});

        //draw control text
        for (int i = 0; i < controlText.size(); i++)
        {
            DrawText(controlText[i].c_str(), 10, HEIGHT - (i+1) * 15 + 3 - 10, 10, Color{0, 0, 0, 100});
            DrawText(controlText[i].c_str(), 10, HEIGHT - (i+1) * 15 + 1 - 10, 10, Color{0, 150, 0, 255});
            DrawText(controlText[i].c_str(), 10, HEIGHT - (i+1) * 15 + 0 - 10, 10, Color{0, 255, 0, 255});
        }

        EndDrawing();
    }
    CloseWindow();
    return 0;
}