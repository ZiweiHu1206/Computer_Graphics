//Ziwei Hu 260889365
#include "DAGNode.h"
#include "MatrixStack.h"
#include "Program.h"
#include "Shape.h"

#include <iostream>
#include <cassert>

#include "GLSL.h"

using namespace std;

class Shape;

GLfloat node_vertices_x_axis[2][3] = {
    {  0,  0,  0 },
    {  0.1,  0,  0 },};
GLfloat node_vertices_y_axis[2][3] = {
    {  0,  0,  0 },
    {  0,  0.1,  0 },};
GLfloat node_vertices_z_axis[2][3] = {
    {  0,  0,  0 },
    {  0,  0,  0.1 },};

DAGNode::DAGNode()
{
}

DAGNode::~DAGNode()
{
}

void DAGNode::init()
{
}

void DAGNode::draw(const std::shared_ptr<Program> prog, const std::shared_ptr<Program> prog2, const std::shared_ptr<MatrixStack> MV, const std::shared_ptr<MatrixStack> P, const std::shared_ptr<Shape> sphere, const std::shared_ptr<Shape> cube, GLuint vao, GLuint posBufID, float* frameData, int frameCounter, int numChannels) const
{
    float x_pos;
    float y_pos;
    float z_pos;
    float x_rot;
    float y_rot;
    float z_rot;
    int x_rot_index;
    int y_rot_index;
    int z_rot_index;
    int channel_index = 0;
    if(this->parent != NULL){
        x_pos = offset[0];
        y_pos = offset[1];
        z_pos = offset[2];
    }
    for(auto it = this->channels.begin(); it != this->channels.end(); ++it){
        if(*it == "Xposition"){
            x_pos = frameData[frameCounter * numChannels + this->channelDataStartIndex + channel_index];
        }else if(*it == "Yposition"){
            y_pos = frameData[frameCounter * numChannels + this->channelDataStartIndex + channel_index];
        }else if(*it == "Zposition"){
            z_pos = frameData[frameCounter * numChannels + this->channelDataStartIndex + channel_index];
        }else if(*it == "Xrotation"){
            x_rot = frameData[frameCounter * numChannels + this->channelDataStartIndex + channel_index];
            x_rot_index = channel_index;
        }else if(*it == "Yrotation"){
            y_rot = frameData[frameCounter * numChannels + this->channelDataStartIndex + channel_index];
            y_rot_index = channel_index;
        }else if(*it == "Zrotation"){
            z_rot = frameData[frameCounter * numChannels + this->channelDataStartIndex + channel_index];
            z_rot_index = channel_index;
        }
        channel_index++;
    }
    //apply rotation and translation with scaling
    MV->pushMatrix();
    MV->translate(x_pos*0.01, y_pos*0.01, z_pos*0.01);
    x_rot = glm::radians(x_rot);
    y_rot = glm::radians(y_rot);
    z_rot = glm::radians(z_rot);
    for(int i = 0; i < this->channels.size(); i++){
        if(x_rot_index == i){
            MV->rotate(x_rot, 1, 0, 0);
        }else if(y_rot_index == i){
            MV->rotate(y_rot, 0, 1, 0);
        }else if(z_rot_index == i){
            MV->rotate(z_rot, 0, 0, 1);
        }
    }
    
    //draw coordinate frame for each DAGNode
    prog2->bind();
    MV->pushMatrix();
    glUniformMatrix4fv(prog2->getUniform("P"), 1, GL_FALSE, &P ->topMatrix()[0][0]);
    glUniformMatrix4fv(prog2->getUniform("MV"), 1, GL_FALSE, &MV->topMatrix()[0][0]);
    glUniform3f(prog2->getUniform("col"), 1, 0, 0);
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glGenBuffers(1, &posBufID);
    glBindBuffer(GL_ARRAY_BUFFER, posBufID);
    glBufferData(GL_ARRAY_BUFFER, 24, node_vertices_x_axis, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glDrawArrays(GL_LINE_LOOP, 0, 2);
    glDeleteBuffers(1, &posBufID);
    MV->popMatrix();
    prog2->unbind();
        
    prog2->bind();
    MV->pushMatrix();
    glUniformMatrix4fv(prog2->getUniform("P"), 1, GL_FALSE, &P ->topMatrix()[0][0]);
    glUniformMatrix4fv(prog2->getUniform("MV"), 1, GL_FALSE, &MV->topMatrix()[0][0]);
    glUniform3f(prog2->getUniform("col"), 0, 1, 0);
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glGenBuffers(1, &posBufID);
    glBindBuffer(GL_ARRAY_BUFFER, posBufID);
    glBufferData(GL_ARRAY_BUFFER, 24, node_vertices_y_axis, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glDrawArrays(GL_LINE_LOOP, 0, 2);
    glDeleteBuffers(1, &posBufID);
    MV->popMatrix();
    prog2->unbind();
        
    prog2->bind();
    MV->pushMatrix();
    glUniformMatrix4fv(prog2->getUniform("P"), 1, GL_FALSE, &P ->topMatrix()[0][0]);
    glUniformMatrix4fv(prog2->getUniform("MV"), 1, GL_FALSE, &MV->topMatrix()[0][0]);
    glUniform3f(prog2->getUniform("col"), 0, 0, 1);
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glGenBuffers(1, &posBufID);
    glBindBuffer(GL_ARRAY_BUFFER, posBufID);
    glBufferData(GL_ARRAY_BUFFER, 24, node_vertices_z_axis, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glDrawArrays(GL_LINE_LOOP, 0, 2);
    glDeleteBuffers(1, &posBufID);
    MV->popMatrix();
    prog2->unbind();
    
    
    //draw sphere for joint
    if(this->name == "Hips" || this->name == "Head" || this->name == "Chest" ||
       this->name == "Neck" || this->name == "LeftCollar" || this->name == "LeftShoulder" || this->name == "LeftElbow" || this->name == "LeftWrist" || this->name == "RightCollar" || this->name == "RightShoulder" || this->name == "RightElbow" ||
       this->name == "RightWrist" || this->name == "LeftHip" ||
       this->name == "LeftKnee" || this->name == "LeftAnkle" || this->name == "RightHip" || this->name == "RightKnee" || this->name == "RightAnkle"){
        prog->bind();
        MV->pushMatrix();
        MV->scale(0.05);
        glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, &P->topMatrix()[0][0]);
        glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE, &MV->topMatrix()[0][0]);
        sphere->Shape::draw(prog);
        MV->popMatrix();
        prog->unbind();
    }
    
    //draw body
    if(this->children.size() > 0){
        DAGNode *child = this->children[0];
        glm::vec3 y_axis = glm::vec3(0.0f, 1.0f, 0.0f);
        glm::vec3 offset = glm::vec3(child->offset[0], child->offset[1], child->offset[2]);
        offset /= 200.0f;
        
        float rot_angle = glm::acos(glm::dot(y_axis, offset));
        glm::vec3 axis_rot = glm::normalize(glm::cross(y_axis, offset));
        prog->bind();
        MV->pushMatrix();
        MV->translate(offset);
        if(glm::length(axis_rot)>0){
            MV->rotate(rot_angle, axis_rot);
        }
        MV->scale(0.02, glm::length(offset), 0.02);
        
        glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, &P->topMatrix()[0][0]);
        glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE, &MV->topMatrix()[0][0]);
        cube->Shape::draw(prog);
        MV->popMatrix();
        prog->unbind();
    }
    
    
    for(auto child : this->children){
        child->draw(prog, prog2, MV, P, sphere, cube, vao, posBufID, frameData, frameCounter, numChannels);
    }
    MV->popMatrix();
 }

    
    

    
    
