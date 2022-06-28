#include "RenderFrameBuffer.h"

#include <glad/glad.h>

#include <iostream>

namespace opengl {
RenderFrameBuffer::RenderFrameBuffer(int width, int height) : width(width), height(height) {
  glGenFramebuffers(1, &frameBufferId);
  glBindFramebuffer(GL_FRAMEBUFFER, frameBufferId);

  glGenTextures(1, &textureId);
  glBindTexture(GL_TEXTURE_2D, textureId);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB,
               GL_UNSIGNED_BYTE, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                         textureId, 0);

  glGenRenderbuffers(1, &renderBufferId);
  glBindRenderbuffer(GL_RENDERBUFFER, renderBufferId);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);

  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
                            GL_RENDERBUFFER, renderBufferId);

  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!"
              << std::endl;

  unbind();
}

void RenderFrameBuffer::free() {
  glDeleteFramebuffers(1, &frameBufferId);
  glDeleteTextures(1, &textureId);
}

void RenderFrameBuffer::bind() {
  glBindFramebuffer(GL_FRAMEBUFFER, frameBufferId);
}

void RenderFrameBuffer::unbind() {
  glBindFramebuffer(GL_FRAMEBUFFER, 0); 
}

void RenderFrameBuffer::prepareForRender() {
  bind();
  glViewport(0, 0, width, height);
  glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void RenderFrameBuffer::updateSize(int newWidth, int newHeight) {
  if (width == newWidth && height == newHeight) {
    return;
  }

  this->width = newWidth;
  this->height = newHeight;

  printf("Resizing to %d, %d\n", newWidth, newHeight);

  glBindTexture(GL_TEXTURE_2D, textureId);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, newWidth, newHeight, 0, GL_RGB,
               GL_UNSIGNED_BYTE, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}
}