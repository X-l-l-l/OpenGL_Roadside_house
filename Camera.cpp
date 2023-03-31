#include "Camera.hpp"

namespace gps {

    //Camera constructor
    Camera::Camera(glm::vec3 cameraPosition, glm::vec3 cameraTarget, glm::vec3 cameraUp) {
        this->cameraPosition = cameraPosition;
        this->cameraTarget = cameraTarget;
        this->cameraUpDirection = cameraUp;

        //TODO - Update the rest of camera parameters
        this->cameraFrontDirection = cameraTarget;
    }

    glm::vec3 Camera::getCameraPosition(){
        return cameraPosition;
    }

    void Camera::setxCameraPosition(float x)
    {
        this->cameraPosition.x = x;
    }
    void Camera::setyCameraPosition(float y)
    {
        this->cameraPosition.y = y;
    }
    void Camera::setzCameraPosition(float z)
    {
        this->cameraPosition.z = z;
    }

    glm::vec3 Camera::getCameraFront(){
        return cameraFrontDirection;
    }

    void Camera::setxCameraFront(float x){
        this->cameraFrontDirection.x = x;
    }
    void Camera::setyCameraFront(float y){
        this->cameraFrontDirection.y = y;
    }
    void Camera::setzCameraFront(float z){
        this->cameraFrontDirection.z = z;
    }

    //return the view matrix, using the glm::lookAt() function
    glm::mat4 Camera::getViewMatrix() {
        return glm::lookAt(cameraPosition, cameraPosition + cameraFrontDirection, cameraUpDirection);
    }

    //update the camera internal parameters following a camera move event
    void Camera::move(MOVE_DIRECTION direction, float speed) {
        if (direction == MOVE_FORWARD)
            this->cameraPosition += speed * this->cameraFrontDirection;
        if (direction == MOVE_BACKWARD)
            this->cameraPosition -= speed * this->cameraFrontDirection;
        if (direction == MOVE_LEFT)
            this->cameraPosition -= glm::normalize(glm::cross(this->cameraFrontDirection, this->cameraUpDirection)) * speed;
        if (direction == MOVE_RIGHT)
            this->cameraPosition += glm::normalize(glm::cross(this->cameraFrontDirection, this->cameraUpDirection)) * speed;
        if (direction == MOVE_UP)
            this->cameraPosition += speed * this->cameraUpDirection;
        if (direction == MOVE_DOWN)
            this->cameraPosition -= speed * this->cameraUpDirection;
    }

    //update the camera internal parameters following a camera rotate event
    //yaw - camera rotation around the y axis
    //pitch - camera rotation around the x axis
    void Camera::rotate(float pitch, float yaw) {
        glm::vec3 direction;
        direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        direction.y = sin(glm::radians(pitch));
        direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        this->cameraFrontDirection = glm::normalize(direction);
    }
}