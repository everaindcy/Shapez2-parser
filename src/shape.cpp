#include "shape.hpp"

Shape::Shape(std::string str, int maxHight) : maxHight(maxHight) {
    if (str.empty()) {
        return;
    }
    std::vector<Item> layer;
    for (int i = 0; i < str.size(); i++) {
        char c = str[i];
        if (c == ':') {
            shape.push_back(layer);
            layer.clear();
        } else {
            layer.emplace_back(c, str[++i]);
        }
    }
    shape.push_back(layer);
}

std::string Shape::toString() const {
    std::string result;
    for (const auto& layer : shape) {
        for (const auto& item : layer) {
            result += item.toString();
        }
        result += ':';
    }
    if (!result.empty()) {
        result.pop_back(); // Remove the last colon
    }
    return result;
}

Shape::Shape(u64 index, int width, int maxHight, std::string cry, std::string sp) : maxHight(maxHight) {
    if (index == 0) {
        return;
    }
    while (index > 0) {
        std::vector<Item> layer;
        for (auto i = 0; i < width; i++) {
            Item item('-', '-');
            int itemIndex = index & 0b11; // Get the last two bits
            switch (itemIndex) {
            case 0:
                item = Item('-', '-');
                break;
            case 1:
                item = Item(cry[0], cry[1]);
                break;
            case 2:
                item = Item('P', '-');
                break;
            case 3:
                item = Item(sp[0], sp[1]);
                break;
            }
            layer.push_back(item);
            index >>= 2;
        }
        shape.push_back(layer);
    }
}

u64 Shape::index() const {
    if (shape.empty()) {
        return 0;
    }
    auto& shape = *this;
    int hight = shape.shape.size();
    int width = shape.shape[0].size();
    u64 index = 0;
    for (auto l = hight - 1; l >= 0; l--) {
        const auto& layer = shape.shape[l];
        for (auto j = width - 1; j >= 0; j--) {
            const auto& item = layer[j];
            u64 itemIndex = 0;
            switch (item.type) {
            case '-':
                itemIndex = 0;
                break;
            case 'c':
                itemIndex = 1;
                break;
            case 'P':
                itemIndex = 2;
                break;
            default: // 'C' or any other type
                itemIndex = 3;
                break;
            }
            index = (index << 2) | itemIndex;
        }
    }
    return index;
}

// this method will change all entity to Cu, all cry to cu
Shape& Shape::rotateToLeast() {
    auto& shape = *this;
    if (shape.isEmpty()) {
        return shape;
    }
    int width = shape.shape[0].size();

    u64 minShape = shape.index();
    for (int i = 1; i < width; i++) {
        u64 now = shape.rotate().index();
        if (now < minShape) {
            minShape = now;
        }
    }
    shape = Shape(minShape, width, shape.maxHight);
    return shape;
}

// return a new shape that is a quadrant of this shape, the width of the new shape is 1
Shape Shape::getQuadrant(int y) const {
    auto& shape = *this;
    int hight = shape.shape.size();
    Shape quad(1, hight, shape.maxHight);
    for (int i = 0; i < hight; i++) {
        quad.shape[i][0] = shape.shape[i][y];
    }
    return quad;
}

// return the index of the quadrant y of the shape
u64 Shape::getQuadrantIndex(int y) const {
    auto& shape = *this;
    if (shape.isEmpty()) {
        return 0;
    }
    int hight = shape.shape.size();
    u64 index = 0;
    for (int i = hight - 1; i >= 0; i--) {
        const auto& item = shape.shape[i][y];
        u64 itemIndex = 0;
        switch (item.type) {
        case '-':
            itemIndex = 0;
            break;
        case 'c':
            itemIndex = 1;
            break;
        case 'P':
            itemIndex = 2;
            break;
        default: // 'C' or any other type
            itemIndex = 3;
            break;
        }
        index = (index << 2) | itemIndex;
    }
    return index;
}

