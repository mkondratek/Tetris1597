#include <stdlib.h>
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <memory>
#include <iostream>
#include <fstream>
#include "Tile.h"

using namespace sf;

//sizes
int width = 9;
int height = 12;

int tileSize = 45;
int tilePadding = 4;
int txtPadding = 4;
int ableLevel;

int winWIDTH;
int winHEIGHT;

//color set
Color clrTile(26, 26, 26);
Color clrNum(230, 0, 0, 200);
Color clrPlay(89, 89, 89);
Color clrBackground(79, 79, 79);

//init static members of Tile
int Tile::tileSize = 0;
int Tile::padding = 0;
Font Tile::font = Font();
Color Tile::clrTxt = Color();

//board init
std::vector <std::vector<std::shared_ptr <Tile>>> board;
std::shared_ptr <Tile> currentShape[4];

unsigned long long int maxValue();
void cleanBoard();
void rotate(point <int> *arr);
void collapseToLeft(bool&);
void collapseToRight(bool&);
void createBoard() ;
void loadConfig() ;

//shapes map
int shape[7][4] {
    -3, -2, 0, 2,
    -3, 1, 0, 2,
    -2, 2, 0, 4,
    3, -2, 0, 2,
    1, 3, 0, -2,
    1, -2, 0, 2,
    1, 3, 0, 2
};

//spawning point
point <int> start;

//controls
std::string controlsStr = " CONTROLS\n    P pause\n   R restart\n     arrows\nctrl + arrows";

Font font;

//texts
Text pauseTxt;
Text defeatTxt;
Text wonTxt;
Text coinsTxt;
Text controlsTxt;
Text signTxt;
//-----

RenderWindow renderWindow;
RectangleShape playField;
RectangleShape monoTile;

Sound music;
SoundBuffer buffer;

