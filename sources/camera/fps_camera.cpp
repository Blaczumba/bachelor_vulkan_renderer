#include "fps_camera.h"

FPSCamera::FPSCamera(glm::vec3 position, glm::vec3 up, float yaw, float pitch)
    : front(glm::vec3(0.0f, 0.0f, -1.0f)), movementSpeed(SPEED), mouseSensitivity(SENSITIVITY), zoom(ZOOM) {
    position = position;
    worldUp = up;
    yaw = yaw;
    pitch = pitch;
    updateCameraVectors();
}

FPSCamera::FPSCamera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch)
    : FPSCamera(glm::vec3(posX, posY, posZ), glm::vec3(upX, upY, upZ), yaw, pitch) {
        
}

glm::mat4 FPSCamera::getViewMatrix() const {
    return glm::lookAt(position, position + front, up);
}

void FPSCamera::processKeyboard(MovementDirections direction, float deltaTime) {
    float velocity = movementSpeed * deltaTime;
    if (direction == MovementDirections::FORWARD)
        position += front * velocity;
    if (direction == MovementDirections::BACKWARD)
        position -= front * velocity;
    if (direction == MovementDirections::LEFT)
        position -= right * velocity;
    if (direction == MovementDirections::RIGHT)
        position += right * velocity;
}

void FPSCamera::processMouseMovement(float xoffset, float yoffset, bool constrainPitch) {
    xoffset *= mouseSensitivity;
    yoffset *= mouseSensitivity;

    yaw += xoffset;
    pitch += yoffset;

    if (constrainPitch)
    {
        if (pitch > 89.0f)
            pitch = 89.0f;
        if (pitch < -89.0f)
            pitch = -89.0f;
    }

    updateCameraVectors();
}

void FPSCamera::processMouseScroll(float yoffset) {
    zoom -= (float)yoffset;
    if (zoom < 1.0f)
        zoom = 1.0f;
    if (zoom > 45.0f)
        zoom = 45.0f;
}

void FPSCamera::updateCameraVectors() {
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
 
    right = glm::normalize(glm::cross(front, worldUp));
    up = glm::normalize(glm::cross(right, front));
}