// return the items layer by layer
std::vector<Shape> Shape::getItemsByLayer(const std::set<std::pair<int, int>>& items) const {
    auto& shape = *this;
    std::vector<Shape> result;
    if (shape.isEmpty()) {
        return result;
    }
    int width = shape.shape[0].size();
    int layerNow = -1;
    for (const auto& [x, y] : items) {
        if (layerNow < x) {
            layerNow = x;
            result.emplace_back(width, 1, shape.maxHight);
        }
        result.back().shape[0][y] = shape.shape[x][y];
    }
    return result;
}

int Shape::isStable(int x, int y, std::vector<std::vector<int>>& stable, bool circleAsStable, bool isfirst) const {
    auto& shape = *this;
    if (stable[x][y] != -1) {
        return stable[x][y]; // Already checked or visiting
    }
    if (shape.shape[x][y].type == '-') {
        return stable[x][y] = 1; // Empty space is always stable
    }

    auto block = shape.findblock(x, y);
    for (const auto& [bx, by] : block) {
        stable[bx][by] = -2; // Mark as visiting
    }

    int stableBlock = 0;
    for (const auto& [bx, by] : block) {
        if (bx == 0) {
            stableBlock = 1; // Block touches the ground
            break;
        }
        if (block.count(std::make_pair(bx-1, by)) != 0) {
            continue; // The support is also in the block
        }
        if (shape.shape[bx-1][by].type == '-') {
            continue; // No support for this item
        }
        int support = shape.isStable(bx-1, by, stable, circleAsStable);
        if (support == 1 || (circleAsStable && support == -2)) {
            stableBlock = 1; // Block is stable if it has support below
            break;
        }
        if (support == -2 && !isfirst) {
            stableBlock = -2;
        }
        // support = 0, continue.
    }

    if (stableBlock == 1) {
        for (const auto& [bx, by] : block) {
            stable[bx][by] = 1;
        }
        return 1;
    } else if (stableBlock == -2) {
        for (const auto& [bx, by] : block) {
            stable[bx][by] = -1;
        }
        return -2;
    } else {
        for (const auto& [bx, by] : block) {
            stable[bx][by] = 0;
        }
        return 0;
    }
}

// 0: unstable, 1: stable
// empty items is considered as stable
std::vector<std::vector<int>> Shape::isStableAll(bool circleAsStable) const {
    auto& shape = *this;
    if (shape.isEmpty()) {
        return std::vector<std::vector<int>>();
    }
    int hight = shape.shape.size();
    int width = shape.shape[0].size();
    // -1: unknown, 0: unstable, 1: stable, -2: visiting
    std::vector<std::vector<int>> stable(hight, std::vector<int>(width, -1));

    for (int i = 0; i < hight; i++) {
        for (int j = 0; j < width; j++) {
            if (stable[i][j] < 0) {
                shape.isStable(i, j, stable, circleAsStable, true);
            }
        }
    }
    return stable;
}

bool Shape::isStable(bool circleAsStable) const {
    auto& shape = *this;
    auto stable = shape.isStableAll(circleAsStable);
    for (auto layer : stable) {
        for (auto cell : layer) {
            if (cell == 0) {
                return false;
            }
        }
    }
    return true;
}

bool Shape::isCompact() const {
    auto& shape = *this;
    if (shape.isEmpty()) {
        return true;
    }
    int hight = shape.shape.size();
    int width = shape.shape[0].size();
    for (auto quad = 0; quad < width; quad++) {
        bool hasvoid = false;
        for (auto layer = 0; layer < hight; layer++) {
            if (shape.shape[layer][quad].type == '-') {
                hasvoid = true;
            } else {
                if (hasvoid) {
                    return false;
                }
            }
        }
    }
    return true;
}

// return true if can divide the shape into two parts by axis, and the two parts are both stable
bool Shape::isSeparable(int axis) const {
    auto& shape = *this;
    if (shape.isEmpty()) {
        return true;
    }
    int hight = shape.shape.size();
    int width = shape.shape[0].size();
    Shape left = Shape(width, hight);
    Shape right = Shape(width, hight);

    axis = axis % width + width;
    for (int i = 0; i < hight; i++) {
        for (int j = axis; j < axis + width/2; j++) {
            left.shape[i][j%width] = shape.shape[i][j%width];
        }
        for (int j = axis + width/2; j < axis + width; j++) {
            right.shape[i][j%width] = shape.shape[i][j%width];
        }
    }
    return left.removeEmptyLayers().isStable() && right.removeEmptyLayers().isStable();
}