int main() {
    srand((unsigned int) time(NULL));
    loadConfig();

    music.setLoop(true);
    music.setPitch(0.8);

    playField.setFillColor(clrPlay);
    monoTile.setFillColor(clrTile);

    bool ctrlPressed;
    bool spawning = true;
    bool paused = true;
    bool defeat = false;
    bool won = false;
    double tickFactor = 1;
    float gameTime = 0;
    float CurrentTime = 0;
    float timer = 0;
    float tick;
    Clock clock;

    bool rotating = false;
    int dx = 0, dy = 1;

    unsigned long long int highestValue;
    int fallingSpeed = 5;
    int coins = 0;
    int type = 0;
    point <int> currPosition[4];

    buffer.loadFromFile("resources/Tetris.ogg");
    music.setBuffer(buffer);

    while (renderWindow.isOpen()) {
        Event event;

        CurrentTime = clock.getElapsedTime().asSeconds();
        clock.restart();
        if (!paused && !defeat) {
            timer += CurrentTime;
            gameTime += CurrentTime;
        }
        tick = (float) (0.08*tickFactor);

        while (renderWindow.pollEvent(event)) {
            if (event.type == Event::Closed) {
                renderWindow.close();
            }
            else if (event.type == Event::KeyPressed) {
                ctrlPressed = Keyboard::isKeyPressed(Keyboard::LControl);
                if (!paused && !defeat && !won) {
                    if (event.key.code == Keyboard::Left) {
                        if (ctrlPressed && coins > 9) {
                            collapseToLeft(won);
                            coins -= 10;
                            coinsTxt.setString("COINS\n" + std::to_string(coins));
                        }
                        else dx = -1;
                    }
                    else if (event.key.code == Keyboard::Right) {
                        if (ctrlPressed && coins > 9) {
                            collapseToRight(won);
                            coins -= 10;
                            coinsTxt.setString("COINS\n" + std::to_string(coins));
                        }
                        else dx = 1;
                    }
                    else if (event.key.code == Keyboard::Up) rotating = true;
                    else if (event.key.code == Keyboard::Down) {fallingSpeed = 2; timer++;}
                    else if (event.key.code == Keyboard::S) spawning = !spawning;
                }
                if (event.key.code == Keyboard::P && !won && !defeat) {
                    paused = !paused;
                    if (!paused) music.play();
                    else music.pause();
                }
                else if (event.key.code == Keyboard::R) {
                    if (ctrlPressed) loadConfig();
                    else cleanBoard();
                    won = defeat = false;
                    paused = true;
                    coins = 0;
                    gameTime = 0;
                    music.stop();
                    music.setPitch(0.8);
                    coinsTxt.setString("COINS\n" + std::to_string(coins));
                }
            }
        }

        //calculate and move things
        if (timer > tick) {
            if (currentShape[0] == nullptr && spawning) {
                ableLevel = 0;
                type = rand()%7;
                highestValue = maxValue();
                unsigned long long int n = (rand() % highestValue);

                for (int i=0; i<4; ++i) {
                    currentShape[i] = std::make_shared<Tile>(renderWindow, n);
                    int x = start.x + shape[type][i] / 2;
                    int y = start.y + abs(shape[type][i] % 2);
                    if (board[x][y] == nullptr) board[x][y] = currentShape[i];
                    else defeat = true;

                    currPosition[i].x = x;
                    currPosition[i].y = y;
                    ableLevel = std::max(ableLevel, y + 1);
                }
            }

            if (currentShape[0] != nullptr && !defeat) {
                point <int> goalPosition[4];
                bool collision = false;
                bool wall = false;

                //rotation
                if (rotating) {
                    //copy array
                    for (int i=0; i<4; ++i) goalPosition[i] = currPosition[i];
                    rotate(goalPosition);

                    for (int i = 0; i < 4; ++i) {
                        int x = goalPosition[i].x;
                        int y = goalPosition[i].y;

                        //any collision
                        collision = (x < 0 || x >= width || y < 0 || wall || y >= height)
                                    || (board[wall?currPosition[i].x:goalPosition[i].x][y] != nullptr
                                        && !board[wall?currPosition[i].x:goalPosition[i].x][y]->falling());
                        if (collision) break;
                    }

                    if (!collision) {
                        //release previous position
                        for (int i = 0; i < 4; ++i) {
                            board[currPosition[i].x][currPosition[i].y] = nullptr;
                        }

                        //obtain new position
                        for (int i = 0; i < 4; ++i) {
                            board[wall?currPosition[i].x:goalPosition[i].x][goalPosition[i].y] = currentShape[i];
                            currPosition[i].x = wall?currPosition[i].x:goalPosition[i].x;
                            currPosition[i].y = goalPosition[i].y;
                            ableLevel = std::max(ableLevel, currPosition[i].y+1);
                        }
                    }
                }

                collision = false;

                //arrows and falling
                for (int i=0; i<4; ++i) {
                    int x = currPosition[i].x;
                    int y = currPosition[i].y;

                    goalPosition[i].x = (x += dx);
                    goalPosition[i].y = (y += (dy == 1));

                    //side collision
                    wall = (x < 0 || x >= width || y < 0 || wall || y >= height
                            || (board[wall?currPosition[i].x:goalPosition[i].x][y] != nullptr
                                && !board[wall?currPosition[i].x:goalPosition[i].x][y]->falling()));
                    //ground/another brick int he wall collision
                    collision = (y >= height)
                                || (board[wall?currPosition[i].x:goalPosition[i].x][y] != nullptr
                                    && !board[wall?currPosition[i].x:goalPosition[i].x][y]->falling());
                    if (collision) break;
                }

                if (!collision) {
                    //release previous position
                    for (int i = 0; i < 4; ++i) {
                        board[currPosition[i].x][currPosition[i].y] = nullptr;
                    }

                    //obtain new position
                    for (int i = 0; i < 4; ++i) {
                        board[wall?currPosition[i].x:goalPosition[i].x][goalPosition[i].y] = currentShape[i];
                        currPosition[i].x = wall?currPosition[i].x:goalPosition[i].x;
                        currPosition[i].y = goalPosition[i].y;
                        ableLevel = std::max(ableLevel, currPosition[i].y+1);
                    }
                }
                else {
                    //stop things falling
                    for (int i = 0; i < 4; ++i) {
                        currentShape[i]->stop();
                        currentShape[i] = nullptr;
                    }
                    ableLevel = 1;
                }
            }

            //reset input
            dx = 0;
            dy = 1 + dy % fallingSpeed;
            fallingSpeed = 5;
            timer = 0;
            rotating = false;

            //update things
            if (gameTime < 400) tickFactor = (1 - 0.002 * gameTime);
            if (gameTime < 500) music.setPitch((float) (0.8 + 0.002 * gameTime));

            SoundStream::Chunk chunk;
            chunk.samples = buffer.getSamples();
            Int16 a = 0, b = 0;
            for (int i=0; i<chunk.sampleCount; ++i) {
                Int16 tmp = chunk.samples[i];
                if (tmp > a) a = tmp;
                else if (tmp < b) b = tmp;
            }
        }

        //collapse full row
        for (int i=0; i<height; ++i) {
            int tmpcoins = 0;
            int count = 0;
            for (int j=0; j<width; ++j) {
                if (board[j][i] != nullptr && !board[j][i]->falling()) {
                    tmpcoins += board[j][i]->getNum();
                    count++;
                }
                else break;
            }
            if (count == width) {
                coins += tmpcoins + count;
                coinsTxt.setString("COINS\n" + std::to_string(coins));
                for (int k=i; k>0; --k) {
                    for (int j = 0; j < width; ++j) {
                        board[j][k] = board[j][k - 1];
                    }
                }
            }
        }

        renderWindow.clear(clrBackground);

        renderWindow.draw(playField);

        //draw tiles
        for (int i=0; i<width; ++i) {
            for (int j=0; j<height; ++j) {
                if (board[i][j] != nullptr) {
                    float x = i * tileSize + tileSize / 2;
                    float y = j * tileSize + tileSize / 2;
                    monoTile.setPosition(x, y);
                    monoTile.setFillColor(getColorFor(board[i][j]->getNum()));
                    renderWindow.draw(monoTile);
                    board[i][j]->draw(x, y);
                }
            }
        }

        if (won) renderWindow.draw(wonTxt);
        else if (defeat) renderWindow.draw(defeatTxt);
        else if (paused) renderWindow.draw(pauseTxt);

        renderWindow.draw(coinsTxt);
        renderWindow.draw(controlsTxt);
        renderWindow.draw(signTxt);

        renderWindow.display();
    }

    cleanBoard();

    return 0;
}

