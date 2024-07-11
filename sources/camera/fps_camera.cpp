#include "fps_camera.h"

FPSCamera::FPSCamera(float fovyRadians, float aspectRatio, float zNear, float zFar)
    : _fovy(fovyRadians), _aspectRatio(aspectRatio), _zNear(zNear), _zFar(zFar) {
    _projectionMatrix = glm::perspective(fovyRadians, aspectRatio, zNear, zFar);
    rotate(_yaw, _pitch);
}

//void FPSCamera::setCallbackMmanager(CallbackManager& cbManager) {
//    cbManager.attach(this);
//}

void FPSCamera::updateKeyboard(const KeyboardData& cbData) {
    switch (cbData.key) {
    case GLFW_KEY_W:
        move(cbData.deltaTime * _front);
        break;
    case GLFW_KEY_S:
        move(-cbData.deltaTime * _front);
        break;
    case GLFW_KEY_A:
        move(-cbData.deltaTime * _right);
        break;
    case GLFW_KEY_D:
        move(cbData.deltaTime * _right);
        break;
    }
}

void FPSCamera::updateMouse(const MouseData& cbData) {
    _yaw += cbData.xoffset * _mouseSensitivity;
    _pitch += cbData.yoffset * _mouseSensitivity;

    if (_pitch > 89.0f)
        _pitch = 89.0f;
    if (_pitch < -89.0f)
        _pitch = -89.0f;

    rotate(_yaw, _pitch);
}

glm::mat4 FPSCamera::getViewMatrix() const {
    return glm::lookAt(_position, _position + _front, _up);
}

const glm::mat4& FPSCamera::getProjectionMatrix() const {
    return _projectionMatrix;
}

void FPSCamera::setAspectRatio(float aspectRatio) {
    _aspectRatio = aspectRatio;
    _projectionMatrix = glm::perspective(_fovy, _aspectRatio, _zNear, _zFar);
}

void FPSCamera::setFovy(float fovyRadians) {
    _fovy = fovyRadians;
    _projectionMatrix = glm::perspective(_fovy, _aspectRatio, _zNear, _zFar);
}

void FPSCamera::setZNear(float zNear) {
    _zNear = zNear;
    _projectionMatrix = glm::perspective(_fovy, _aspectRatio, _zNear, _zFar);
}

void FPSCamera::setZFar(float zFar) {
    _zFar = zFar;
    _projectionMatrix = glm::perspective(_fovy, _aspectRatio, _zNear, _zFar);
}

void FPSCamera::move(glm::vec3 direction) {
    _position += direction * _movementSpeed;
}

void FPSCamera::rotate(float theta, float phi) {
    _front.x = cos(theta) * cos(phi);
    _front.y = sin(phi);
    _front.z = sin(theta) * cos(phi);

    _right = glm::normalize(glm::cross(_front, _worldUp));
    _up = glm::normalize(glm::cross(_right, _front));
}