// return the items that are not stable after divide the shape into two parts by axis
std::set<std::pair<int, int>> Shape::notSeparableItems(int axis) const {
    auto notSeparable = std::set<std::pair<int, int>>();

    auto& shape = *this;
    if (shape.isEmpty()) {
        return notSeparable;
    }
    int hight = shape.shape.size();
    int width = shape.shape[0].size();
    Shape left = Shape(width, hight);
    Shape right = Shape(width, hight);

    axis = axis % width + width;
    for (int i = 0; i < hight; i++) {
        for (int j = axis; j < axis + width/2; j++) {
            left.shape[i][j%width] = shape.shape[i][j%width];
        }
        for (int j = axis + width/2; j < axis + width; j++) {
            right.shape[i][j%width] = shape.shape[i][j%width];
        }
    }

    auto leftStable = left.isStableAll();
    auto rightStable = right.isStableAll();
    for (int i = 0; i < hight; i++) {
        for (int j = 0; j < width; j++) {
            if (leftStable[i][j] == 0 || rightStable[i][j] == 0) {
                notSeparable.insert(std::make_pair(i, j));
            }
        }
    }

    return notSeparable;
}

// return an axis which let the shape be separable, return -1 if the shape is not seperable
int Shape::separableAxis() const {
    auto& shape = *this;
    if (shape.isEmpty()) {
        return 0;
    }
    int width = shape.shape[0].size();
    for (int axis = 0; axis < width/2; axis++) {
        if (shape.isSeparable(axis)) {
            return axis;
        }
    }
    return -1;
}

// return true if the quadrant y of the shape is creatable
// totalWidth == 0 to use the width of the shape, totalWidth != 0 will use the totalWidth
// onlyUseWeekFall: if true, will only use the weak fall
bool Shape::isQuadrantCreatable(int y, int totalWidth, bool onlyUseWeekFall) const {
    auto& shape = *this;
    if (shape.isEmpty()) {
        return true;
    }
    int hight = shape.shape.size();
    int width = shape.shape[0].size();
    bool more6Quad = (totalWidth == 0) ? (width >= 6) : (totalWidth >= 6);
    
    int cryLayer = -1; // cryLayer is the highest layer with type 'c'
    for (cryLayer = hight-1; cryLayer >= 0; cryLayer--) {
        if (shape.shape[cryLayer][y].type == 'c') {
            break;
        }
    }
    for (int layer = cryLayer+2; layer < hight; layer++) {
        if (shape.shape[layer][y].type == 'P' && shape.shape[layer-1][y].type == '-') {
            return false;
        }
    }
    if (cryLayer == -1) {
        return true;
    }

    int pinLayer = 0; // pinLayer is the first layer from down to up which type is not 'P'
    for (pinLayer = 0; pinLayer < cryLayer; pinLayer++) {
        if (shape.shape[pinLayer][y].type != 'P') {
            break;
        }
    }
    for (int layer = pinLayer+1; layer < cryLayer; layer++) {
        if (shape.shape[layer][y].type == 'P') {
            return false;
        }
    }

    int emptyNum = 0; // emptyNum is the number of empty layers below the first cry
    for (int layer = pinLayer; layer < cryLayer; layer++) {
        if (shape.shape[layer][y].type == '-') {
            emptyNum++;
        } else if (shape.shape[layer][y].type == 'c') {
            break;
        }
    }

    if (!onlyUseWeekFall && (more6Quad || pinLayer > 0 || emptyNum >= 2)) {
        bool needUp = false;
        for (int layer = cryLayer; layer >= pinLayer; layer--) {
            auto thistype = shape.shape[layer][y].type;
            auto downtype = layer > pinLayer ? shape.shape[layer-1][y].type : 'G';
            if (needUp) {
                if (thistype == '-') {
                    continue;
                } else if (thistype == 'c') {
                    return false;
                } else { // thistype is C
                    if (downtype == 'c') {
                        return false;
                    } else { // downtype is C or G or -
                        needUp = false;
                    }
                }
            } else {
                if (thistype == '-' && downtype == 'c') {
                    return false;
                }
                if (thistype == 'c' && downtype == '-') {
                    needUp = true;
                    layer--;
                }
            }
        }
        if (needUp) {
            return false;
        }
    } else {
        bool needUp = false;
        for (int layer = cryLayer; layer >= pinLayer; layer--) {
            auto thistype = shape.shape[layer][y].type;
            auto downtype = layer > pinLayer ? shape.shape[layer-1][y].type : 'G';
            if (needUp) {
                if (thistype == '-') {
                    continue;
                } else if (thistype == 'c') {
                    return false;
                } else { // thistype is C
                    if (downtype == 'c') {
                        return false;
                    } else if (downtype == '-') {
                        layer--;
                    } else { // downtype is C or G
                        needUp = false;
                    }
                }
            } else {
                if (thistype == '-' && downtype == 'c') {
                    return false;
                }
                if (thistype == 'c' && downtype == '-') {
                    needUp = true;
                    layer--;
                }
            }
        }
        if (needUp) {
            return false;
        }
    }

    return true;
}