unsigned long long int maxValue() {
    unsigned long long int x = 1;
    for (int i=0; i<width; ++i) {
        for (int j=0; j<height; ++j) {
            if (board[i][j] != nullptr) {
                x = std::max(x, board[i][j]->getNum());
            }
        }
    }
    return x;
}

void cleanBoard() {
    for (int i=0; i<board.size(); ++i) {
        for (int j=0; j<board[0].size(); ++j) {
            board[i][j] = nullptr;
        }
    }
    for (int i=0; i<4; ++i) currentShape[i] = nullptr;
}

void createBoard() {
    cleanBoard();
    board.clear();

    for (int i=0; i<width; ++i) {
        board.push_back(std::vector<std::shared_ptr<Tile>>());
        for (int j=0; j<height; ++j) {
            board[i].push_back(std::shared_ptr<Tile>());
        }
    }
}

void rotate(point<int> *arr) {
    point <int> center = arr[2];
    for (int i=0; i<4; ++i) {
        int x = arr[i].y - center.y;
        int y = arr[i].x - center.x;
        arr[i].x = center.x - x;
        arr[i].y = center.y + y;
    }
}

void collapseToLeft(bool &won) {
    for (int i=ableLevel; i<height; ++i) {
        //join
        for (int j=0; j<width-1; ++j) {
            if (board[j][i] != nullptr && !board[j][i]->falling()) {
                for (int k=j+1; k<width; ++k) {
                    if (board[k][i] != nullptr && !board[k][i]->falling()
                        && board[j][i]->getNum() == board[k][i]->getNum()) {
                        board[j][i]->setNum(board[j][i]->getNum() + 1);
                        if (board[j][i]->getNum() == 15) won = true;
                        board[k][i] = nullptr;
                        break;
                    }
                }
            }
        }
        //move to left
        for (int j=0; j<width-1; ++j) {
            if (board[j][i] != nullptr) continue;
            for (int k=j+1; k<width; ++k) {
                if (board[k][i] != nullptr && !board[k][i]->falling()) {
                    board[j][i] = board[k][i];
                    board[k][i] = nullptr;
                    break;
                }
            }
        }
    }
}

void collapseToRight(bool &won) {
    for (int i=ableLevel; i<height; ++i) {
        //join
        for (int j=width-1; j>0; --j) {
            if (board[j][i] != nullptr && !board[j][i]->falling()) {
                for (int k=j-1; k>=0; --k) {
                    if (board[k][i] != nullptr && !board[k][i]->falling()
                        && board[j][i]->getNum() == board[k][i]->getNum()) {
                        board[j][i]->setNum(board[j][i]->getNum() + 1);
                        if (board[j][i]->getNum() == 15) won = true;
                        board[k][i] = nullptr;
                        break;
                    }
                }
            }
        }
        //move to right
        for (int j=width-1; j>0; --j) {
            if (board[j][i] != nullptr) continue;
            for (int k=j-1; k>=0; --k) {
                if (board[k][i] != nullptr && !board[k][i]->falling()) {
                    board[j][i] = board[k][i];
                    board[k][i] = nullptr;
                    break;
                }
            }
        }
    }
}

void placeText(RectangleShape&, Text&, int);

