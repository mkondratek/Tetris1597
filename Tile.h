//
// Created by mkondratek on 14.04.17.
//

#ifndef TETRIS1597_TILE_H
#define TETRIS1597_TILE_H

#include <cmath>
#include <string>
#include <SFML/Graphics.hpp>

long double phi = 1.61803398874989484820;
unsigned long long int nthfib(unsigned long long int);

using namespace sf;

class Tile {
    RenderWindow &rw;
    Text txt;
    bool fall;
    int digits;
    unsigned long long int number;

public:
    static int tileSize;
    static int padding;
    static Color clrTxt;
    static Font font;

    static void TileInit(Color _color, Font _font, int _tileSize, int _padding) {
        tileSize = _tileSize;
        padding = _padding;
        clrTxt = _color;
        font = _font;
    }

    Tile(RenderWindow &_RenderWindow, unsigned long long int num):
            rw(_RenderWindow) {
        txt.setFont(font);
        txt.setColor(Color::Black);
        fall = true;
        number = num;
        setNum(num);
    }

    unsigned long long int getNum() { return number; }

    void setNum(unsigned long long int num) {
        unsigned long long int fib = nthfib(num);
        txt.setString(std::to_string(fib));
        //txt.setColor();
        digits = 0;
        number = num;
        if (!fib) fib++;
        while (fib) {
            fib /= 10;
            digits++;
        }
        txt.setCharacterSize((unsigned int) ((tileSize - padding) - digits * tileSize / 10 - digits));
    }

    bool falling() { return fall; }
    void stop() { fall = false; }

    void draw(float x, float y) {
        float ratio = txt.getLocalBounds().height / txt.getLocalBounds().width;
        txt.setPosition(x + (tileSize - txt.getLocalBounds().width - 1) / 2 - 1,
                        y + (tileSize - txt.getLocalBounds().height) / 2 - 1
                        - txt.getCharacterSize() + txt.getLocalBounds().height);
        rw.draw(txt);
    }
};

unsigned long long int nthfib(unsigned long long int n) {
    n += 2;
    long double tmp = pow((double) phi, (double)n);
    tmp = ((tmp - (pow(-1, n)) / tmp) / sqrt(5));
    return (unsigned long long int) (tmp + 0.1);
}

Color getColorFor(unsigned long long int n) {
    n -= 100;
    return Color((Uint8) ((n + 15) % 200 + 55),
    (Uint8) ((abs((int) (n * (n + 9)))) % 100 + 155),
    (Uint8) ((abs((int) (n * n * (n + 7)))) % 100 + 155), 200);
}

template <typename T>
struct point {
    T x;
    T y;
    point(T xx, T yy): x(xx), y(yy) {}
    point() {}

    void set(T xx, T yy) { x = xx; y = yy; }
};

#endif //TETRIS1597_TILE_H