// return true if all quadrants of the shape is creatable
// more6Quad: if true, the shape is thinking as a more than 6 quadrants shape
bool Shape::isAllQuadrantCreatable(int totalWidth, bool onlyUseWeekFall) const {
    auto& shape = *this;
    if (shape.isEmpty()) {
        return true;
    }
    int width = shape.shape[0].size();
    for (int y = 0; y < width; y++) {
        if (!isQuadrantCreatable(y, totalWidth, onlyUseWeekFall)) {
            return false;
        }
    }
    return true;
}

// to use this method, the shape must be not separable
// if the shape is creatable from a separable shape stack something, return the items to stack
// if the shape is not creatable, or must use pin from a separable shape, return an empty set
std::set<std::pair<int, int>> Shape::isCreatableNoPinToStack() const {
    auto& shape = *this;
    if (shape.isEmpty()) {
        return std::set<std::pair<int, int>>();
    }
    int width = shape.shape[0].size();
    int hight = shape.shape.size();

    for (int axis = 0; axis < width/2; axis++) {
        bool ok = true;
        auto notSeparable = shape.notSeparableItems(axis);
        std::set<std::pair<int, int>> notSeparableStack;
        std::vector<int> lowestLayer(width, hight);
        std::vector<int> cryLayer(width, -1);
        for (auto y = 0; y < width; y++) {
            int x = hight - 1;
            while (x >= 0 && shape.shape[x][y].type != 'c') {
                x--;
            }
            cryLayer[y] = x;
        }
        for (const auto& [x, y] : notSeparable) {
            if (shape.shape[x][y].type == 'c') {
                ok = false;
                break;
            }
            if (lowestLayer[y] > x) {
                lowestLayer[y] = x;
            }
        }
        if (!ok) {
            continue;
        }
        for (auto i = 0; i < hight; i++) {
            for (auto j = 0; j < width; j++) {
                if (i < lowestLayer[j] || notSeparableStack.count({i, j}) > 0) {
                    continue;
                }
                if (shape.shape[i][j].type == '-') {
                    continue;
                }
                if (cryLayer[j] >= i) {
                    ok = false;
                    break;
                }
                auto newBlock = shape.findeblock(i, j);
                bool isSupported = false;
                int y = j;
                if (i == 0 || shape.shape[i-1][y].type != '-') {
                    isSupported = true;
                }
                notSeparableStack.insert({i, y});
                for (y = (j+1)%width; y != j; y = (y+1)%width) {
                    if (cryLayer[y] >= i || newBlock.count({i, y}) == 0) {
                        break;
                    }
                    if (lowestLayer[y] > i) {
                        lowestLayer[y] = i;
                    }
                    if (i == 0 || shape.shape[i-1][y].type != '-') {
                        isSupported = true;
                    }
                    notSeparableStack.insert({i, y});
                }
                if (y != j) {
                    for (y = (j-1+width)%width; y != j; y = (y-1+width)%width) {
                        if (cryLayer[y] >= i || newBlock.count({i, y}) == 0) {
                            break;
                        }
                        if (lowestLayer[y] > i) {
                            lowestLayer[y] = i;
                        }
                        if (i == 0 || shape.shape[i-1][y].type != '-') {
                            isSupported = true;
                        }
                        notSeparableStack.insert({i, y});
                    }
                }
                // for (const auto & [x, y] : newBlock) {
                //     if (cryLayer[y] >= x) {
                //         continue;
                //     }
                //     if (lowestLayer[y] > x) {
                //         lowestLayer[y] = x;
                //     }
                //     if (x == 0 || shape.shape[x-1][y].type != '-') {
                //         isSupported = true;
                //     }
                //     notSeparableStack.insert({x, y});
                // }
                if (!isSupported) {
                    ok = false;
                    break;
                }
            }
            if (!ok) {
                break;
            }
        }
        if (!ok) {
            continue;
        }
        if (!shape.copy().breakItems(notSeparableStack).removeEmptyLayers().isSeparable(axis)) {
            continue;
        }
        return notSeparableStack;
    }
    return std::set<std::pair<int, int>>();
}

