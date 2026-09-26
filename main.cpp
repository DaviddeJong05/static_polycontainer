#include <iostream>
#include <concepts>
#include "static_polycontainer.h"

struct Circle    { float radius; };
struct Rectangle { float width, height; };
struct Triangle  { float base, height; };

int main() {
    static_polycontainer<Circle, Rectangle, Triangle> canvas;

    // 1. Memory Management (reserve)
    canvas.reserve_for<Circle>(10);
    canvas.reserve_all(5);

    // 2. State checks (empty & size on an empty container)
    std::cout << std::boolalpha << canvas.empty() << "\n";          // true
    std::cout << std::boolalpha << canvas.empty<Rectangle>() << "\n"; // true
    std::cout << canvas.size() << "\n";                             // 0

    // 3. Adding data (add & emplace)
    Circle c1{5.0f};
    canvas.add(c1); 
    canvas.add(Circle{10.0f}); 
    
    canvas.emplace<Rectangle>(4.0f, 5.0f);   
    canvas.emplace<Triangle>(6.0f, 3.0f);    

    // 4. Size checks (global & per-type)
    std::cout << canvas.size() << "\n";             // 4
    std::cout << canvas.size<Circle>() << "\n";     // 2
    std::cout << canvas.size<Rectangle>() << "\n";  // 1

    // 5. Direct vector access via get()
    const auto& circle_vector = canvas.get<Circle>();
    std::cout << circle_vector.size() << "\n";       // 2

    // 6. for_object: By-Reference Overload (Modifying internal data)
    canvas.for_object<Circle>([](Circle& c) {
        c.radius *= 2.0f; 
    });

    // 7. for_object: By-Value Overload (Reading register-copied data automatically)
    canvas.for_object<Circle>([](Circle c) {
        std::cout << c.radius << "\n";               // 10
                                                     // 20
    });

    // 8. Const-correctness check (using a const container reference)
    const auto& const_canvas = canvas;
    const_canvas.for_object<Rectangle>([](const Rectangle& r) {
        std::cout << (r.width * r.height) << "\n";   // 20
    });

    // 9. for_all (Bulk processing via C++23 Deducing This)
    canvas.for_all([](auto&& shape) {
        using T = std::remove_cvref_t<decltype(shape)>;

        if constexpr (std::same_as<T, Circle>) {
            std::cout << shape.radius << "\n";       // 10
                                                     // 20
        }
        else if constexpr (std::same_as<T, Rectangle>) {
            std::cout << shape.width << "\n";        // 4
        }
        else if constexpr (std::same_as<T, Triangle>) {
            std::cout << shape.base << "\n";         // 6
        }
    });

    // 10. Clearing a single specific type
    canvas.clear<Circle>();
    std::cout << canvas.size<Circle>() << "\n";     // 0
    std::cout << canvas.size() << "\n";             // 2

    // 11. Full cleanup
    canvas.clear_all();
    std::cout << std::boolalpha << canvas.empty() << "\n"; // true

    return 0;
}
