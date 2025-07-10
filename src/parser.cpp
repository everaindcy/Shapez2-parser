#include "main.hpp"

int main(int argc, char *argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <shape_file>" << std::endl;
        return 1;
    }
    auto shapeFile = argv[1];
    auto creatableShapes = fileMap(shapeFile);

    for (;;) {
        std::string input;
        std::cout << "Enter a shape to parse (or 'exit' to quit): " << std::endl;
        std::cin >> input;
        if (input == "exit") break;

        Shape shape(0,0, MAX_HIGHT);
        if (input[0] == '0' && input[1] == 'x') {
            // If input is a hex number, convert it to u64
            u64 value;
            try {
                value = std::stoull(input.substr(2), nullptr, 16);
            } catch (const std::invalid_argument&) {
                std::cerr << "Invalid hex number." << std::endl;
                continue;
            }
            shape = Shape(value, QUAD_SIZE, MAX_HIGHT);
            std::cout << "Shape created from hex: " << shape << std::endl;
        } else {
            shape = Shape(input, MAX_HIGHT);
        }
        Shape shapeRotated = shape;
        
        if (!shape.isAllQuadrantCreatable()) {
            std::cout << "Shape is not creatable due to an invalid quadrant." << std::endl;
            continue;
        }

        if (shape.separableAxis() != -1) {
            std::cout << "Shape is creatable due to separable." << std::endl;
            continue;
        }

        if (creatableShapes.count(shapeRotated.rotateToLeast().index()) > 0) {
            std::cout << "Shape is creatable. Method:" << std::endl;
            std::cout << "\t" << shape;
            while(creatableShapes.count(shapeRotated.index()) > 0) {
                std::cout << " from:" << std::endl;
    
                u64 value = creatableShapes[shapeRotated.index()];
                auto shapeFrom = Shape(getIdx(value), QUAD_SIZE, MAX_HIGHT);
                u64 mtd = getMtd(value);
                
                int rotateTimes = 0;
                shapeRotated = shapeFrom;
                if (mtd == PIN_CODE) {
                    shapeRotated.pin();
                } else {
                    shapeRotated.stackBase(stackShapes[mtd]);
                }
                while (shapeRotated.index() != shape.index()) {
                    shapeRotated.rotate();
                    rotateTimes++;
                }
    
                std::cout << "\t" << shapeFrom.rotate(rotateTimes)
                    << (mtd == PIN_CODE ? " pin" : (" stack: " + stackShapes[mtd].copy().rotate(rotateTimes).toString()));
                
                shape = shapeFrom;
                shapeRotated = shapeFrom.rotateToLeast();
            }
            std::cout << std::endl;
            continue;
        }

        auto toStack = shape.isCreatableNoPinToStack();
        if (!toStack.empty()) {
            auto stackLayers = shape.getItemsByLayer(toStack);
            auto stackShapes = std::vector<Shape>();
            stackShapes.push_back(shape.breakItems(toStack).removeEmptyLayers());
            for (const auto& layer : stackLayers) {
                stackShapes.push_back(shape.stackBase(layer));
            }

            std::cout << "Shape is creatable. Method:" << std::endl;
            std::cout << "\t" << stackShapes.back();
            for (int i = stackLayers.size() - 1; i >= 0; i--) {
                std::cout << " from: " << std::endl;
                std::cout << "\t" << stackShapes[i] << " stack: " << stackLayers[i];
            }
            std::cout << std::endl;
            continue;
        }

        std::cout << "Shape is not creatable." << std::endl;
    }

    return 0;
}