// to use this method, the shape must be not separable
// return true if the shape is creatable from a separable shape stack something
// return false if the shape is not creatable or must use pin from a separable shape
bool Shape::isCreatableNoPin() const {
    auto& shape = *this;
    if (shape.isEmpty()) {
        return false;
    }
    auto notSeparableStack = shape.isCreatableNoPinToStack();
    return !notSeparableStack.empty();
}

// do not check if the position is valid
// return the block of cry that contains the position (x, y)
std::set<std::pair<int, int>> Shape::findcblock(int x, int y) const {
    auto& shape = *this;
    int hight = shape.shape.size();
    int width = shape.shape[0].size();
    std::set<std::pair<int, int>> block;
    if (x < 0 || x >= hight || y < 0 || y >= width || shape.shape[x][y].type != 'c') {
        return block;
    }

    auto visited = std::vector<std::vector<bool>>(hight, std::vector<bool>(width, false));
    std::queue<std::pair<int, int>> q;
    block.insert({x, y});
    visited[x][y] = true;
    q.push({x, y});
    while (!q.empty()) {
        auto [cx, cy] = q.front();
        q.pop();
        if (cx+1 < hight && !visited[cx+1][cy] && shape.shape[cx+1][cy].type == 'c') {
            block.insert({cx+1, cy});
            visited[cx+1][cy] = true;
            q.push({cx+1, cy});
        }
        if (cx-1 >= 0    && !visited[cx-1][cy] && shape.shape[cx-1][cy].type == 'c') {
            block.insert({cx-1, cy});
            visited[cx-1][cy] = true;
            q.push({cx-1, cy});
        }
        auto ty = (cy+1) % width;
        if (!visited[cx][ty] && shape.shape[cx][ty].type == 'c') {
            block.insert({cx, ty});
            visited[cx][ty] = true;
            q.push({cx, ty});
        }
        ty = (cy-1+width) % width;
        if (!visited[cx][ty] && shape.shape[cx][ty].type == 'c') {
            block.insert({cx, ty});
            visited[cx][ty] = true;
            q.push({cx, ty});
        }
    }
    return block;
}

