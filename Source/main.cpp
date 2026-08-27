#include "raylib.h"
#include "raymath.h"

#include <vector>
#include <iostream>
#include <cmath>
#include <random>

#include <unordered_set>
#include <unordered_map>

//include personal external tools
#include "DynamicCam.h"
#include "InputField.h"
#include "Button.h"

//fps timeline
#include <deque>

//saving system
#include <filesystem>
#include <fstream>
#include <limits>
//#include iostream (already included)

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

/* saving system funcs */

//check whether the input field contents are a valid filename
bool IsValidFilename(const std::string& filename)
{return !filename.empty() && filename.find_first_of("<>:\"/\\|?*") == std::string::npos;}

//save world data to the entered save file
bool SaveWorld(const std::string& filename, const Viewport& viewport, const Grid& grid)
{
    //open file
    std::ofstream file(filename);
    if (!file.is_open())
    {std::cout << "Failed to open file!\n"; return false;}

    //save camera pos and zoom
    file << "cpos " << viewport.worldPos.x << " " << viewport.worldPos.y << "\n";
    file.flush();
    file << "czum " << viewport.zoom << "\n";
    file.flush();

    //save live cells
    int count = 0;
    for (const Coord& cell : grid.liveCells)
    {
        file << "lvcl " << cell.x << " " << cell.y << "\n";
        count++;

        if (count % 100 == 0) 
        {file.flush();}
    }

    file.close();
    return true;    //coomunicate successfull save
}

