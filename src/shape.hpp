#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <queue>
#include <set>
#include <algorithm>

struct Item {
    char type = '-';
    char color = '-';

    Item(char t = '-', char c = '-') : type(t), color(c) {}

    std::string toString() const {
        return std::string({type, color});
    }
    friend std::ostream& operator<<(std::ostream& os, const Item& item) {
        return os << item.toString();
    }
    bool operator==(const Item& other) const {
        return type == other.type && color == other.color;
    }
    bool operator!=(const Item& other) const {
        return !(*this == other);
    }
    bool isEntity() const {
        return type != '-' && type != 'c' && type != 'P';
    }
};

typedef u_int64_t u64;

// layers are from down to up, quadrants are clockwise
class Shape {
public:
    int maxHight = 0; // 0 if infinite
    std::vector<std::vector<Item>> shape;

    ~Shape() = default;
    Shape(int width, int hight, int maxHight = 0)
        : maxHight(maxHight), shape(std::vector<std::vector<Item>>(hight, std::vector<Item>(width))) {}
    Shape(std::string str, int maxHight = 0);
    Shape(u64 index, int width, int maxHight = 0, std::string cry = "cu", std::string sp = "Cu");

    bool operator==(const Shape& other) const { return shape == other.shape; }
    bool operator!=(const Shape& other) const { return !(*this == other); }
    bool isEmpty() const { return shape.empty(); }

    Shape copy() const { return Shape(*this); }

    std::string toString() const;
    friend std::ostream& operator<<(std::ostream& os, const Shape& shape) { return os << shape.toString(); }
    u64 index() const;
    Shape& rotateToLeast();

    Shape getQuadrant(int y) const;
    u64 getQuadrantIndex(int y) const;
    std::vector<Shape> getItemsByLayer(const std::set<std::pair<int, int>>& items) const;

    std::vector<std::vector<int>> isStableAll(bool circleAsStable = true) const;
    bool isStable(bool circleAsStable = true) const;
    bool isCompact() const;
    bool isSeparable(int axis) const;
    std::set<std::pair<int, int>> notSeparableItems(int axis) const;
    int separableAxis() const; // -1: not seperable, other: the seperable axis
    bool isQuadrantCreatable(int y, int totalWidth = 0, bool onlyUseWeekFall = false) const;
    bool isAllQuadrantCreatable(int totalWidth = 0, bool onlyUseWeekFall = false) const;
    std::set<std::pair<int, int>> isCreatableNoPinToStack() const;
    bool isCreatableNoPin() const;

    std::set<std::pair<int,int>> findcblock(int x, int y) const;
    std::set<std::pair<int,int>> findblock(int x, int y) const;
    std::set<std::pair<int,int>> findeblock(int x, int y) const;

    Shape& breakItem(int x, int y);
    Shape& breakLayer(int x);
    Shape& breakQuadrant(int y);
    Shape& breakItems(const std::set<std::pair<int, int>>& items);

    Shape& removeEmptyLayers();
    Shape& addEmptyLayersUp(int count = 1);
    Shape& addEmptyLayersDown(int count = 1);
    
    Shape& cutHight(int maxHight, bool useFall = true);
    Shape& combine(const Shape& other); // change - in this shape to the item in other
    Shape& fall();

    Shape& rotate(int times = 1);
    Shape& cry(char color);
    Shape& pin();
    Shape& stack(const Shape& other);
    Shape& stackBase(const Shape& other);
    // the axis is between quadrant axis and axis-1
    Shape& halfBreak(int axis = 0);
    Shape cut(int axis = 0);
    Shape& exchange(Shape& other, int axis = 0);

private:
    int isStable(int x, int y, std::vector<std::vector<int>>& stable, bool circleAsStable, bool isfirst = false) const;
};
