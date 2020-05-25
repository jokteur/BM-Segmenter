#ifndef BM_SEGMENTER_DIMENSIONS_H
#define BM_SEGMENTER_DIMENSIONS_H

struct Dimension {
    int width = 0;
    int height = 0;
};

struct Position {
    float x;
    float y;
};

struct Rectangle {
    float pos_x = 0;
    float pos_y = 0;
    float width = 0;
    float height = 0;
};

#endif //BM_SEGMENTER_DIMENSIONS_H