// do not check if the position is valid
// return the block of items that contains the position (x, y)
std::set<std::pair<int,int>> Shape::findblock(int x, int y) const {
    auto& shape = *this;
    int hight = shape.shape.size();
    int width = shape.shape[0].size();
    std::set<std::pair<int, int>> block;
    if (x < 0 || x >= hight || y < 0 || y >= width || shape.shape[x][y].type == '-') {
        return block;
    }

    auto visited = std::vector<std::vector<bool>>(hight, std::vector<bool>(width, false));
    std::queue<std::pair<int, int>> q;
    block.insert({x, y});
    visited[x][y] = true;
    q.push({x, y});
    while (!q.empty()) {
        auto [cx, cy] = q.front();
        q.pop();
        Item currentItem = shape.shape[cx][cy];
        if (currentItem.type == '-') {
            continue;
        }
        if (currentItem.type == 'P') {
            continue;
        }
        if (currentItem.type == 'c') {
            if (cx+1 < hight && !visited[cx+1][cy] && shape.shape[cx+1][cy].type == 'c') {
                block.insert({cx+1, cy});
                visited[cx+1][cy] = true;
                q.push({cx+1, cy});
            }
            if (cx-1 >= 0    && !visited[cx-1][cy] && shape.shape[cx-1][cy].type == 'c') {
                block.insert({cx-1, cy});
                visited[cx-1][cy] = true;
                q.push({cx-1, cy});
            }
        }
        if (currentItem.type == 'c' || currentItem.isShape()) {
            auto ty = (cy+1) % width;
            if (!visited[cx][ty] && (shape.shape[cx][ty].type == 'c' || shape.shape[cx][ty].isShape())) {
                block.insert({cx, ty});
                visited[cx][ty] = true;
                q.push({cx, ty});
            }
            ty = (cy-1+width) % width;
            if (!visited[cx][ty] && (shape.shape[cx][ty].type == 'c' || shape.shape[cx][ty].isShape())) {
                block.insert({cx, ty});
                visited[cx][ty] = true;
                q.push({cx, ty});
            }
        }
    }
    return block;
}

// do not check if the position is valid
// return the block of items that contains the position (x, y), ignore all cry items
std::set<std::pair<int,int>> Shape::findeblock(int x, int y) const {
    auto& shape = *this;
    int hight = shape.shape.size();
    int width = shape.shape[0].size();
    std::set<std::pair<int, int>> block;
    if (x < 0 || x >= hight || y < 0 || y >= width || shape.shape[x][y].type == '-' || shape.shape[x][y].type == 'c') {
        return block;
    }

    auto visited = std::vector<std::vector<bool>>(hight, std::vector<bool>(width, false));
    std::queue<std::pair<int, int>> q;
    block.insert({x, y});
    visited[x][y] = true;
    q.push({x, y});
    while (!q.empty()) {
        auto [cx, cy] = q.front();
        q.pop();
        Item currentItem = shape.shape[cx][cy];
        if (currentItem.type == '-' || currentItem.type == 'P' || currentItem.type == 'c') {
            continue;
        }
        if (currentItem.isShape()) {
            auto ty = (cy+1) % width;
            if (!visited[cx][ty] && shape.shape[cx][ty].isShape()) {
                block.insert({cx, ty});
                visited[cx][ty] = true;
                q.push({cx, ty});
            }
            ty = (cy-1+width) % width;
            if (!visited[cx][ty] && shape.shape[cx][ty].isShape()) {
                block.insert({cx, ty});
                visited[cx][ty] = true;
                q.push({cx, ty});
            }
        }
    }
    return block;
}

// do not check if the position is valid
// break c will also break the cblock
// do not remove the empty layers
Shape& Shape::breakItem(int x, int y) {
    auto& shape = *this;
    Item& item = shape.shape[x][y];
    if (item.type != 'c') {
        item = Item('-', '-');
    } else {
        auto block = shape.findcblock(x, y);
        for (const auto& [bx, by] : block) {
            shape.shape[bx][by] = Item('-', '-');
        }
    }
    return shape;
}

Shape& Shape::breakLayer(int x) {
    auto& shape = *this;
    int width = shape.shape[0].size();
    for (int j = 0; j < width; j++) {
        shape.breakItem(x, j);
    }
    return shape;
}

Shape& Shape::breakQuadrant(int y) {
    auto& shape = *this;
    int hight = shape.shape.size();
    int width = shape.shape[0].size();
    y = (y % width + width) % width;
    for (int i = 0; i < hight; i++) {
        shape.breakItem(i, y);
    }
    return shape;
}

// do not check if the position is valid
// do not remove the empty layers
Shape& Shape::breakItems(const std::set<std::pair<int, int>>& items) {
    auto& shape = *this;
    for (const auto& [x, y] : items) {
        shape.breakItem(x, y);
    }
    return shape;
}

