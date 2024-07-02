#include "fps_camera.h"

#include <iostream>

FPSCamera::FPSCamera(glm::vec3 position, glm::vec3 up, float yaw, float pitch)
    : _front(glm::vec3(0.0f, 0.0f, -1.0f)), _movementSpeed(SPEED), _mouseSensitivity(SENSITIVITY), _zoom(ZOOM) {
    _position = position;
    _worldUp = up;
    _yaw = yaw;
    _pitch = pitch;
    updateCameraVectors();
}

FPSCamera::FPSCamera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch)
    : FPSCamera(glm::vec3(posX, posY, posZ), glm::vec3(upX, upY, upZ), yaw, pitch) {
        
}

glm::mat4 FPSCamera::getViewMatrix() const {
    return glm::lookAt(_position, _position + _front, _up);
}

void FPSCamera::processKeyboard(MovementDirections direction, float deltaTime) {
    float velocity = _movementSpeed * deltaTime;
    switch (direction) {
    case MovementDirections::FORWARD:
        _position += _front * velocity;
        break;
    case MovementDirections::BACKWARD:
        _position -= _front * velocity;
        break;
    case MovementDirections::LEFT:
        _position -= _right * velocity;
        break;
    case MovementDirections::RIGHT:
        _position += _right * velocity;
        break;
    }
}

void FPSCamera::processMouseMovement(float xoffset, float yoffset, bool constrainPitch) {
    xoffset *= _mouseSensitivity;
    yoffset *= _mouseSensitivity;

    _yaw += xoffset;
    _pitch += yoffset;

    if (constrainPitch)
    {
        if (_pitch > 89.0f)
            _pitch = 89.0f;
        if (_pitch < -89.0f)
            _pitch = -89.0f;
    }

    updateCameraVectors();
}

void FPSCamera::processMouseScroll(float yoffset) {
    _zoom -= (float)yoffset;
    if (_zoom < 1.0f)
        _zoom = 1.0f;
    if (_zoom > 45.0f)
        _zoom = 45.0f;
}

void FPSCamera::updateCameraVectors() {
    _front.x = cos(_yaw) * cos(_pitch);
    _front.y = sin(_pitch);
    _front.z = sin(_yaw) * cos(_pitch);
 
    _right = glm::normalize(glm::cross(_front, _worldUp));
    _up = glm::normalize(glm::cross(_right, _front));
}