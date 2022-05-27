#include "DepthFrameBuffer.h"

#include <glad/glad.h>

namespace opengl {
DepthFrameBuffer::DepthFrameBuffer(int width, int height)
    : width(width), height(height) {
  glGenFramebuffers(1, &this->frameBufferId);
  glGenTextures(1, &this->depthMapId);
  glBindTexture(GL_TEXTURE_2D, depthMapId);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0,
               GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

  bind();
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D,
                         depthMapId, 0);
  glDrawBuffer(GL_NONE);
  glReadBuffer(GL_NONE);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void DepthFrameBuffer::free() { glDeleteFramebuffers(1, &this->frameBufferId); }

void DepthFrameBuffer::bind() {
  glBindFramebuffer(GL_FRAMEBUFFER, this->frameBufferId);
}

void DepthFrameBuffer::unbind() { glBindFramebuffer(GL_FRAMEBUFFER, 0); }

void DepthFrameBuffer::prepareForCalculate() {
  glViewport(0, 0, width, height);
  glBindFramebuffer(GL_FRAMEBUFFER, frameBufferId);
  glClear(GL_DEPTH_BUFFER_BIT);
  bind();
}

void DepthFrameBuffer::bindAsTexture() {
  glBindTexture(GL_TEXTURE_2D, depthMapId);
}
}  // namespace opengl