// remove all empty layers from the top of the shape
Shape& Shape::removeEmptyLayers() {
    for (int i = shape.size() - 1; i >= 0; i--) {
        if (std::all_of(shape[i].begin(), shape[i].end(), [](const Item& item) { return item.type == '-'; })) {
            shape.erase(shape.begin() + i);
        } else {
            break;
        }
    }
    return *this;
}

Shape& Shape::addEmptyLayersUp(int count) {
    for (int i = 0; i < count; i++) {
        shape.push_back(std::vector<Item>(shape[0].size(), Item('-', '-')));
    }
    return *this;
}

Shape& Shape::addEmptyLayersDown(int count) {
    for (int i = 0; i < count; i++) {
        shape.insert(shape.begin(), std::vector<Item>(shape[0].size(), Item('-', '-')));
    }
    return *this;
}

Shape& Shape::cutHight(int maxHight, bool useFall) {
    auto& shape = *this;
    if (maxHight <= 0) {
        shape.removeEmptyLayers();
        return shape;
    }
    if (shape.isEmpty()) {
        return shape;
    }
    int hight = shape.shape.size();
    for (int i = maxHight; i < hight; i++) {
        shape.breakLayer(i);
    }
    if (useFall) {
        shape.fall();
    } else {
        shape.removeEmptyLayers();
    }
    return shape;
}

// change every empty item in this to the item in other
Shape& Shape::combine(const Shape& other) {
    auto& shape = *this;
    if (shape.isEmpty() || other.isEmpty()) {
        return shape;
    }
    int width = shape.shape[0].size();
    if (other.shape[0].size() != width) {
        return shape;
    }
    int hight = other.shape.size();

    for (int i = 0; i < shape.shape.size()-hight; i++) {
        shape.addEmptyLayersUp();
    }

    for (int i = 0; i < hight; i++) {
        for (int j = 0; j < width; j++) {
            if (shape.shape[i][j].type == '-') {
                shape.shape[i][j] = other.shape[i][j];
            }
        }
    }
    return shape;
}

Shape& Shape::fall() {
    auto& shape = *this;
    if (shape.isEmpty()) {
        return shape;
    }
    int hight = shape.shape.size();
    int width = shape.shape[0].size();

    auto stable = shape.isStableAll();
    for (int i = 0; i < hight; i++) {
        for (int j = 0; j < width; j++) {
            if (stable[i][j] == 0 && shape.shape[i][j].type == 'c') {
                shape.shape[i][j] = Item('-', '-');
            }
        }
    }

    for (int i = 0; i < hight; i++) {
        for (int j = 0; j < width; j++) {
            if (stable[i][j] == 0 && shape.shape[i][j].type != '-') {
                auto block = shape.findblock(i, j);
                int fallTo = i - 2;
                for(;;) {
                    if (fallTo < 0) {
                        fallTo = 0;
                        break;
                    }
                    bool canFall = true;
                    for (const auto& [bx, by] : block) {
                        if (shape.shape[fallTo][by].type != '-') {
                            canFall = false;
                            break;
                        }
                    }
                    if (!canFall) {
                        fallTo++;
                        break;
                    }
                    fallTo--;
                }
                for (const auto& [bx, by] : block) {
                    shape.shape[fallTo][by] = shape.shape[bx][by];
                    shape.shape[bx][by] = Item('-', '-');
                }
            }
        }
    }
    shape.removeEmptyLayers();
    return shape;
}

Shape& Shape::rotate(int times) {
    auto& shape = *this;
    if (shape.isEmpty()) {
        return shape;
    }
    int hight = shape.shape.size();
    int width = shape.shape[0].size();
    Shape rotated(width, hight, shape.maxHight);
    times = times % width;
    for (int i = 0; i < hight; i++) {
        for (int j = 0; j < width; j++) {
            rotated.shape[i][j] = shape.shape[i][(j-times+width) % width];
        }
    }
    shape.shape = rotated.shape;
    return shape;
}