void placeText(Vector2f rect_pos, Vector2f rect_size, Text &txt, int padding = 0) {
    RectangleShape tmp(rect_size);
    tmp.setPosition(rect_pos);
    placeText(tmp, txt, padding);
}

void placeText(RectangleShape &rect, Text &txt, int padding = 0) {
    Vector2f rect_pos = rect.getPosition();
    Vector2f rect_size = rect.getSize();

    while (rect_size.x - 10.0f < padding * 2.0f) padding--;
    float value = txt.getLocalBounds().width - rect_size.x + 2.0f * padding;
    float prevVal = 0, prevPrevVal = 0;

    while (std::abs(value) > 1.0f && value - prevPrevVal != 0) {
        if (value > 0) txt.setCharacterSize(txt.getCharacterSize() - 1);
        else if (value < 0) txt.setCharacterSize(txt.getCharacterSize() + 1);
        prevPrevVal = prevVal;
        prevVal = value;
        value = txt.getLocalBounds().width - rect_size.x + 2.0f * padding;
    }

    txt.setOrigin(txt.getLocalBounds().left + txt.getLocalBounds().width / 2.0f,
                  txt.getLocalBounds().top + txt.getLocalBounds().height / 2.0f);
    txt.setPosition(rect_pos.x + rect_size.x / 2.0f,
                    rect_pos.y + txt.getCharacterSize());
}

void loadConfig() {
    std::fstream configFile("resources/config", std::ios::in);
    std::string line, varName, varValue;
    unsigned long pos;

    while (!configFile.eof()) {
        getline(configFile, line);
        pos = line.find("=");

        if (pos != std::string::npos && pos + 2 < line.length()) {
            varName = line.substr(0, line.find("=") - 1);
            varValue = line.substr(line.find("=") + 2);
            if (varName == "width") width = std::stoi(varValue);
            else if (varName == "height") height = std::stoi(varValue);
            else if (varName == "tileSize") tileSize = std::stoi(varValue);
            else if (varName == "tilePadding") tilePadding = std::stoi(varValue);
            else if (varName == "txtPadding") txtPadding = std::stoi(varValue);
//            else if (varName == "")  = std::stoul(varValue);
//            else if (varName == "")  = std::stoul(varValue);
//            else if (varName == "")  = std::stoul(varValue);
//            else if (varName == "")  = std::stoul(varValue);
        }
    }

    configFile.close();

    buffer.loadFromFile("resources/Tetris.ogg");
    font.loadFromFile("resources/Ubuntu-C.ttf");

    Tile::TileInit(clrNum, font, tileSize, tilePadding);

    winWIDTH = tileSize * width + tileSize * 5;
    winHEIGHT = tileSize * height + tileSize;

    renderWindow.create(VideoMode((unsigned int) winWIDTH, (unsigned int) winHEIGHT), "Tetris1597");
    playField.setSize(Vector2f(tileSize * width, tileSize * height));
    playField.setPosition(tileSize / 2.0f, tileSize / 2.0f);
    monoTile.setSize(Vector2f(tileSize - tilePadding, tileSize - tilePadding));
    monoTile.setOrigin(-tilePadding / 2.0f, -tilePadding / 2.0f);
    start.set(width / 2, 0);

    createBoard();

    //texts
    pauseTxt = Text("PAUSE", font);
    placeText(playField, pauseTxt, txtPadding);
    pauseTxt.setColor(Color(255, 255, 255, 150));

    defeatTxt = Text("DEFEAT", font);
    placeText(playField, defeatTxt, txtPadding);
    defeatTxt.setColor(Color(255, 255, 255, 150));

    wonTxt = Text("VICTORY", font);
    placeText(playField, wonTxt, txtPadding);
    wonTxt.setColor(Color(255, 255, 255, 150));

    coinsTxt = Text("COINS", font);
    placeText(Vector2f((width + 0.5f) * tileSize, tileSize / 2.0f),
              Vector2f(4.5f * tileSize, 2.0f * tileSize), coinsTxt, txtPadding);
    coinsTxt.setColor(Color(255, 255, 255, 230));

    controlsTxt = Text(controlsStr, font);
    placeText(Vector2f((width + 0.5f) * tileSize, tileSize * height * 0.6f),
              Vector2f(4.5f * tileSize, 6.0f * tileSize), controlsTxt, txtPadding);
    controlsTxt.setColor(Color(35, 35, 35, 230));

    signTxt = Text("//mk", font);
    placeText(Vector2f((width + 0.5f) * tileSize, (height - 1.0f) * tileSize),
              Vector2f(4.5f * tileSize, tileSize), signTxt, (int) (txtPadding * 3.0f));
    signTxt.setColor(Color(255, 255, 255, 40));
    //-----
}