//load world data from the entered save file
bool LoadWorld(const std::string& filename, Viewport& viewport, Grid& grid)
{
    //open file
    std::ifstream file(filename);
    if (!file.is_open())
    {std::cout << "Failed to open save file!\n"; return false;}

    //clear current world
    grid.liveCells.clear();

    //go through each line of data and procces depending on the type
    std::string type;
    while (file >> type)
    {
        if (type == "cpos")
        {file >> viewport.worldPos.x >> viewport.worldPos.y;}

        else if (type == "czum")
        {file >> viewport.zoom;}

        else if (type == "lvcl")
        {
            Coord cell;
            file >> cell.x >> cell.y;
            grid.liveCells.insert(cell);
        }
        else
        {
            //skip the rest of the line
            std::cout << "Unknown save data: " << type << "\n";
            file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
    }

    file.close();
    return true;    //coomunicate successfull load
}

/* benchmark history structs */

//helper struct to track fps history samples
struct FpsSample
{
    int fps;
    float time;
};

//helper struct to track live cell count history
struct CellCountSample
{
    int cellCount;
    float time;
};

int main() 
{
    std::cout << "Working directory: " << std::filesystem::current_path() << "\n";

    //screen initialisation
    const int WIDTH = 1000;
    const int HEIGHT = 800;
    InitWindow(WIDTH, HEIGHT, "Infinite Conway's Game of Life");

    int targetFps = 60;
    bool limitFps = true;
    SetTargetFPS(targetFps);

    //tiny fps timeline
    std::deque<FpsSample> fpsHistory;
    int timelineWidth = 100;
    int timelineHeight = 80;
    int historyLength = 3;  //in seconds
    Vector2 timelinePos = Vector2(10, 10);

    //tiny cell count timeline
    std::deque<CellCountSample> cellCountHistory;
    int cellTimelineWidth = 100;
    int cellTimelineHeight = 80;
    int cellHistoryLength = 3;  //in seconds
    Vector2 cellTimelinePos = Vector2(115, 10);

    //variable initialisation
    Viewport viewport = Viewport(Vector2(0, 0), 10, 0.05f, 0.05f, 5);

    bool isPaused = true;
    Grid grid = Grid();
    grid.randomize(-50, 50, -50, 50);
    float cellSize = 10.0f;

    //adjust viewport zoom so that on start it doesnt draw cells too large
    viewport.zoom = 0.35f;

    //save file UI
    //colors: backColor borderColor selectedBackColor selectedBorderColor textColor fillerTextColor selectedFillerTextColor
    InputField sv_inputField = InputField(200, 40, 2, "SaveFile.txt", 15, Color{0, 0, 0, 100}, Color(255, 255, 255, 255), Color{0, 0, 0, 200}, Color{255, 255, 255, 255}, Color{255, 255, 255, 255}, Color{200, 200, 200, 255}, Color{255, 255, 255, 255});
    Button sv_saveButton = Button(97, 40, 2, "Save to", 15, Color{0, 0, 0, 100}, Color(0, 255, 0, 255), Color{0, 0, 0, 50}, Color{0, 150, 0, 255}, Color{0, 255, 0, 255}, Color{0, 150, 0, 255});
    Button sv_loadButton = Button(97, 40, 2, "Load from", 15, Color{0, 0, 0, 100}, Color(255, 0, 0, 255), Color{0, 0, 0, 50}, Color{150, 0, 0, 255}, Color{255, 0, 0, 255}, Color{150, 0, 0, 255});

    //controls text
    bool displayInterface = true;
    std::vector<std::string> controlText = {
        "Left Click - draw/erase",
        "Scroll - zoom",
        "WASD - move",
        "P - pause",
        "L - toggle FPS limit",
        "T - toggle interface"
    };

    //while loop
    while (!WindowShouldClose())
    {
        //check if input field in vefore obeying key toggles
        bool isInInputField = sv_inputField.IsSelected();

        //check fps limit toggle
        if (!isInInputField)
        {
            if (IsKeyPressed(KEY_L)) 
            {
                limitFps = !limitFps;

                if (limitFps) 
                {SetTargetFPS(targetFps);}
                else 
                {SetTargetFPS(0);}
            }
        }

        //update
        if (!isInInputField)
        {viewport.move(GetFrameTime());}
        viewport.zoomCamera();

        //check for pausing
        if (!isInInputField)
        {if (IsKeyPressed(KEY_P)) {isPaused = !isPaused;}}

        if (!isPaused)
        {grid.updateGrid();}

        //get user drawing
        //dont draw over UI elements
        Vector2 mousePos = GetMousePosition();
        if (mousePos.x < WIDTH - 200 - 5 - 5 || mousePos.y > 140)
        {drawCells(viewport, grid, cellSize);}

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

        //update fps history
        int currFps = GetFPS();
        float currTime = GetTime();
        fpsHistory.push_back({currFps, currTime});
        //remnove outdated samples
        while (!fpsHistory.empty() && currTime - fpsHistory.front().time > historyLength)
        {fpsHistory.pop_front();}

        //update cell count history
        int currCellCount = grid.liveCells.size();
        cellCountHistory.push_back({currCellCount, currTime});
        //remnove outdated samples
        while (!cellCountHistory.empty() && currTime - cellCountHistory.front().time > cellHistoryLength)
        {cellCountHistory.pop_front();}

        //text drawing + interface toggle
        if (!isInInputField)
        {
            if (IsKeyPressed(KEY_T)) 
            {displayInterface = !displayInterface;}
        }
        if (displayInterface)
        {
            //draw control text
            for (int i = 0; i < controlText.size(); i++)
            {
                DrawText(controlText[i].c_str(), 10, HEIGHT - (i+1) * 15 + 3 - 10, 10, Color{0, 0, 0, 100});
                DrawText(controlText[i].c_str(), 10, HEIGHT - (i+1) * 15 + 1 - 10, 10, Color{150, 150, 150, 255});
                DrawText(controlText[i].c_str(), 10, HEIGHT - (i+1) * 15 + 0 - 10, 10, Color{255, 255, 255, 255});
            }

            //draw fps history timeline
            DrawRectangle(timelinePos.x, timelinePos.y, timelineWidth, timelineHeight, Color{0, 0, 0, 50});
            if (!fpsHistory.empty())
            {
                //get maximum for height tacking
                int localMaximum = fpsHistory[0].fps;
                for (const FpsSample& sample : fpsHistory)
                {
                    if (sample.fps > localMaximum)
                    {localMaximum = sample.fps;}
                }

                //do some math for correct graph representation;
                float heightPerSingleFps = (float)timelineHeight / (float)localMaximum;
                float oldestTime = currTime - historyLength;

                for (size_t i = 0; i < fpsHistory.size() - 1; i++)
                {
                    FpsSample sample1 = fpsHistory[i];
                    FpsSample sample2 = fpsHistory[i + 1];

                    //convert time to  position
                    float x1 = ((sample1.time - oldestTime) / historyLength) * timelineWidth;
                    float x2 =((sample2.time - oldestTime) / historyLength) * timelineWidth;

                    //convert fps to height
                    float y1 = timelineHeight - heightPerSingleFps * sample1.fps;
                    float y2 = timelineHeight - heightPerSingleFps * sample2.fps;

                    DrawLine(
                        timelinePos.x + x1,
                        timelinePos.y + y1,
                        timelinePos.x + x2,
                        timelinePos.y + y2,
                        Color{0, 255, 0, 255}
                    );
                }
            }

            //draw fps aftere timeline
            DrawText(TextFormat("FPS: %i", currFps), timelinePos.x + 5, timelinePos.y + timelineHeight - 15 + 4, 10, Color{0, 0, 0, 100});
            DrawText(TextFormat("FPS: %i", currFps), timelinePos.x + 5, timelinePos.y + timelineHeight - 15 + 1, 10, Color{0, 150, 0, 255});
            DrawText(TextFormat("FPS: %i", currFps), timelinePos.x + 5, timelinePos.y + timelineHeight - 15 + 0, 10, Color{0, 255, 0, 255});

            //draw cell count history timeline
            DrawRectangle(cellTimelinePos.x, cellTimelinePos.y, cellTimelineWidth, cellTimelineHeight, Color{0, 0, 0, 50});
            if (!cellCountHistory.empty())
            {
                //get maximum for height tacking
                int localMaximum = cellCountHistory[0].cellCount;
                for (const CellCountSample& sample : cellCountHistory)
                {
                    if (sample.cellCount > localMaximum)
                    {localMaximum = sample.cellCount;}
                }

                //do some math for correct graph representation;
                float heightPerSingleCount = (float)cellTimelineHeight / (float)localMaximum;
                float oldestTime = currTime - cellHistoryLength;

                for (size_t i = 0; i < cellCountHistory.size() - 1; i++)
                {
                    CellCountSample sample1 = cellCountHistory[i];
                    CellCountSample sample2 = cellCountHistory[i + 1];

                    //convert time to  position
                    float x1 = ((sample1.time - oldestTime) / cellHistoryLength) * cellTimelineWidth;
                    float x2 =((sample2.time - oldestTime) / cellHistoryLength) * cellTimelineWidth;

                    //convert cellcount to height
                    float y1 = cellTimelineHeight - heightPerSingleCount * sample1.cellCount;
                    float y2 = cellTimelineHeight - heightPerSingleCount * sample2.cellCount;

                    DrawLine(
                        cellTimelinePos.x + x1,
                        cellTimelinePos.y + y1,
                        cellTimelinePos.x + x2,
                        cellTimelinePos.y + y2,
                        Color{255, 255, 0, 255}
                    );
                }
            }

            //draw active cell count after timeline
            DrawText(TextFormat("Live cells: %i", currCellCount), cellTimelinePos.x + 5, cellTimelinePos.y + cellTimelineHeight - 15 + 4, 10, Color{0, 0, 0, 100});
            DrawText(TextFormat("Live cells: %i", currCellCount), cellTimelinePos.x + 5, cellTimelinePos.y + cellTimelineHeight - 15 + 1, 10, Color{150, 150, 0, 255});
            DrawText(TextFormat("Live cells: %i", currCellCount), cellTimelinePos.x + 5, cellTimelinePos.y + cellTimelineHeight - 15 + 0, 10, Color{255, 255, 0, 255});

            //draw viewport position and mouse potiion
            int viewpportX = (int)std::round(viewport.worldPos.x);
            int viewpportY = (int)std::round(viewport.worldPos.y);
            DrawText(TextFormat("Camera position: %d, %d", viewpportX, viewpportY), 10, 95, 10, Color{0, 0, 0, 100});
            DrawText(TextFormat("Camera position: %d, %d", viewpportX, viewpportY), 10, 95, 10, Color{150, 0, 0, 255});
            DrawText(TextFormat("Camera position: %d, %d", viewpportX, viewpportY), 10, 95, 10, Color{255, 0, 0, 255});

            Vector2 mouseWorldPos = viewport.screenToWorldPos(GetMousePosition());
            Coord mouseCell = {static_cast<int>(floor((mouseWorldPos.x + cellSize/2) / cellSize)), static_cast<int>(floor((mouseWorldPos.y + cellSize/2) / cellSize))};
            int mX = mouseCell.x;
            int mY = mouseCell.y;
            DrawText(TextFormat("Mouse position: %d, %d", mX, mY), 10, 105, 10, Color{0, 0, 0, 100});
            DrawText(TextFormat("Mouse position: %d, %d", mX, mY), 10, 105, 10, Color{0, 75, 150, 255});
            DrawText(TextFormat("Mouse position: %d, %d", mX, mY), 10, 105, 10, Color{0, 127, 255, 255});
        }

        //draw UI elements
        if (displayInterface)
        {
            sv_inputField.Draw(WIDTH - 200 - 5, 5);
            sv_saveButton.Draw(WIDTH - 200 - 5, 5 + 40 + 5);
            sv_loadButton.Draw(WIDTH - 200 - 5 + 103, 5 + 40 + 5);
        }

        //get save system inputs
        //save data
        if (sv_saveButton.IsPressedFirstFrame())
        {
            //get input field save file name and check validity
            std::string filename = sv_inputField.GetText();
            if (IsValidFilename(filename))
            {SaveWorld(filename, viewport, grid);}
        }

        //load data
        if (sv_loadButton.IsPressedFirstFrame())
        {
            //get input field save file name and check validity
            std::string filename = sv_inputField.GetText();
            if (IsValidFilename(filename))
            {LoadWorld(filename, viewport, grid);}
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}