Shape& Shape::cry(char color) {
    auto& shape = *this;
    if (shape.isEmpty()) {
        return shape;
    }
    int hight = shape.shape.size();
    int width = shape.shape[0].size();
    
    for (int i = 0; i < hight; i++) {
        for (int j = 0; j < width; j++) {
            if (shape.shape[i][j].type == '-' || shape.shape[i][j].type == 'P') {
                shape.shape[i][j] = Item('c', color);
            }
        }
    }
    return shape;
}

Shape& Shape::pin() {
    auto& shape = *this;
    if (shape.isEmpty()) {
        return shape;
    }
    int width = shape.shape[0].size();
    shape.addEmptyLayersDown();
    for (int i = 0; i < width; i++) {
        if (shape.shape[1][i].type != '-') {
            shape.shape[0][i] = Item('P', '-');
        }
    }
    shape.cutHight(maxHight);
    return shape;
}

Shape& Shape::stack(const Shape& other) {
    auto& shape = *this;
    if (shape.isEmpty() || other.isEmpty()) {
        return shape;
    }
    int hight = shape.shape.size();
    int width = shape.shape[0].size();
    if (other.shape[0].size() != width) {
        return shape;
    }

    for (int i = 0; i < other.shape.size(); i++) {
        shape.shape.push_back(other.shape[i]);
    }
    for (int i = hight; i < shape.shape.size(); i++) {
        for (int j = 0; j < width; j++) {
            if (shape.shape[i][j].type == 'c') {
                shape.shape[i][j] = Item('-', '-');
            }
        }
    }

    shape.fall();
    shape.cutHight(maxHight, false);
    return shape;
}

// do not check if the other is valid
// do not fall the shape
// other should not have c
Shape& Shape::stackBase(const Shape& other) {
    auto& shape = *this;
    if (shape.isEmpty() || other.isEmpty()) {
        return shape;
    }
    int hight = shape.shape.size();
    int width = shape.shape[0].size();
    if (other.shape[0].size() != width) {
        return shape;
    }

    shape.addEmptyLayersUp();
    for (int i = 0; i < other.shape.size(); i++) {
        shape.shape.push_back(other.shape[i]);
    }

    for (int i = hight+1; i < shape.shape.size(); i++) {
        for (int j = 0; j < width; j++) {
            if (shape.shape[i][j].type != '-') {
                auto block = shape.findblock(i, j);
                int fallTo = i - 2;
                for(;;) {
                    if (fallTo < 0) {
                        fallTo = 0;
                        break;
                    }
                    bool canFall = true;
                    for (const auto& [bx, by] : block) {
                        if (shape.shape[fallTo][by].type != '-') {
                            canFall = false;
                            break;
                        }
                    }
                    if (!canFall) {
                        fallTo++;
                        break;
                    }
                    fallTo--;
                }
                for (const auto& [bx, by] : block) {
                    shape.shape[fallTo][by] = shape.shape[bx][by];
                    shape.shape[bx][by] = Item('-', '-');
                }
            }
        }
    }
    shape.cutHight(maxHight, false);
    return shape;
}

Shape& Shape::halfBreak(int axis) {
    auto& shape = *this;
    if (shape.isEmpty()) {
        return shape;
    }
    int width = shape.shape[0].size();
    
    for (int j = axis - width/2; j < axis; j++) {
        shape.breakQuadrant(j);
    }
    shape.fall();
    return shape;
}

Shape Shape::cut(int axis) {
    auto& shape = *this;
    if (shape.isEmpty()) {
        return shape;
    }
    int width = shape.shape[0].size();

    auto cutShape = shape;
    cutShape.halfBreak(axis+width/2);

    shape.halfBreak(axis);
    return cutShape;
}

Shape& Shape::exchange(Shape& other, int axis) {
    auto& shape = *this;
    if (shape.isEmpty() || other.isEmpty()) {
        return shape;
    }
    int width = shape.shape[0].size();
    if (other.shape[0].size() != width) {
        return shape;
    }

    auto toOther = shape.cut(axis);
    auto toThis = other.cut(axis);

    shape.combine(toThis);
    other.combine(toOther);
    return shape;
}
