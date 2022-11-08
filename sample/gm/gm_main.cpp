#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace std;

int main(int, char **)
{
    auto V4 = glm::vec4{ 1.f, 0, 0, 1.f };
    auto Trans = glm::translate(glm::identity<glm::mat4>(), glm::vec3(1.f, 1.f, 0));

    V4 = Trans * V4;

    cout << V4.x << ", " << V4.y << ", " << V4.z << endl;
}
